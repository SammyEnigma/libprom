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

#ifndef PROM_PROCESS_STATS_I_H
#define PROM_PROCESS_STATS_I_H

#include "prom_process_stat_t.h"

pps_file_t *pps_file_new(const char *path);
int pps_file_destroy(pps_file_t *self);
pps_t *pps_new(pps_file_t *stat_f);
int pps_destroy(pps_t *self);
int pps_init(void);
void pps_cleanup(void);

#endif  // PROM_PROCESS_STATS_I_H
