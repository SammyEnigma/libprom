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
 * @file prom_histogram.h
 * @brief https://prometheus.io/docs/concepts/metric_types/#histogram
 */

#ifndef PROM_HISTOGRAM_INCLUDED
#define PROM_HISTOGRAM_INCLUDED

#include <stdlib.h>

#include "prom_histogram_buckets.h"
#include "prom_metric.h"

/**
 * @brief Prometheus metric: histogram
 *
 * References
 * * See https://prometheus.io/docs/concepts/metric_types/#histogram
 */
typedef prom_metric_t prom_histogram_t;

/**
 * @brief Construct a new metric of type \c histogram (or short: histogram)
 * @param name Name of the histogram.
 * @param help Sort histogram description.
 * @param buckets	Buckets to assign. See prom_histogram_buckets.h .
 * @param label_key_count	The number of labels associated with the given
 *	histogram. Pass \c 0 if the histogram does not require labels.
 * @param label_keys A collection of label keys. The number of keys MUST match
 *	the value passed as \c label_key_count. If no labels are required, pass
 *	\c NULL. Otherwise, it may be convenient to pass this value as a literal.
 * @return The new prom histogram on success, \c NULL otherwise.
 *
 * *Example*
 *
 *	// An example with labels
 *	phb_t* buckets = phb_linear(5.0, 5.0, 10);
 *	prom_histogram_new("foo", "foo is a histogram with labels", buckets, 2, (const char**) { "one", "two" });
 *
 *	// An example without labels
 *	phb_t* buckets = phb_linear(5.0, 5.0, 10);
 *	prom_histogram_new("foo", "foo is a histogram without labels", buckets, 0, NULL);
 */
prom_histogram_t *prom_histogram_new(const char *name, const char *help, phb_t *buckets, size_t label_key_count, const char **label_keys);

/**
 * @brief Destroy the given histogram.
 * @return Non-zero value upon failure, \c 0 otherwise.
 * @note No matter what gets returned, you should never use any metric
 *	passed to this function but set it to \c NULL .
 */
int prom_histogram_destroy(prom_histogram_t *self);

/**
 * @brief Observe the given value of the given histogram with the given labels.
 * @param self	Histogram to observe.
 * @param value Value to observe.
 * @param label_values	The label values associated with the histogram sample
 *	being updated. The number of labels must match the value passed as
 *	\c label_key_count in the histogram's constructor. If no label values are
 *	necessary, pass \c NULL. Otherwise, it may be convenient to pass this value
 *	as a literal.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 */
int prom_histogram_observe(prom_histogram_t *self, double value, const char **label_values);

#endif  // PROM_HISTOGRAM_INCLUDED
