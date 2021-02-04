/**
 * Copyright 2019-2020 DigitalOcean Inc.
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

// Private
#include "prom_metric_sample_histogram_t.h"
#include "prom_metric_t.h"

#ifndef PROM_METRIC_I_INCLUDED
#define PROM_METRIC_I_INCLUDED

/**
 * @brief PRIVATE Returns a *prom_metric
 */
prom_metric_t *prom_metric_new(prom_metric_type_t type, const char *name, const char *help, size_t label_key_count, const char **label_keys);

/**
 * @brief PRIVATE Destroys a *prom_metric
 */
int prom_metric_destroy(prom_metric_t *self);

/**
 * @brief PRIVATE Cast given item to *prom_metric_t and destroy it
 */
int prom_metric_destroy_generic(void *item);

/**
 * @brief PRIVATE Casts given item to *prom_metric_t and destroy it.
 */
void prom_metric_free_generic(void *item);

#endif  // PROM_METRIC_I_INCLUDED
