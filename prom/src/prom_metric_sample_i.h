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

#include "prom_metric_sample_t.h"
#include "prom_metric_t.h"

#ifndef PROM_METRIC_SAMPLE_I_H
#define PROM_METRIC_SAMPLE_I_H

/**
 * @brief PRIVATE Return a pms_t*
 *
 * @param type The type of metric sample
 * @param l_value The entire left value of the metric e.g metric_name{foo="bar"}
 * @param r_value A double representing the value of the sample
 */
pms_t *pms_new(prom_metric_type_t type, const char *l_value, double r_value);

/**
 * @brief PRIVATE Destroy the pms
 */
int pms_destroy(pms_t *self);

/**
 * @brief PRIVATE A pll_free_item_fn to enable item destruction
 *	within a linked list's destructor
 */
int pms_destroy_generic(void *);

/**
 * @brief PRIVATE A pll_free_item_fn to enable item destruction
 *	within a linked list's destructor.
 *
 * This function ignores any errors.
 */
void pms_free_generic(void *gen);

#endif  // PROM_METRIC_SAMPLE_I_H
