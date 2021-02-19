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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Public
#include "prom_alloc.h"
#include "prom_gauge.h"
#include "prom_log.h"
#include "prom_collector.h"
#include "prom_collector_registry.h"
#include "prom_log.h"

// Private
#include "prom_process_collector_t.h"
#include "prom_process_fds_i.h"
#include "prom_process_limits_i.h"
#include "prom_process_stat_i.h"
#include "prom_process_stat_t.h"

static prom_map_t *ppc_collect(prom_collector_t *self);

typedef struct ppc_data {
	const char **label_vals;
	int fd[FD_COUNT];
	char *fd_dir;
	pid_t pid;
	prom_metric_t *m[PM_COUNT];
} ppc_cdata_t;

static void
ppc_free_data(prom_collector_t *self) {
	if (self == NULL)
		return;
	ppc_cdata_t *data = prom_collector_data_get(self);
	if (data == NULL)
		return;

	// we are brave and reset before free, even so we do not need it
	for (int i=0; i < FD_COUNT; i++) {
		if (data->fd[i] >= 0)
			close(data->fd[i]);
		data->fd[i] = -3;
	}
	prom_free(data->fd_dir);
	data->fd_dir = NULL;
	memset(&(data->m[0]), 0, sizeof(data->m));
	data->label_vals = NULL;
	prom_free(data);
}

prom_collector_t *
ppc_new(const char *limits_path, const char *stat_path, pid_t pid,
	const char **label_keys, const char **label_vals)
{
#define BUF_SZ 32
	char buf[BUF_SZ];
	int err;

	prom_collector_t *self = prom_collector_new(COLLECTOR_NAME_PROCESS);
	if (self == NULL)
		return NULL;

	ppc_cdata_t *data = prom_malloc(sizeof(ppc_cdata_t));
	if (data == NULL)
		return NULL;

	// init
	memset(data, 0, sizeof(ppc_cdata_t));
	for (int i=0; i < FD_COUNT;  i++)
		data->fd[i] = -2;
	prom_collector_data_set(self, data, &ppc_free_data);

	data->label_vals = label_vals;
	data->pid = pid < 1 ? getpid() : pid;
	sprintf(buf, "/proc/%d/fd", data->pid);
	data->fd_dir = prom_strdup(buf);

	if (limits_path != NULL) {
		if ((data->fd[FD_LIMITS] = open(limits_path, O_RDONLY, 0666)) == -1) {
			PROM_WARN("Failed to open '%s'", limits_path);
			goto fail;
		}
	}
	if (stat_path == NULL) {
#ifdef __sun
		#define STAT_FILE "/proc/%d/status"
#else
		// asume Linux
		#define STAT_FILE "/proc/%d/stat"
#endif
		if ((err = snprintf(buf, BUF_SZ, STAT_FILE, data->pid)) < 0) {
			PROM_WARN("Failed to open '%s'", buf);
			goto fail;
		}
		stat_path = buf;
	} else {
		err = strlen(stat_path);
	}
	if ((data->fd[FD_STAT] = open(stat_path, O_RDONLY, 0666)) == -1) {
		PROM_WARN("Failed to open '%s'", stat_path);
		goto fail;
	}
#undef STAT_FILE

#ifdef __sun
	if (err > (BUF_SZ - 7) || err < 0) {
		PROM_WARN("stat_path '%s' - unexpected length (%d)", stat_path, err);
		goto fail;
	}
	err -= 6;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
	strncpy(buf, stat_path, err);
#pragma GCC diagnostic pop
	strcpy(buf + err, "psinfo");
	if ((data->fd[FD_PSINFO] = open(buf, O_RDONLY)) == -1) {
		PROM_WARN("Failed to open '%s'", buf);
		goto fail;
	}
	strcpy(buf + err, "usage");
	if ((data->fd[FD_USAGE] = open(buf, O_RDONLY)) == -1) {
		PROM_WARN("Failed to open '%s'", buf);
		goto fail;
	}
#endif
#undef BUF_SZ

	if (ppc_limits_new(data->m, label_keys) == 0)
		goto fail;
	if (ppc_fds_new(data->m, label_keys) == 0)
		goto fail;
	if (ppc_stats_new(data->m, label_keys) == 0)
		goto fail;

	err = 0;
	for (int i = 0; i < PM_COUNT; i++)
		err += prom_collector_add_metric(self, data->m[i]);

	prom_collector_set_collect_fn(self, &ppc_collect);
	return self;

fail:
	for (int i = 0; i < PM_COUNT; i++)
		prom_gauge_destroy((prom_gauge_t *) data->m[i]);
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

	ppc_fds_update(data->fd_dir, data->m, data->label_vals);
	ppc_limits_update(data->fd, data->m, data->label_vals);
	ppc_stats_update(data->fd, data->m, data->label_vals);

	return prom_collector_metrics_get(self);
}
