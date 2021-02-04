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
 * @file prom_gauge.h
 * @brief https://prometheus.io/docs/concepts/metric_types/#gauge
 */

#ifndef PROM_GAUGE_H
#define PROM_GAUGE_H

#include <stdlib.h>

#include "prom_metric.h"

/**
 * @brief Prometheus metric: gauge
 *
 * References
 * * See https://prometheus.io/docs/concepts/metric_types/#gauge
 */
typedef prom_metric_t prom_gauge_t;

/**
 * @brief Construct a new metric of type \c gauge (or short: gauge).
 * @param name Name of the gauge.
 * @param help Short gauge description.
 * @param label_key_count   The number of labels associated with the given
 *	gauge. Pass \c 0 if the gauge does not require labels.
 * @param label_keys A collection of label keys. The number of keys MUST match
 *  the value passed as \c label_key_count. If no labels are required, pass
 *  \c NULL. Otherwise, it may be convenient to pass this value as a literal.
 * @return The new gauge on success, \c NULL otherwise.
 *
 *	// An example with labels
 *	prom_gauge_new("foo", "foo is a gauge with labels", 2, (const char**) { "one", "two" });
 *
 *	// An example without labels
 *	prom_gauge_new("foo", "foo is a gauge without labels", 0, NULL);
 */
prom_gauge_t *prom_gauge_new(const char *name, const char *help, size_t label_key_count, const char **label_keys);

/**
 * @brief Destroys the given gauge.
 * @param self	Gauge to destroy.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 * @note No matter what gets returned, you should never use any metric
 *	passed to this function but set it to \c NULL .
 */
int prom_gauge_destroy(prom_gauge_t *self);

/**
 * @brief Increment the given gauge by 1.
 * @param self	Gauge to increment.
 * @param label_values	The label values associated with the gauge sample
 *	being updated. The number of labels must match the value passed as
 *	\c label_key_count in the gauge's constructor. If no label values are
 *	necessary, pass \c NULL. Otherwise, it may be convenient to pass this value
 *	as a literal.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 *
 * *Example*
 *
 *	// An example with labels
 *	prom_gauge_inc(foo_gauge, (const char**) { "bar", "bang" });
 *
 *	// An example without labels
 *	prom_gauge_inc(foo_gauge, NULL);
 */
int prom_gauge_inc(prom_gauge_t *self, const char **label_values);

/**
 * @brief Decrement the given gauge by 1.
 * @param self	Gauge to decrement.
 * @param label_values	The label values associated with the gauge sample
 *	being updated. The number of labels must match the value passed as
 *	\c label_key_count in the gauge's constructor. If no label values are
 *	necessary, pass \c NULL. Otherwise, it may be convenient to pass this value
 *	as a literal.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 *
 * *Example*
 *
 *	// An example with labels
 *	prom_gauge_dec(foo_gauge, (const char**) { "bar", "bang" });
 *
 *	// An example without labels
 *	prom_gauge_dec(foo_gauge, NULL);
 */
int prom_gauge_dec(prom_gauge_t *self, const char **label_values);

/**
 * @brief Add the given value to the given gauge.
 * @param self	Where to add the given value.
 * @param r_value	Value to add. MUST be >= 0.
 * @param label_values	The label values associated with the gauge sample
 *	being updated. The number of labels must match the value passed as
 *	\c label_key_count in the gauge's constructor. If no label values are
 *	necessary, pass \c NULL. Otherwise, it may be convenient to pass this value
 *	as a literal.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 *
 * *Example*
 *
 *	// An example with labels
 *	prom_gauge_add(foo_gauge 22, (const char**) { "bar", "bang" });
 *
 *	// An example without labels
 *	prom_gauge_add(foo_gauge, 22, NULL);
 */
int prom_gauge_add(prom_gauge_t *self, double r_value, const char **label_values);

/**
 * @brief Subtract the value to the given gauge.
 * @param self	Where to substract the given value.
 * @param r_value	Value to substract (which might be < 0).
 * @param label_values	The label values associated with the gauge sample
 *	being updated. The number of labels must match the value passed as
 *	\c label_key_count in the gauge's constructor. If no label values are
 *	necessary, pass \c NULL. Otherwise, it may be convenient to pass this value
 *	as a literal.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 *
 * *Example*
 *
 *	// An example with labels
 *	prom_gauge_sub(foo_gauge 22, (const char**) { "bar", "bang" });
 *
 *	// An example without labels
 *	prom_gauge_sub(foo_gauge, 22, NULL);
 */
int prom_gauge_sub(prom_gauge_t *self, double r_value, const char **label_values);

/**
 * @brief Set the given gauge to the given value.
 * @param self	Where to set the given value.
 * @param r_value	Value to set (which might be < 0).
 * @param label_values	The label values associated with the gauge sample
 *	being updated. The number of labels must match the value passed as
 *	\c label_key_count in the gauge's constructor. If no label values are
 *	necessary, pass \c NULL. Otherwise, it may be convenient to pass this value
 *	as a literal.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 *
 * *Example*
 *
 *	// An example with labels
 *	prom_gauge_set(foo_gauge 22, (const char**) { "bar", "bang" });
 *
 *	// An example without labels
 *	prom_gauge_set(foo_gauge, 22, NULL);
 */
int prom_gauge_set(prom_gauge_t *self, double r_value, const char **label_values);

#endif  // PROM_GAUGE_H
