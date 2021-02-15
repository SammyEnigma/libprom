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

#ifndef PROM_PROCESS_I_H
#define PROM_PROCESS_I_H

#include "prom_process_limits_t.h"

/** @brief Setup the global gauge instance to track max open files limit. */
int ppl_init(void);

/** @brief Destroy the global gauge instance used to track max open files limit.
 */
void ppl_cleanup(void);

/** @brief Determine the current max open files limit and update the global
 * gauage instance accordingly.
 * @return \c 0 on success, a value != 0 if the update of the gauge failed.
 */
int ppl_update(const char *path);

#endif  // PROM_PROCESS_I_H
