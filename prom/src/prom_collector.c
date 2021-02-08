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

#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>

// Public
#include "prom_alloc.h"
#include "prom_collector.h"
#include "prom_collector_registry.h"

// Private
#include "prom_assert.h"
#include "prom_collector_t.h"
#include "prom_log.h"
#include "prom_map_i.h"
#include "prom_metric_i.h"
#include "prom_process_fds_i.h"
#include "prom_process_fds_t.h"
#include "prom_process_limits_i.h"
#include "prom_process_limits_t.h"
#include "prom_process_stat_i.h"
#include "prom_process_stat_t.h"
#include "prom_string_builder.h"

prom_map_t *
prom_collector_default_collect(prom_collector_t *self) {
	return self->metrics;
}

prom_collector_t *
prom_collector_new(const char *name) {
	prom_collector_t *self = (prom_collector_t *)
		prom_malloc(sizeof(prom_collector_t));
	if (self == NULL)
		return NULL;

	self->name = prom_strdup(name);
	self->collect_fn = &prom_collector_default_collect;
	self->string_builder = NULL;
	self->proc_limits_file_path = NULL;
	self->proc_stat_file_path = NULL;

	self->metrics = prom_map_new();
	if (self->metrics == NULL)
		goto fail;
	if (prom_map_set_free_value_fn(self->metrics, &prom_metric_free_generic))
		goto fail;
	if ((self->string_builder = psb_new()) == NULL)
		goto fail;
	return self;

fail:
    prom_collector_destroy(self);
    return NULL;
}

int
prom_collector_destroy(prom_collector_t *self) {
	if (self == NULL)
		return 0;

	int r = 0;

	r += prom_map_destroy(self->metrics);
	self->metrics = NULL;
	r += psb_destroy(self->string_builder);
	self->string_builder = NULL;
	prom_free((char *)self->name);
	self->name = NULL;
	prom_free((char *)self->proc_limits_file_path);
	self->proc_limits_file_path = NULL;
	prom_free((char *)self->proc_stat_file_path);
	self->proc_stat_file_path = NULL;
	self->collect_fn = NULL;
	prom_free(self);
	return r;
}

int
prom_collector_destroy_generic(void *gen) {
	if (gen == NULL)
		return 0;
	prom_collector_t *self = (prom_collector_t *)gen;
	return prom_collector_destroy(self);
}

void
prom_collector_free_generic(void *gen) {
	if (gen == NULL)
		return;
	prom_collector_t *self = (prom_collector_t *)gen;
	prom_collector_destroy(self);
}

int
prom_collector_set_collect_fn(prom_collector_t *self, prom_collect_fn *fn) {
	if (self == NULL)
		return 1;
	self->collect_fn = fn;
	return 0;
}

int
prom_collector_add_metric(prom_collector_t *self, prom_metric_t *metric) {
	if (self == NULL)
		return 1;
	if (prom_map_get(self->metrics, metric->name) != NULL) {
		PROM_LOG("metric already found in collector");
		return 1;
	}
	return prom_map_set(self->metrics, metric->name, metric);
}

//////////////////////////////////////////////////////////////////////////////
// Process Collector

static prom_map_t *pcp_collect(prom_collector_t *self);

prom_collector_t *
prom_collector_process_new(const char *limits_path, const char *stat_path) {
	prom_collector_t *self = prom_collector_new("process");
	if (self == NULL)
		return NULL;

	self->proc_limits_file_path =
		(limits_path == NULL) ? NULL : prom_strdup(limits_path);
	self->proc_stat_file_path =
		(stat_path == NULL) ? NULL : prom_strdup(stat_path);
	self->collect_fn = &pcp_collect;

	if (ppl_init())
		goto fail;
	if (prom_process_fds_init())
		goto fail;
	if (prom_collector_add_metric(self, prom_process_max_fds))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_open_fds))
		goto fail;

	if (pps_init())
		goto fail;
	if (prom_collector_add_metric(self, prom_process_minflt))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_cminflt))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_majflt))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_cmajflt))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_utime))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_stime))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_time))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_cutime))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_cstime))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_ctime))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_num_threads))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_starttime))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_vsize))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_rss))
		goto fail;
	if (prom_collector_add_metric(self, prom_process_blkio))
		goto fail;

	return self;

