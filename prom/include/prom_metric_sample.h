/*
Copyright 2019-2020 DigitalOcean Inc.
Copyright 2021 Jens Elkner <jel+libprom@cs.uni-magdeburg.de>


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
 * @file prom_metric_sample.h
 * @brief Functions for interfacting with metric samples directly
 */

#ifndef PROM_METRIC_SAMPLE_H
#define PROM_METRIC_SAMPLE_H

struct pms;
/**
 * @brief Contains the specific metric and value given the name and label set
 * @note All operations MUST be called on a sample derived from a gauge metric.
 * Reference: https://prometheus.io/docs/concepts/data_model/#metric-names-and-labels
 */
typedef struct pms pms_t;

/**
 * @brief Add the given r_value to the given sample.
 * @param self		Where to add the given value.
 * @param r_value	Value to add. Must be >= 0.
 * @return Non-zero integer value upon failure, \c 0 otherwise.
 */
int pms_add(pms_t *self, double r_value);

/**
 * @brief Subtract the given r_value from the given sample.
 * @param self		Where to add the given value.
 * @param r_value	Value to substract.
 * @return Non-zero integer value upon failure, \c 0 otherwise.
 */
int pms_sub(pms_t *self, double r_value);

/**
 * @brief Set the given r_value to the given ample.
 * @param self		Where to set the given value.
 * @param r_value	Value to set.
 * @return Non-zero integer value upon failure, \c 0 otherwise.
 */
int pms_set(pms_t *self, double r_value);

#endif  // PROM_METRIC_SAMPLE_H
