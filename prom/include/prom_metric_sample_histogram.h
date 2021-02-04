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
 * @file prom_metric_sample_histogram.h
 * @brief Functions for interacting with histogram metric samples directly
 */

#ifndef PROM_METRIC_SAMPLE_HISOTGRAM_H
#define PROM_METRIC_SAMPLE_HISOTGRAM_H

struct pms_histogram;
/**
 * @brief A histogram metric sample.
 */
typedef struct pms_histogram pms_histogram_t;

/**
 * @brief Find the bucket for the given value in the given prom sample
 *	metric histogram and increment the sample assigned to it by 1. Furthermore
 *	update its sum and count sample, if found and appropriate.
 * @param self		Where to lockup the bucket and sample.
 * @param value		The value to find.
 * @return Non-zero integer value upon failure, \c 0 otherwise.
 */
int pms_histogram_observe(pms_histogram_t *self, double value);

#endif  // PROM_METRIC_SAMPLE_HISOTGRAM_H
