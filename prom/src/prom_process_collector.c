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
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/types.h>

// Public
#include "prom_alloc.h"
#include "prom_collector.h"
#include "prom_collector_registry.h"

// Private
#include "prom_process_fds_i.h"
#include "prom_process_fds_t.h"
#include "prom_process_limits_i.h"
#include "prom_process_stat_i.h"
#include "prom_process_stat_t.h"

static prom_map_t *ppc_collect(prom_collector_t *self);

typedef struct ppc_data {
	char *proc_limits_file_path;
	char *proc_stat_file_path;
	pid_t pid;
} ppc_cdata_t;

static void
ppc_free_data(prom_collector_t *self) {
	if (self == NULL)
		return;
	ppc_cdata_t *data = prom_collector_data_get(self);
	if (data == NULL)
		return;
	prom_free(data->proc_limits_file_path);
	data->proc_limits_file_path = NULL;
	prom_free(data->proc_stat_file_path);
	data->proc_stat_file_path = NULL;
	prom_free(data);
}

prom_collector_t *
ppc_new(const char *limits_path, const char *stat_path, pid_t pid) {
	prom_collector_t *self = prom_collector_new(COLLECTOR_NAME_PROCESS);
	if (self == NULL)
		return NULL;

	ppc_cdata_t *data = prom_malloc(sizeof(ppc_cdata_t));
	data->proc_limits_file_path =
		(limits_path == NULL) ? NULL : prom_strdup(limits_path);
	data->proc_stat_file_path =
		(stat_path == NULL) ? NULL : prom_strdup(stat_path);
	data->pid = pid < 1 ? getpid() : pid;
	prom_collector_set_collect_fn(self, &ppc_collect);
	prom_collector_data_set(self, data, &ppc_free_data);

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

static prom_map_t *
ppc_collect(prom_collector_t *self) {
	if (self == NULL)
		return NULL;

	ppc_cdata_t *data = prom_collector_data_get(self);
	if (data == NULL)
		return NULL;

	static int PAGE_SZ = 0;
	double TPS = sysconf(_SC_CLK_TCK);
	static unsigned long long last_starttime = 0;
	static double timestamp = 0;

	prom_map_t *res = NULL;
	pps_file_t *stat_f = NULL;
	pps_t *stat = NULL;

	if (PAGE_SZ == 0) {
		PAGE_SZ = sysconf(_SC_PAGE_SIZE);
		TPS = sysconf(_SC_CLK_TCK);
	}

	if (ppl_update(data->proc_limits_file_path))
		goto end;
	// count open files and update
	if (prom_gauge_set(prom_process_open_fds,prom_process_fds_count(NULL),NULL))
		goto end;


	// Allocate and create a *pps_file_t
	stat_f = pps_file_new(data->proc_stat_file_path);
	if (stat_f == NULL) {
		res = prom_collector_metrics_get(self);
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

	res = prom_collector_metrics_get(self);

end:
	// If there is any issue deallocating the following structures, return NULL
	// to indicate failure
	if (pps_file_destroy(stat_f))
		res = NULL;
	if (pps_destroy(stat))
		res = NULL;
	return res;
}
