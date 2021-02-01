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
#include "prom_string_builder_i.h"

prom_collector_registry_t *PROM_COLLECTOR_REGISTRY;

prom_collector_registry_t *prom_collector_registry_new(const char *name) {
	prom_collector_registry_t *self = (prom_collector_registry_t *)
		prom_malloc(sizeof(prom_collector_registry_t));
	if (self == NULL)
		return NULL;

	self->features = 0;
	self->scrape_duration = NULL;

	self->name = prom_strdup(name);
	self->collectors = prom_map_new();
	if (self->collectors != NULL) {
		prom_map_set_free_value_fn(self->collectors,
			&prom_collector_free_generic);
		prom_map_set(self->collectors, COLLECTOR_NAME_DEFAULT,
			prom_collector_new(COLLECTOR_NAME_DEFAULT));
	}

	self->metric_formatter = prom_metric_formatter_new();
	self->string_builder = prom_string_builder_new();
	self->lock = (pthread_rwlock_t *) prom_malloc(sizeof(pthread_rwlock_t));
	if (pthread_rwlock_init(self->lock, NULL) != 0 || self->name == NULL
		|| self->collectors == NULL || self->metric_formatter == 0
		|| self->string_builder == NULL)
	{
		PROM_LOG("failed to initialize rwlock");
		prom_collector_registry_destroy(self);
		return NULL;
	}
	return self;
}

