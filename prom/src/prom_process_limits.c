/**
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
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#include "prom_gauge.h"
#include "prom_log.h"
#include "prom_process_limits_i.h"
#include "prom_process_collector_t.h"

/**
 * @brief Initializes each gauge metric found in prom_process_limits_t.h
 */
int
ppc_limits_new(prom_metric_t *m[], const char **label_keys) {
	if (m == NULL)
		return 0;
	m[PM_MAX_FDS] = prom_gauge_new("process_max_fds",
		"Max. number of open file descriptors (soft limit)", 0, label_keys);
	return m[PM_MAX_FDS] == NULL ? 0 : 1 << PM_MAX_FDS;
}

static double
ppc_limits_get_maxfds(int fd) {
	if (fd < 0) {
		struct rlimit l;
		getrlimit(RLIMIT_NOFILE, &l);
		return l.rlim_cur == RLIM_INFINITY ? -1 : l.rlim_cur;
	}

	char line[17*80];
	ssize_t slen;
	char *p;

	if ((slen = pread(fd, line, sizeof(line) - 1, 0)) == -1)
		return NaN;
	line[slen] = '\0';

	if ((p = strstr(line, "Max open files  ")) == NULL)
		return NaN;
	p += 16;
	while (*p != '\n' && p < (line + slen)) {
		if (*p != ' ')
			break;
		p++;
	}
	if (*p == '\n' || *p == '\0')
		return NaN;
	return strncmp(p, "unlimited  ", 11) ? strtoul(p, NULL, 10) : -1;
}

int
ppc_limits_update(int fd[], prom_metric_t *m[], const char **lvals) {
	return gup(PM_MAX_FDS, ppc_limits_get_maxfds(fd[FD_LIMITS]));
}
