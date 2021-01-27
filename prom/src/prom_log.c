/**
 * Copyright 2020 Jens Elkner <jel+libprom@cs.uni-magdeburg.de>.
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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "prom_log.h"

#ifdef PROM_LOG_ENABLE

#define MAX_MSG_LEN 1024

static char LVL_TXT[][6] = { "", "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };

void
prom_log(PROM_LOG_LEVEL level, const char* format, ...) {
	static PROM_LOG_LEVEL lvl = 0;
	char s[MAX_MSG_LEN];
	size_t slen;
	va_list args;

	if (lvl == 0) {
		char *s = getenv("PROM_LOG_LEVEL");
		if (s == NULL) {
			lvl = PLL_INFO;
		} else if (strncmp("DEBUG", s, 6) == 0) {
			lvl = PLL_DBG;
		} else if (strncmp("WARN", s, 6) == 0) {
			lvl = PLL_WARN;
		} else if (strncmp("ERROR", s, 6) == 0) {
			lvl = PLL_ERR;
		} else if (strncmp("FATAL", s, 6) == 0) {
			lvl = PLL_FATAL;
		} else {
			lvl = PLL_INFO;
		}
	}

	if (level < lvl)
		return;

	va_start(args,format);

	slen = snprintf(s, sizeof(s) - 2, "%s: ", LVL_TXT[level]);
	slen += vsnprintf(s + slen, sizeof(s) - 2 - slen, format, args);
	fwrite(s, slen, 1, stderr);
	fputc('\n', stderr);

	va_end(args);
}

#endif  // PROM_LOG_ENABLE