int
prom_collector_registry_enable_process_metrics(prom_collector_registry_t *self)
{
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
prom_collector_registry_enable_scrape_metrics(prom_collector_registry_t *self) {
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
prom_collector_registry_enable_custom_process_metrics(prom_collector_registry_t *self,
	const char *limits_path, const char *stats_path)
{
	const char *cname = COLLECTOR_NAME_PROCESS;
	if (self == NULL) {
		PROM_WARN("prom_collector_registry_t is NULL", "");
		return 1;
	}
	prom_collector_t *c = prom_collector_registry_get(self, cname);
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

int prom_collector_registry_init(PROM_INIT_FLAGS features) {
	int err = 0;

	const char *cname = REGISTRY_NAME_DEFAULT;

	if (PROM_COLLECTOR_REGISTRY != NULL) {
		PROM_WARN("The registry '%s' is already set as default registry.",
			PROM_COLLECTOR_REGISTRY->name);
		abort();
		return 1;
	}

	PROM_COLLECTOR_REGISTRY = prom_collector_registry_new(cname);
	if (PROM_COLLECTOR_REGISTRY == NULL)
		return 1;

	if (features & PROM_PROCESS)
		err +=
		prom_collector_registry_enable_process_metrics(PROM_COLLECTOR_REGISTRY);
	if (features & PROM_SCRAPETIME_ALL)
		features |= PROM_SCRAPETIME;
	if ((err == 0) && (features & PROM_SCRAPETIME))
		err +=
		prom_collector_registry_enable_scrape_metrics(PROM_COLLECTOR_REGISTRY);
	if (err) {
		prom_collector_registry_destroy(PROM_COLLECTOR_REGISTRY);
		PROM_COLLECTOR_REGISTRY = NULL;
	} else if (features & PROM_SCRAPETIME_ALL) {
		PROM_COLLECTOR_REGISTRY->features |= PROM_SCRAPETIME_ALL;
	}

	return err;
}

int prom_collector_registry_default_init(void) {
	return prom_collector_registry_init(PROM_PROCESS | PROM_SCRAPETIME);
}

int prom_collector_registry_destroy(prom_collector_registry_t *self) {
	if (self == NULL)
		return 0;

	int err = prom_map_destroy(self->collectors);
	err += prom_gauge_destroy(self->scrape_duration);
	err += prom_metric_formatter_destroy(self->metric_formatter);
	err += prom_string_builder_destroy(self->string_builder);
	err += pthread_rwlock_destroy(self->lock);
	prom_free(self->lock);
	prom_free((char *)self->name);
	prom_free(self);
	return err;
}

int prom_collector_registry_register_metric(prom_metric_t *metric) {
	PROM_ASSERT(metric != NULL);

	prom_collector_t *default_collector = (prom_collector_t *)
		prom_map_get(PROM_COLLECTOR_REGISTRY->collectors,
			COLLECTOR_NAME_DEFAULT);

	if (default_collector == NULL)
		return 1;

	return prom_collector_add_metric(default_collector, metric);
}

prom_metric_t *prom_collector_registry_must_register_metric(prom_metric_t *metric) {
  int err = prom_collector_registry_register_metric(metric);
  if (err != 0) {
    exit(err);
  }
  return metric;
}

int prom_collector_registry_register_collector(prom_collector_registry_t *self, prom_collector_t *collector) {
  PROM_ASSERT(self != NULL);
  if (self == NULL) return 1;

  int r = 0;

  r = pthread_rwlock_wrlock(self->lock);
  if (r) {
    PROM_LOG(PROM_PTHREAD_RWLOCK_LOCK_ERROR);
    return 1;
  }
  if (prom_map_get(self->collectors, collector->name) != NULL) {
		PROM_WARN("The prom_collector '%s' is already registered - skipping.",
			collector->name);
    int rr = pthread_rwlock_unlock(self->lock);
    if (rr) {
      PROM_LOG(PROM_PTHREAD_RWLOCK_UNLOCK_ERROR);
      return rr;
    } else {
      return 1;
    }
  }
  r = prom_map_set(self->collectors, collector->name, collector);
  if (r) {
    int rr = pthread_rwlock_unlock(self->lock);
    if (rr) {
      PROM_LOG(PROM_PTHREAD_RWLOCK_UNLOCK_ERROR);
      return rr;
    } else {
      return r;
    }
  }
  r = pthread_rwlock_unlock(self->lock);
  if (r) {
    PROM_LOG(PROM_PTHREAD_RWLOCK_UNLOCK_ERROR);
    return 1;
  }
  return 0;
}

prom_collector_t *
prom_collector_registry_get(prom_collector_registry_t *self, const char *name) {
	return (self == NULL || name == NULL)
		? NULL
		: prom_map_get(self->collectors, name);
}

int prom_collector_registry_validate_metric_name(prom_collector_registry_t *self, const char *metric_name) {
  regex_t r;
  int ret = 0;
  ret = regcomp(&r, "^[a-zA-Z_:][a-zA-Z0-9_:]*$", REG_EXTENDED);
  if (ret) {
    PROM_LOG(PROM_REGEX_REGCOMP_ERROR);
    regfree(&r);
    return ret;
  }

  ret = regexec(&r, metric_name, 0, NULL, 0);
  if (ret) {
    PROM_LOG(PROM_REGEX_REGEXEC_ERROR);
    regfree(&r);
    return ret;
  }
  regfree(&r);
  return 0;
}

const char *prom_collector_registry_bridge(prom_collector_registry_t *self) {
	struct timespec start, end;
	static const char *labels[] = { METRIC_LABEL_SCRAPE };
	bool scrape = (self->scrape_duration != NULL)
		&& (self->features & PROM_SCRAPETIME);

	if (scrape)
		clock_gettime(CLOCK_MONOTONIC, &start);

	prom_metric_formatter_clear(self->metric_formatter);
	prom_metric_formatter_load_metrics(self->metric_formatter, self->collectors,
		 (self->features & PROM_SCRAPETIME_ALL) ? self->scrape_duration : NULL);

	if (scrape) {
		int r = clock_gettime(CLOCK_MONOTONIC, &end);
		time_t s = (r == 0) ? end.tv_sec - start.tv_sec : 0;
		long ns = (r == 0) ? end.tv_nsec - start.tv_nsec : 0;
		double duration = s + ns*1e-9;
		prom_gauge_set(self->scrape_duration, duration, labels);
		prom_metric_formatter_load_metric(self->metric_formatter,
			self->scrape_duration);
	}
	return (const char *) prom_metric_formatter_dump(self->metric_formatter);
}
