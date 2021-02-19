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
#ifndef PROM_PROCESS_COLLECTOR_H
#define PROM_PROCESS_COLLECTOR_H

typedef enum proc_metric_t {
	PM_OPEN_FDS = 0,
	PM_MAX_FDS,
	PM_MINFLT,
	PM_MAJFLT,
#ifdef __sun
	PM_CPU_UTIL,
	PM_MEM_UTIL,
#else	// assume Linux
	PM_CMINFLT,
	PM_CMAJFLT,
#endif
	PM_UTIME,
	PM_STIME,
	PM_TIME,
	PM_CUTIME,
	PM_CSTIME,
	PM_CTIME,
	PM_NUM_THREADS,
	PM_STARTTIME,
	PM_VSIZE,
	PM_RSS,
#ifdef __sun
	PM_VCTX,
	PM_ICTX,
#else	// assume Linux
	PM_BLKIO,
#endif
	PM_COUNT /* required to be last */
} proc_metric_t;

typedef enum fd_t {
	FD_LIMITS = 0,
	FD_STAT,
#ifdef __sun
	FD_PSINFO,
	FD_USAGE,
#endif
	FD_COUNT /* required to be last */
} fd_t;


#define cup(what, val) \
	prom_counter_reset(m[(what)], (val), lvals) ? 0 : 1 << (what)

#define gup(what, val) \
	prom_gauge_set(m[(what)], (val), lvals) ? 0 : 1 << (what)

#endif // PROM_PROCESS_COLLECTOR_H
