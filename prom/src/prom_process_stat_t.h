/**
 * Copyright 2019-2020 DigitalOcean Inc.
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

#ifndef PROM_PROCESS_STATS_T_H
#define PROM_PROCESS_STATS_T_H

#ifdef __linux
/**
 * @brief Refer to man proc and search for /proc/self/stat
 */
typedef struct stats {
									// Linux type	Expected final value
	int pid;						// (1) %d
	char *comm;						// (2) %s		16 byte + ()
	char state;						// (3) %c
	int ppid;						// (4) %d
	int pgrp;						// (5) %d
	int session;					// (6) %d
	int tty_nr;						// (7) %d
	int tpgid;						// (8) %d
	unsigned flags;					// (9) %u
	unsigned long minflt;			// (10) %lu
	unsigned long cminflt;			// (11) %lu
	unsigned long majflt;			// (12) %lu
	unsigned long cmajflt;			// (13) %lu
	double utime;					// (14) %lu		in seconds
	double stime;					// (15) %lu		in seconds
	double cutime;					// (16) %ld		in seconds
	double cstime;					// (17) %ld		in seconds
	long int priority;				// (18) %ld
	long int nice;					// (19) %ld
	long int num_threads;			// (20) %ld
	long int itrealvalue;			// (21) %ld
	unsigned long long starttime;	// (22) %llu	in seconds since Epoche
	unsigned long vsize;			// (23) %lu		in bytes
	long int rss;					// (24) %ld		in bytes
	unsigned long rsslim;			// (25) %lu
	unsigned long startcode;		// (26) %lu  [PT]
	unsigned long endcode;			// (27) %lu  [PT]
	unsigned long startstack;		// (28) %lu  [PT]
	unsigned long kstkesp;			// (29) %lu  [PT]
	unsigned long kstkeip;			// (30) %lu  [PT]
	unsigned long signal;			// (31) %lu
	unsigned long blocked;			// (32) %lu
	unsigned long sigignore;		// (33) %lu
	unsigned long sigcatch;			// (34) %lu
	unsigned long wchan;			// (35) %lu  [PT]
	unsigned long nswap;			// (36) %lu
	unsigned long cnswap;			// (37) %lu
	int exit_signal;				// (38) %d  (since Linux 2.1.22)
	int processor;					// (39) %d  (since Linux 2.2.8)
	unsigned rt_priority;			// (40) %u  (since Linux 2.5.19)
	unsigned policy;				// (41) %u  (since Linux 2.5.19)
	unsigned long long blkio;		// (42) %llu delayacct_blkio_ticks
	unsigned long guest_time;		// (43) %lu  (since Linux 2.6.24)
	long int cguest_time;			// (44) %ld  (since Linux 2.6.24)
	unsigned long start_data;		// (45) %lu  (since Linux 3.3)  [PT]
	unsigned long end_data;			// (46) %lu  (since Linux 3.3)  [PT]
	unsigned long start_brk;		// (47) %lu  (since Linux 3.3)  [PT]
	unsigned long arg_start;		// (48) %lu  (since Linux 3.5)  [PT]
	unsigned long arg_end;			// (49) %lu  (since Linux 3.5)  [PT]
	unsigned long env_start;		// (50) %lu  (since Linux 3.5)  [PT]
	unsigned long env_end;			// (51) %lu  (since Linux 3.5)  [PT]
	int exit_code;					// (52) %d  (since Linux 3.5)  [PT]
} stats_t;
#endif

#endif  // PROM_PROCESS_STATS_T_H
