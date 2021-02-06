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

#ifndef PROM_COUNTER_H
#define PROM_COUNTER_H

#include <stdlib.h>

#include "prom_metric.h"

/**
 * @file prom_counter.h
 * @brief https://prometheus.io/docs/concepts/metric_types/#counter
 */

/**
 * @brief Prometheus metric: counter
 *
 * References
 * * See https://prometheus.io/docs/concepts/metric_types/#counter
 */
typedef prom_metric_t prom_counter_t;

/**
 * @brief Construct a new metric of type \c counter (or short: counter).
 * @param name	Name of the counter.
 * @param help	Short counter description.
 * @param label_key_count	The number of labels associated with the given
 *	counter. Pass \c 0 if the counter does not require labels.
 * @param label_keys A collection of label keys. The number of keys MUST match
 *	the value passed as \c label_key_count. If no labels are required, pass
 *	\c NULL. Otherwise, it may be convenient to pass this value as a literal.
 * @return The new prom counter on success, \c NULL otherwise.
 *
 * *Example*
 *
 *	// An example with labels
 *	prom_counter_new("foo", "foo is a counter with labels", 2, (const char**) { "one", "two" });
 *
 *	// An example without labels
 *	prom_counter_new("foo", "foo is a counter without labels", 0, NULL);
 */
prom_counter_t *prom_counter_new(const char *name, const char *help, size_t label_key_count, const char **label_keys);

/**
 * @brief Destroys the given counter.
 * @param self	Counter to destroy.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 * @note No matter what gets returned, you should never use any metric
 *	passed to this function but set it to \c NULL .
 */
int prom_counter_destroy(prom_counter_t *self);

/**
 * @brief Increment the given counter by 1.
 * @param self	counter to increment.
 * @param label_values	The label values associated with the counter sample
 *	being updated. The number of labels must match the value passed as
 *	\c label_key_count in the counter's constructor. If no label values are
 *	necessary, pass \c NULL. Otherwise, it may be convenient to pass this value
 *	as a literal.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 *
 * *Example*
 *
 *	// An example with labels
 *	prom_counter_inc(foo_counter, (const char**) { "bar", "bang" });
 *
 *	// An example without labels
 *	prom_counter_inc(foo_counter, NULL);
 */
int prom_counter_inc(prom_counter_t *self, const char **label_values);

/**
 * @brief Add the value to the give counter.
 * @param self	Where to add the value.
 * @param r_value	Value to add. MUST be >= 0.
 * @param label_values	The label values associated with the counter sample
 *	being updated. The number of labels must match the value passed as
 *	\c label_key_count in the counter's constructor. If no label values are
 *	necessary, pass \c NULL. Otherwise, it may be convenient to pass this value
 *	as a literal.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 *
 * *Example*
 *
 *	// An example with labels
 *	prom_counter_add(foo_counter, 22, (const char**) { "bar", "bang" });
 *
 *	// An example without labels
 *	prom_counter_add(foo_counter, 22, NULL);
 */
int prom_counter_add(prom_counter_t *self, double r_value, const char **label_values);

/**
 * @brief Reset the given counter to the given value.
 * @param self	Where to set the given value.
 * @param r_value	Value to set. MUST be >= 0.
 * @param label_values	The label values associated with the counter sample
 *	being updated. The number of labels must match the value passed as
 *	\c label_key_count in the counter's constructor. If no label values are
 *	necessary, pass \c NULL. Otherwise, it may be convenient to pass this value
 *	as a literal.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 */
int prom_counter_reset(prom_counter_t *self, double r_value, const char **label_values);

#endif  // PROM_COUNTER_H
