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

#include <sys/types.h>
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
 * @brief The function to use to cleanup and free custom data attached via
 * prom_collector_data_set(). Per default it gets called by
 * \c prom_collector_destroy() right after the prom_collect_fn() has been set
 * to \c NULL. The default implementation simply sets the pointer to the data
 * to \c NULL, which may cause memory leaks.
 */
typedef void prom_collector_free_data_fn(prom_collector_t *self);

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
 * @param pid	If the given \c limits_path or \c stat_path is \c NULL, collect
 *	the data from the process with the given \c pid . If \c pid is < 1, the
 *	process id of the running process will be used.
 * @param label_keys	An arbitrary set of labels to assign to all metrics
 *	managed by the created collector. Use \c NULL for none.
 * @param label_vals	The values to use for the given label_keys. Same order
 *	as label_keys is required.  Use \c NULL for none.
 * @return The new collector on success, \c NULL otherwise.
 */
prom_collector_t *ppc_new(const char *limits_path, const char *stat_path, pid_t pid, const char **label_keys, const char **label_vals);

/**
 * @brief Destroy the given collector including all attached metrics.
 * @param self collector to destroy.
 * @return A non-zero integer value upon failure, 0 otherwise.
 * @note No matter what gets returned, you should never use any collector
 *	passed to this function but set it to \c NULL . Also remember, that per
 *	default all metrics of the given collector get freed via
 *	\c prom_gauge_destroy() or \c prom_counter_destroy() and should not be
 *	used anymore.
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
 * @brief Add the given metric to the given collector. It gets automatically
 *	destroyed when the given collector gets destroyed.
 * @param self Where to add the metric.
 * @param metric Metric to add. \c NULL is allowed and gets silently ignored.
 * @return A non-zero integer value upon failure, \c 0 otherwise.
 * @see \c prom_collector_destroy().
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

/**
 * @brief Attach custom data to the given collector as well as the callback to
 * use to clean it up.
 *
 * @param self	Where to attach data and cleanup function.
 * @param data	A pointer to custom data to attach. It gets not used by the
 *	framework itself, just piggybacked to the collector as is.
 * @param fn	Function, which should be used to free/cleanup the given custom
 *	data. If not set or \c NULL, the default implementation will be used, which
 *	just sets the related pointer to \c NULL, which might cause memory leaks.
 *	It gets automagically called by prom_collector_destroy().
 * @return A pointer to the data currently attached to the collector or \c NULL
 *	if not yet set.
 */
void *prom_collector_data_set(prom_collector_t *self, void *data, prom_collector_free_data_fn *fn);

/**
 * @brief Get the pointer to the custom data attached to the given collector.
 *
 * @param self	The collector in question.
 * @return a pointer to custom data, which might be \c NULL.
 */
void *prom_collector_data_get(prom_collector_t *self);

/**
 * @brief Get a map of all metrics of the given collector keyed by their names.
 * Per default this function will be used, if no collect function has been set.
 * @param self	The collector in question.
 * @see \c prom_collector_set_collect_fn()
 */
prom_map_t *prom_collector_metrics_get(prom_collector_t *self);

#endif  // PROM_COLLECTOR_H
