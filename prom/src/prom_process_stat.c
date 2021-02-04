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
#include <unistd.h>

// Public
#include "prom_alloc.h"

// Private
#include "prom_assert.h"
#include "prom_process_stat_t.h"
#include "prom_procfs_i.h"

prom_gauge_t *prom_process_cpu_seconds_total = NULL;
prom_gauge_t *prom_process_virtual_memory_bytes = NULL;
prom_gauge_t *prom_process_rss_memory_bytes = NULL;
prom_gauge_t *prom_process_start_time_seconds = NULL;

/**
 * @brief Initializes each gauge metric
 */
int
pps_init(void) {
	// /proc/self/stat cutime + cstime / 100
	prom_process_cpu_seconds_total = prom_gauge_new("process_cpu_seconds_total",
		"Total user and system CPU time spent in seconds.", 0, NULL);

	// /proc/self/stat Field 23
	prom_process_virtual_memory_bytes =
		prom_gauge_new("process_virtual_memory_bytes",
			"Virtual memory size in bytes.", 0, NULL);

	// /proc/self/stat Field 24
	prom_process_rss_memory_bytes = prom_gauge_new("process_rss_memory_bytes",
		"Resident set size of memory in bytes.", 0, NULL);

	prom_process_start_time_seconds =
		prom_gauge_new("process_start_time_seconds",
			"Start time of the process since unix epoch in seconds.", 0, NULL);
	return 0;
}

void
pps_cleanup(void) {
	prom_gauge_destroy(prom_process_cpu_seconds_total);
	prom_process_cpu_seconds_total = NULL;
	prom_gauge_destroy(prom_process_virtual_memory_bytes);
	prom_process_virtual_memory_bytes = NULL;
	prom_gauge_destroy(prom_process_rss_memory_bytes);
	prom_process_rss_memory_bytes = NULL;
	prom_gauge_destroy(prom_process_start_time_seconds);
	prom_process_start_time_seconds = NULL;
}


pps_file_t *
pps_file_new(const char *path) {
	return prom_procfs_buf_new(path == NULL ? "/proc/self/stat" : path);
}

int
pps_file_destroy(pps_file_t *self) {
	return prom_procfs_buf_destroy(self);
}

pps_t *
pps_new(pps_file_t *stat_f) {
	pps_t *self = (pps_t *) prom_malloc(sizeof(pps_t));
	self->comm = prom_malloc(128);

	sscanf((const char *)stat_f->buf,
"%d "	// (1) pid  %d
"%s "	// (2) comm  %s
"%c "	// (3) state  %c
"%d "	// (4) ppid  %d
"%d "	// (5) pgrp  %d
"%d "	// (6) session  %d
"%d "	// (7) tty_nr  %d
"%d "	// (8) tpgid  %d
"%u "	// (9) flags  %u
"%lu "	// (10) minflt  %lu
"%lu "	// (11) cminflt  %lu
"%lu "	// (12) majflt  %lu
"%lu "	// (13) cmajflt  %lu
"%lu "	// (14) utime  %lu
"%lu "	// (15) stime  %lu
"%ld "	// (16) cutime  %ld
"%ld "	// (17) cstime  %ld
"%ld "	// (18) priority  %ld
"%ld "	// (19) nice  %ld
"%ld "	// (20) num_threads  %ld
"%ld "	// (21) itrealvalue  %ld
"%llu "	// (22) starttime  %llu
"%lu "	// (23) vsize  %lu
"%ld "	// (24) rss  %ld
"%lu "	// (25) rsslim  %lu
"%lu "	// (26) startcode  %lu  [PT]
"%lu "	// (27) endcode  %lu  [PT]
"%lu "	// (28) startstack  %lu  [PT]
"%lu "	// (29) kstkesp  %lu  [PT]
"%lu "	// (30) kstkeip  %lu  [PT]
"%lu "	// (31) signal  %lu
"%lu "	// (32) blocked  %lu
"%lu "	// (33) sigignore  %lu
"%lu "	// (34) sigcatch  %lu
"%lu "	// (35) wchan  %lu  [PT]
"%lu "	// (36) nswap  %lu
"%lu "	// (37) cnswap  %lu
"%d "	// (38) exit_signal  %d  (since Linux 2.1.22)
"%d "	// (39) processor  %d  (since Linux 2.2.8)
"%u "	// (40) rt_priority  %u  (since Linux 2.5.19)
"%u "	// (41) policy  %u  (since Linux 2.5.19)
"%llu "	// (42) delayacct_blkio_ticks  %llu  (since Linux 2.6.18)
"%lu "	// (43) guest_time  %lu  (since Linux 2.6.24)
"%ld "	// (44) cguest_time  %ld  (since Linux 2.6.24)
"%lu "	// (45) start_data  %lu  (since Linux 3.3)  [PT]
"%lu "	// (46) end_data  %lu  (since Linux 3.3)  [PT]
"%lu "	// (47) start_brk  %lu  (since Linux 3.3)  [PT]
"%lu "	// (48) arg_start  %lu  (since Linux 3.5)  [PT]
"%lu "	// (49) arg_end  %lu  (since Linux 3.5)  [PT]
"%lu "	// (50) env_start  %lu  (since Linux 3.5)  [PT]
"%lu "	// (51) env_end  %lu  (since Linux 3.5)  [PT]
"%d ",	// (52) exit_code  %d  (since Linux 3.5)  [PT]

		&self->pid,				// (1)
		self->comm,				// (2)
		&self->state,			// (3)
		&self->ppid,			// (4)
		&self->pgrp,			// (5)
		&self->session,			// (6)
		&self->tty_nr,			// (7)
		&self->tpgid,			// (8)
		&self->flags,			// (9)
		&self->minflt,			// (10)
		&self->cminflt,			// (11)
		&self->majflt,			// (12)
		&self->cmajflt,			// (13)
		&self->utime,			// (14)
		&self->stime,			// (15)
		&self->cutime,			// (16)
		&self->cstime,			// (17)
		&self->priority,		// (18)
		&self->nice,			// (19)
		&self->num_threads,		// (20)
		&self->itrealvalue,		// (21)
		&self->starttime,		// (22)
		&self->vsize,			// (23)
		&self->rss,				// (24)
		&self->rsslim,			// (25)
		&self->startcode,		// (26)
		&self->endcode,			// (27)
		&self->startstack,		// (28)
		&self->kstkesp,			// (29)
		&self->kstkeip,			// (30)
		&self->signal,			// (31)
		&self->blocked,			// (32)
		&self->sigignore,		// (33)
		&self->sigcatch,		// (34)
		&self->wchan,			// (35)
		&self->nswap,			// (36)
		&self->cnswap,			// (37)
		&self->exit_signal,		// (38)
		&self->processor,		// (39)
		&self->rt_priority,		// (40)
		&self->policy,			// (41)
		&self->blkio_ticks,		// (42)
		&self->guest_time,		// (43)
		&self->cguest_time,		// (44)
		&self->start_data,		// (45)
		&self->end_data,		// (46)
		&self->start_brk,		// (47)
		&self->arg_start,		// (48)
		&self->arg_end,			// (49)
		&self->env_start,		// (50)
		&self->env_end,			// (51)
		&self->exit_code		// (52)
	);

	return self;
}

int
pps_destroy(pps_t *self) {
	if (self == NULL)
		return 0;
	prom_free((void *) self->comm);
	self->comm = NULL;
	prom_free((void *) self);
	return 0;
}
