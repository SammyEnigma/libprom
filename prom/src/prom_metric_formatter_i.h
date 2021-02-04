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

// Reference: https://prometheus.io/docs/instrumenting/exposition_formats/

#ifndef PROM_METRIC_FORMATTER_I_H
#define PROM_METRIC_FORMATTER_I_H

#include <stdbool.h>

// Private
#include "prom_metric_formatter_t.h"
#include "prom_metric_t.h"

/**
 * @brief PRIVATE prom_metric_formatter constructor
 */
pmf_t *pmf_new();

/**
 * @brief PRIVATE prom_metric_formatter destructor
 */
int pmf_destroy(pmf_t *self);

/**
 * @brief PRIVATE Loads the help text
 */
int pmf_load_help(pmf_t *self, const char *prefix, const char *name, const char *help);

/**
 * @brief PRIVATE Loads the type text
 */
int pmf_load_type(pmf_t *self, const char *prefix, const char *name, prom_metric_type_t metric_type);

/**
 * @brief PRIVATE Loads the formatter with a metric sample L-value
 * @param name The metric name
 * @param suffix The metric suffix for Summary and Histogram metric types.
 * @param label_count The number of labels for the given metric.
 * @param label_keys An array of constant strings.
 * @param label_values An array of constant strings.
 *
 * The number of const char **and prom_label_value must be the same.
 */
int pmf_load_l_value(pmf_t *metric_formatter, const char *name, const char *suffix, size_t label_count, const char **label_keys, const char **label_values);

/**
 * @brief PRIVATE Loads the formatter with a metric sample
 */
int pmf_load_sample(pmf_t *metric_formatter, pms_t *sample, const char *prefix);

/**
 * @brief PRIVATE Loads a metric in the string exposition format
 */
int pmf_load_metric(pmf_t *self, prom_metric_t *metric, const char *prefix, bool compact);

/**
 * @brief PRIVATE Loads the given metrics
 */
int pmf_load_metrics(pmf_t *self, prom_map_t *collectors, prom_metric_t *scrape_metric, const char *prefix, bool compact);

/**
 * @brief PRIVATE Clear the underlying string_builder
 */
int pmf_clear(pmf_t *self);

/**
 * @brief PRIVATE Returns the string built by prom_metric_formatter
 */
char *pmf_dump(pmf_t *metric_formatter);

#endif  // PROM_METRIC_FORMATTER_I_H
