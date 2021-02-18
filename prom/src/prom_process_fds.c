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

#include <dirent.h>
#include <string.h>

// Public
#include "prom_gauge.h"

// Private
#include "prom_errors.h"
#include "prom_log.h"
#include "prom_process_collector_t.h"

int
ppc_fds_new(prom_metric_t *m[], const char **label_keys) {
	if (m == NULL)
		return 0;
	m[PM_OPEN_FDS] = prom_gauge_new("process_open_fds",
		"Number of open file descriptors", 0, label_keys);
	return m[PM_OPEN_FDS] == NULL ? 0 : 1 << PM_OPEN_FDS;
}

static double
ppc_fds_count(const char *path) {
	int count = 0;
	struct dirent *de;

	if (path == NULL)
		return NaN;

	DIR *d = opendir(path);
	if (d == NULL) {
		PROM_WARN(PROM_STDIO_OPEN_DIR_ERROR " '%s'", path);
		return NaN;
	}

	while ((de = readdir(d)) != NULL) {
		if (strcmp(".", de->d_name) == 0 || strcmp("..", de->d_name) == 0)
			continue;
		count++;
	}
	if (closedir(d))
		PROM_WARN(PROM_STDIO_CLOSE_DIR_ERROR " '%s'", path);
	return count;
}

int
ppc_fds_update(const char *path, prom_metric_t *m[], const char **lvals) {
	return gup(PM_OPEN_FDS, ppc_fds_count(path));
}
