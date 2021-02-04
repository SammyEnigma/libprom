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

#ifndef PROM_METRIC_HISTOGRAM_SAMPLE_I_H
#define PROM_METRIC_HISTOGRAM_SAMPLE_I_H

// Public
#include "prom_metric_sample_histogram.h"

// Private
#include "prom_metric_sample_histogram_t.h"

/**
 * @brief PRIVATE Create a pointer to a pms_histogram_t
 */
pms_histogram_t *pms_histogram_new(const char *name, phb_t *buckets, size_t label_count, const char **label_keys, const char **label_vales);

/**
 * @brief PRIVATE Destroy a pms_histogram_t
 */
int pms_histogram_destroy(pms_histogram_t *self);

/**
 * @brief PRIVATE Cast to a pms_histogram_t* and destroy.
 */
int pms_histogram_destroy_generic(void *gen);

char *bucket_to_str(double bucket);

void pms_histogram_free_generic(void *gen);

#endif  // PROM_METRIC_HISTOGRAM_SAMPLE_I_H
