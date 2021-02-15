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
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdbool.h>

#include "prom_log.h"
#include "prom_process_limits_i.h"
#include "prom_process_limits_t.h"

prom_gauge_t *prom_process_max_fds = NULL;

/**
 * @brief Initializes each gauge metric found in prom_process_limits_t.h
 */
int
ppl_init(void) {
	prom_process_max_fds = prom_gauge_new("process_max_fds",
		"Maximum number of open file descriptors (soft limit)", 0, NULL);

	return 0;
}

void
ppl_cleanup(void) {
	prom_gauge_destroy(prom_process_max_fds);
	prom_process_max_fds = NULL;
}

int
ppl_update(const char *path) {
	if (prom_process_max_fds == NULL) {
		PROM_WARN("prom_process_max_fds instance not yet initialized", "");
		return 1;
	}
	if (path == NULL) {
		struct rlimit limit;
		getrlimit(RLIMIT_NOFILE, &limit);
		if (prom_gauge_set(prom_process_max_fds,
			limit.rlim_cur == RLIM_INFINITY ? -1 : limit.rlim_cur, NULL))
		{
			return 2;
		}
		return 0;
	}

	char *line = NULL;
	size_t len = 0;
	ssize_t slen;
	char *p;
	double val;
	bool found = false;
	FILE *f = fopen(path, "r");

	if (f == NULL) {
		perror(path);
		return 1;
	}
	while ((slen = getline(&line, &len, f)) != -1) {
		if (line[slen-1] != '\n')
			continue;
		if (strncmp(line, "Max open files  ", 16) != 0)
			continue;
		for (p = line + 16; *p != '\n'; p++)
			if (*p != ' ')
				break;
		if (*p == '\n')
			break;
		found = true;
		val = (strncmp(p, "unlimited  ", 11) == 0) ? -1 : strtoul(p, NULL, 10);
	}
	fclose(f);
	if (! found)
		return 1;
	return prom_gauge_set(prom_process_max_fds, val, NULL);
}
