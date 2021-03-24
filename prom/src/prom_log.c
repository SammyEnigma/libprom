/**
 * Copyright 2021 Jens Elkner <jel+libprom@cs.uni-magdeburg.de>.
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
#include <unistd.h>

#include "prom_log.h"

#ifdef PROM_LOG_ENABLE

#define MAX_MSG_LEN 1024

static char LVL_TXT[][6] = { "", "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };
static FILE *stream = NULL;
static int stream_fd = -1;
static PROM_LOG_LEVEL lvl = PLL_NONE;

FILE *
prom_log_use(FILE *dst) {
	int fdold, fdnew;
	FILE *fnew, *fold;

	if (dst == NULL)
		return NULL;
	fdold = fileno(dst);
	if (fdold == -1)
		return NULL;
	fdnew = dup(fdold);
	if (fdnew == -1)
		return NULL;
	fnew = fdopen(fdnew, "a");
	if (fnew == NULL) {
		close(fdnew);
		return NULL;
	}
	fold = stream;
	stream = fnew;
	stream_fd = fdnew;
	return fold;
}

PROM_LOG_LEVEL
prom_log_level(PROM_LOG_LEVEL level) {
	PROM_LOG_LEVEL old;
	if (level == PLL_NONE)
		return lvl;
	old = lvl;
	lvl = level;
	return old;
}

PROM_LOG_LEVEL
prom_log_level_parse(const char *level) {
	if (level == NULL)
		return 0;
	// allow 1..5 as well
	if (strlen(level) == 1) {
		int c = level[0] - '0';
		return  (c > 0 && c < PLL_COUNT) ? c : PLL_NONE;
	}
	if (strncmp("DEBUG", level, 6) == 0) {
		return PLL_DBG;
	} else if (strncmp("INFO", level, 5) == 0) {
		return PLL_INFO;
	} else if (strncmp("WARN", level, 5) == 0) {
		return PLL_WARN;
	} else if (strncmp("ERROR", level, 6) == 0) {
		return PLL_ERR;
	} else if (strncmp("FATAL", level, 6) == 0) {
		return PLL_FATAL;
	}
	return PLL_NONE;
}

void
prom_log(PROM_LOG_LEVEL level, const char* format, ...) {
	char s[MAX_MSG_LEN];
	size_t slen;
	va_list args;

	if (lvl == PLL_NONE) {
		char *s = getenv("PROM_LOG_LEVEL");
		if ((lvl = prom_log_level_parse(s)) == PLL_NONE)
			lvl = PLL_INFO;
	}

	if (level < lvl)
		return;

	// just because of Linux: stderr is not a constant ...
	if (stream == NULL)
		stream = stderr;

	va_start(args,format);

	slen = snprintf(s, sizeof(s) - 2, "%s: ", LVL_TXT[level]);
	slen += vsnprintf(s + slen, sizeof(s) - 2 - slen, format, args);
	fwrite(s, slen, 1, stream);
	fputc('\n', stream);
	fflush(stream);			// flush to page cache
	fsync(stream_fd);		// flush to disk

	va_end(args);
}

#endif  // PROM_LOG_ENABLE
