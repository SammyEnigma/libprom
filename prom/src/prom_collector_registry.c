/**
 * Copyright 2019-2020 DigitalOcean Inc.
 * Copyright 2021 Jens Elkner <jel+libprom@cs.uni-magdeburg.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <time.h>

// Public
#include "prom_alloc.h"
#include "prom_collector.h"
#include "prom_collector_registry.h"

// Private
#include "prom_assert.h"
#include "prom_collector_registry_t.h"
#include "prom_collector_t.h"
#include "prom_errors.h"
#include "prom_log.h"
#include "prom_map_i.h"
#include "prom_metric_formatter_i.h"
#include "prom_metric_i.h"
#include "prom_metric_t.h"
#include "prom_process_limits_i.h"
#include "prom_string_builder.h"

pcr_t *PROM_COLLECTOR_REGISTRY;

pcr_t *
pcr_new(const char *name) {
	pcr_t *self = (pcr_t *) prom_malloc(sizeof(pcr_t));
	if (self == NULL)
		return NULL;

	self->features = 0;
	self->scrape_duration = NULL;
	self->mprefix = NULL;

	self->name = prom_strdup(name);
	self->collectors = prom_map_new();
	if (self->collectors != NULL) {
		prom_map_set_free_value_fn(self->collectors,
			&prom_collector_free_generic);
		prom_map_set(self->collectors, COLLECTOR_NAME_DEFAULT,
			prom_collector_new(COLLECTOR_NAME_DEFAULT));
	}

	self->metric_formatter = pmf_new();
	self->string_builder = psb_new();
	self->lock = (pthread_rwlock_t *) prom_malloc(sizeof(pthread_rwlock_t));
	if (pthread_rwlock_init(self->lock, NULL) != 0 || self->name == NULL
		|| self->collectors == NULL || self->metric_formatter == 0
		|| self->string_builder == NULL)
	{
		PROM_WARN("failed to initialize rwlock for pcr '%s'", name);
		pcr_destroy(self);
		return NULL;
	}
	return self;
}

int
pcr_enable_process_metrics(pcr_t *self) {
	if (self == NULL)
		return 0;

	const char *cname = COLLECTOR_NAME_PROCESS;
	if (prom_map_get(self->collectors, cname) != NULL) {
		PROM_WARN("A collector named '%s' is already registered.", cname);
		return 1;
	}
	prom_collector_t *c = prom_collector_process_new(NULL, NULL);
	if (c == NULL)
		return 2;
	if (prom_map_set(self->collectors, cname, c) != 0) {
		return 3;
	}

	self->features |= PROM_PROCESS;
	return 0;
}

int
pcr_enable_scrape_metrics(pcr_t *self) {
	const char *mname = METRIC_NAME_SCRAPE;
	if (self == NULL)
		return 1;

	prom_gauge_t *g = prom_gauge_new(mname, "Duration of a collector scrape",
		1, (const char *[]) {"collector"});
	if (g == NULL)
		return 1;
	self->scrape_duration = g;
	self->features |= PROM_SCRAPETIME;
	return 0;
}

int
pcr_enable_custom_process_metrics(pcr_t *self, const char *limits_path,
	const char *stats_path)
{
	const char *cname = COLLECTOR_NAME_PROCESS;
	if (self == NULL) {
		PROM_WARN("pcr_t is NULL", "");
		return 1;
	}
	prom_collector_t *c = pcr_get(self, cname);
	if (c != NULL) {
		PROM_WARN("The registry '%s' already contains a '%s' collector.",
			self->name, cname);
		return 1;
	}
	c = prom_collector_process_new(limits_path, stats_path);
	if (c == NULL) {
		PROM_WARN("Failed to create a new '%s' collector from '%s' and '%s'.",
			cname, limits_path, stats_path);
		return 1;
	}

	if (prom_map_set(self->collectors, cname, c) != 0) {
		prom_collector_destroy(c);
		return 1;
	}
	self->features |= PROM_PROCESS;
    return 0;
}

int
pcr_init(PROM_INIT_FLAGS features, const char *mprefix) {
	int err = 0;

	const char *cname = REGISTRY_NAME_DEFAULT;

	if (PROM_COLLECTOR_REGISTRY != NULL) {
		PROM_WARN("The registry '%s' is already set as default registry.",
			PROM_COLLECTOR_REGISTRY->name);
		return 1;
	}

	PROM_COLLECTOR_REGISTRY = pcr_new(cname);
	if (PROM_COLLECTOR_REGISTRY == NULL)
		return 1;

	if (features & PROM_PROCESS)
		err += pcr_enable_process_metrics(PROM_COLLECTOR_REGISTRY);
	if (features & PROM_SCRAPETIME_ALL)
		features |= PROM_SCRAPETIME;
	if ((err == 0) && (features & PROM_SCRAPETIME))
		err += pcr_enable_scrape_metrics(PROM_COLLECTOR_REGISTRY);
	if (err) {
		pcr_destroy(PROM_COLLECTOR_REGISTRY);
		PROM_COLLECTOR_REGISTRY = NULL;
	} else {
		if (features & PROM_SCRAPETIME_ALL)
			PROM_COLLECTOR_REGISTRY->features |= PROM_SCRAPETIME_ALL;
		if (features & PROM_COMPACT)
			PROM_COLLECTOR_REGISTRY->features |= PROM_COMPACT;
		PROM_COLLECTOR_REGISTRY->mprefix = (mprefix==NULL||strlen(mprefix) == 0)
			? NULL
			: prom_strdup(mprefix);
	}

	return err;
}

int
pcr_default_init(void) {
	return pcr_init(PROM_PROCESS | PROM_SCRAPETIME, METRIC_LABEL_SCRAPE "_");
}

int
pcr_destroy(pcr_t *self) {
	if (self == NULL)
		return 0;

	int err = prom_map_destroy(self->collectors);
	err += prom_gauge_destroy(self->scrape_duration);
	err += pmf_destroy(self->metric_formatter);
	err += psb_destroy(self->string_builder);
	err += pthread_rwlock_destroy(self->lock);
	prom_free(self->lock);
	if (self->mprefix != NULL)
		prom_free((char *) self->mprefix);
	prom_free((char *)self->name);
	prom_free(self);
	return err;
}

int
pcr_register_metric(prom_metric_t *metric) {
	PROM_ASSERT(metric != NULL);

	prom_collector_t *default_collector = (prom_collector_t *)
		prom_map_get(PROM_COLLECTOR_REGISTRY->collectors,
			COLLECTOR_NAME_DEFAULT);

	if (default_collector == NULL)
		return 1;

	return prom_collector_add_metric(default_collector, metric);
}

prom_metric_t *
pcr_must_register_metric(prom_metric_t *metric) {
	if (pcr_register_metric(metric))
		exit(1);
	return metric;
}

int
pcr_register_collector(pcr_t *self, prom_collector_t *collector) {
	int ret = 1;
	if (self == NULL)
		return 1;

	if (pthread_rwlock_wrlock(self->lock)) {
		PROM_WARN(PROM_PTHREAD_RWLOCK_LOCK_ERROR, NULL);
		return 2;
	}
	if (prom_map_get(self->collectors, collector->name) != NULL) {
		PROM_WARN("The prom_collector '%s' is already registered - skipping.",
			collector->name);
		goto end;
	}
	if (prom_map_set(self->collectors, collector->name, collector))
		goto end;

	ret = 0;

end:
	if (pthread_rwlock_unlock(self->lock)) {
		PROM_WARN(PROM_PTHREAD_RWLOCK_UNLOCK_ERROR, NULL);
		return 1;
	}
	return ret;
}

prom_collector_t *
pcr_get(pcr_t *self, const char *name) {
	return (self == NULL || name == NULL)
		? NULL
		: prom_map_get(self->collectors, name);
}

int
pcr_validate_metric_name(pcr_t *self, const char *metric_name) {
	regex_t r;
	int ret = 0;
	if (regcomp(&r, "^[a-zA-Z_:][a-zA-Z0-9_:]*$", REG_EXTENDED)) {
		PROM_WARN(PROM_REGEX_REGCOMP_ERROR, NULL);
		ret = 1;
	} else if (regexec(&r, metric_name, 0, NULL, 0)) {
		PROM_WARN(PROM_REGEX_REGEXEC_ERROR, NULL);
		ret = 2;
	}
	regfree(&r);
	return ret;
}

const char *
pcr_bridge(pcr_t *self) {
	struct timespec start, end;
	static const char *labels[] = { METRIC_LABEL_SCRAPE };
	bool scrape = (self->scrape_duration != NULL)
		&& (self->features & PROM_SCRAPETIME);
	bool compact = (self->features & PROM_COMPACT) ? true : false;

	if (scrape)
		clock_gettime(CLOCK_MONOTONIC, &start);

	pmf_clear(self->metric_formatter);
	pmf_load_metrics(self->metric_formatter, self->collectors,
		 (self->features & PROM_SCRAPETIME_ALL) ? self->scrape_duration : NULL,
		 self->mprefix, compact);

	if (scrape) {
		int r = clock_gettime(CLOCK_MONOTONIC, &end);
		time_t s = (r == 0) ? end.tv_sec - start.tv_sec : 0;
		long ns = (r == 0) ? end.tv_nsec - start.tv_nsec : 0;
		double duration = s + ns*1e-9;
		prom_gauge_set(self->scrape_duration, duration, labels);
		pmf_load_metric(self->metric_formatter, self->scrape_duration,
			self->mprefix, compact);
	}
	return (const char *) pmf_dump(self->metric_formatter);
}
