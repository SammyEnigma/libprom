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

#ifndef PROM_COLLECTOR_H
#define PROM_COLLECTOR_H

#include "prom_map.h"
#include "prom_metric.h"

/**
 * @file prom_collector.h
 * @brief A \c prom_collector is used to collect metrics.
 */

/**
 * @brief A prom collector calls collect to prepare metrics and return
 * them to the registry to which it is registered.
 */
typedef struct prom_collector prom_collector_t;

/**
 * @brief The function used to prepare and return all relevant metrics of the
 *	given collector ready for Prometheus exposition.
 *
 * If you use the default collector registry, this should not concern you. If
 * you are using a custom collector, you may set this function on your
 * collector to do additional work before returning the contained metrics.
 *
 * @param self The collector with the relevant metrics.
 * @return The metrics to expose.
 */
typedef prom_map_t *prom_collect_fn(prom_collector_t *self);

/**
 * @brief Create a collector
 * @param name	name of the collector.
 * @note	Name MUST NOT be \c default or \c process.
 * @return The new collector on success, \c NULL otherwise.
 */
prom_collector_t *prom_collector_new(const char *name);

/**
 * @brief Create a prom collector which includes the default process metrics.
 * @param limits_path	If \c NULL POSIX and OS specific will be used to
 *	determine limits. Otherwise, read the limits from the given file path -
 *	ususally used for testing, only.
 * @param stat_path		If \c NULL POSIX and OS specific will be used to
 *	determine the stats. Otherwise, read the stats from the given file path -
 *	ususally used for testing, only.
 * @return The new collector on success, \c NULL otherwise.
 */
prom_collector_t *prom_collector_process_new(const char *limits_path, const char *stat_path);

/**
 * @brief Destroy the given collector.
 * @param self collector to destroy.
 * @return A non-zero integer value upon failure, 0 otherwise.
 * @note No matter what gets returned, you should never use any collector
 *	passed to this function but set it to \c NULL .
 */
int prom_collector_destroy(prom_collector_t *self);


/**
 * @brief Cast the given pointer to \c prom_collector_t and call
 * \c prom_collector_destroy() with it.
 * @param gen Collector to destroy.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 */
int prom_collector_destroy_generic(void *gen);

/**
 * @brief	Same as \c prom_collector_destroy_generic(), but drops any return
 * 	codes.
 */
void prom_collector_free_generic(void *gen);

/**
 * @brief Add the given metric to the given collector
 * @param self Where to add the metric.
 * @param metric Metric to add.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 */
int prom_collector_add_metric(prom_collector_t *self, prom_metric_t *metric);

/**
 * @brief Set the function, which prepares (if needed) and returns all relevant
 * metrics of the given collector ready for Prometheus exposition.
 * @param self	Collector containing the metrics.
 * @param fn	The function to repare and return the metrics.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 */
int prom_collector_set_collect_fn(prom_collector_t *self, prom_collect_fn *fn);

#endif  // PROM_COLLECTOR_H