fail:
	ppl_cleanup();
	pps_cleanup();
	prom_process_fds_cleanup();
	prom_collector_destroy(self);
	return NULL;
}

prom_map_t *
pcp_collect(prom_collector_t *self) {
	if (self == NULL)
		return NULL;

	static int PAGE_SZ = 0;
	double TPS = sysconf(_SC_CLK_TCK);
	static unsigned long long last_starttime = 0;
	static double timestamp = 0;

	prom_map_t *res = NULL;
	pps_file_t *stat_f = NULL;
	ppl_file_t *limits_f = NULL;
	prom_map_t *limits_map = NULL;
	pps_t *stat = NULL;

	if (PAGE_SZ == 0) {
		PAGE_SZ = sysconf(_SC_PAGE_SIZE);
		TPS = sysconf(_SC_CLK_TCK);
	}

	// Allocate and create a *ppl_file_t
	limits_f = ppl_file_new(self->proc_limits_file_path);
	if (limits_f == NULL)
		goto end;
	// Allocate and create a *prom_map_t from ppl_file_t.
	// This is the main storage container for the limits metric data
	limits_map = ppl(limits_f);
	if (limits_map == NULL)
		goto end;

	// Retrieve the *ppl_row_t for Max open files
	ppl_row_t *max_fds = (ppl_row_t *)
		prom_map_get(limits_map, "Max open files");
	if (max_fds == NULL)
		goto end;
	if (prom_gauge_set(prom_process_max_fds, max_fds->soft, NULL))
		goto end;
	// count open files and update
	if (prom_gauge_set(prom_process_open_fds,prom_process_fds_count(NULL),NULL))
		goto end;


	// Allocate and create a *pps_file_t
	stat_f = pps_file_new(self->proc_stat_file_path);
	if (stat_f == NULL) {
		res = self->metrics;
		goto end;
	}
	// Allocate and create a *pps_t from *pps_file_t
	stat = pps_new(stat_f);
	if (stat == NULL)
		goto end;

	// Set the metrics related to the stat file
	if (prom_counter_reset(prom_process_minflt, stat->minflt, NULL))
		goto end;
	if (prom_counter_reset(prom_process_cminflt, stat->cminflt, NULL))
		goto end;
	if (prom_counter_reset(prom_process_majflt, stat->majflt, NULL))
		goto end;
	if (prom_counter_reset(prom_process_cmajflt, stat->cmajflt, NULL))
		goto end;
	if (prom_counter_reset(prom_process_utime, stat->utime / TPS, NULL))
		goto end;
	if (prom_counter_reset(prom_process_stime, stat->stime / TPS, NULL))
		goto end;
	if (prom_counter_reset(prom_process_time, (stat->utime + stat->stime) / TPS,
		NULL))
	{
		goto end;
	}
	if (prom_counter_reset(prom_process_cutime, stat->cutime / TPS, NULL))
		goto end;
	if (prom_counter_reset(prom_process_cstime, stat->cstime / TPS, NULL))
		goto end;
	if (prom_counter_reset(prom_process_ctime,(stat->cutime+stat->cstime) / TPS,
		NULL))
	{
		goto end;
	}
	if (prom_gauge_set(prom_process_num_threads, stat->num_threads, NULL))
		goto end;
	// usually a constant value. However if this fn gets called for several
	// other processes ...
	if (last_starttime != stat->starttime || timestamp == 0) {
		struct sysinfo s_info;
		time_t now = time(NULL);
		long uptime = sysinfo(&s_info) ? now : s_info.uptime;
		last_starttime = stat->starttime;
		timestamp = now - uptime + (stat->starttime / TPS);
	}
	if (prom_counter_reset(prom_process_starttime, timestamp, NULL))
		goto end;
	if (prom_gauge_set(prom_process_vsize, stat->vsize, NULL))
		goto end;
	if (prom_gauge_set(prom_process_rss, stat->rss * PAGE_SZ, NULL))
		goto end;
	if (prom_counter_reset(prom_process_blkio, stat->blkio, NULL))
		goto end;

	res = self->metrics;

end:
	// If there is any issue deallocating the following structures, return NULL
	// to indicate failure
	if (ppl_file_destroy(limits_f))
		res = NULL;
	if (prom_map_destroy(limits_map))
		res = NULL;
	if (pps_file_destroy(stat_f))
		res = NULL;
	if (pps_destroy(stat))
		res = NULL;
	return res;
}
