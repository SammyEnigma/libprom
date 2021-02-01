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
 * @file prom_collector_registry.h
 * @brief The collector registry registers collectors for metric exposition.
 */

#ifndef PROM_REGISTRY_H
#define PROM_REGISTRY_H

#include <stdbool.h>
#include "prom_collector.h"
#include "prom_metric.h"

/** @brief	Reserved label value for libprom's own scrape duration metrics.
	@note	Do not use unless you know, what you are doing. */
#define METRIC_LABEL_SCRAPE "libprom"
/** @brief	Reserved name for libprom's own scrape duration metric.
	@note Do not use unless you know, what you are doing. */
#define METRIC_NAME_SCRAPE "scrape_duration_seconds"
/** @brief	Reserved name for libprom's own default prom collector, where
		usually new metrics get attached.
	@note	Do not use unless you know, what you are doing. */
#define COLLECTOR_NAME_DEFAULT "default"
/** @brief	Reserved name for libprom's own process stats prom collector.
	@note	Do not use unless you know, what you are doing. */
#define COLLECTOR_NAME_PROCESS "process"
/** @brief	Reserved name for libprom's own default prom collector registry.
	@note Do not use unless you know, what you are doing. */
#define REGISTRY_NAME_DEFAULT "default"

/**
 * @brief flags for the setup of a prom collector registry.
 * @see \c prom_collector_registry_init()
 */
enum prom_init_flag {
	/** placeholder for \c 0 - implies nothing */
	PROM_NONE = 0,
	/** Automatically setup and attach a \c process collector, which collects
		stats of this process on \c prom_collector_registry_bridge() and
		appends its to the output. */
	PROM_PROCESS = 1,
	/** Automatically create and use a metric to monitor the time needed to
		dump all the metrics of all registered collectors in Prometheus
		exposition format and include it in the dump as well. The metric gets
		labeled as \c METRIC_LABEL_SCRAPE , the metric's implied name is
		\c METRIC_NAME_SCRAPE . */
	PROM_SCRAPETIME = 2,
	/** Implies \c PROM_SCRAPETIME, but measures and dumps the dump time of
		every single collector as well. The related metrics get labeled with
		the name of the collector. */
	PROM_SCRAPETIME_ALL = 4
};

/** @brief collection of prom collector registry features.
	@see \c prom_init_flag
 */
typedef unsigned int PROM_INIT_FLAGS;

/**
 * @brief A prom_registry_t is responsible for registering metrics and briding them to the string exposition format
 */
typedef struct prom_collector_registry prom_collector_registry_t;

/**
 * @brief Initialize the default registry by calling
 *	prom_collector_registry_init() within your program.
 * @note You MUST NOT modify this value.
 */
extern prom_collector_registry_t *PROM_COLLECTOR_REGISTRY;
/** @brief backward compatibility to 0.1.3 - will vanish soon. */
#define PROM_COLLECTOR_REGISTRY_DEFAULT PROM_COLLECTOR_REGISTRY

/**
 * @brief Initializes the default collector registry
 *	\c PROM_COLLECTOR_REGISTRY and enables metric collection on the executing
 *	process and sets the metric name prefix to \c METRIC_LABEL_SCRAPE + "_".
 *	Same as \c prom_collector_registry_init(PROM_PROCESS|PROM_SCRAPETIME,
 *		METRIC_LABEL_SCRAPE "_").
 * @return A non-zero integer value upon failure
 */
int prom_collector_registry_default_init(void);

/**
 * @brief Initializes the default collector registry
 *	\c PROM_COLLECTOR_REGISTRY named \c REGISTRY_NAME_DEFAULT.
 * @param features	If \c 0, a prom collector registry gets created, which
 *	just contains a single empty collector named \c COLLECTOR_NAME_DEFAULT
 *	where per default all new metrics get attached.
 * @param mprefix	If not \c NULL, prefix each metric's name with this
 *	string when metrics get exposed. E.g. one may use "appname_".
 * @return A non-zero integer value upon failure  - the registry is unusabele.
 */
int prom_collector_registry_init(PROM_INIT_FLAGS features, const char *mprefix);

/**
 * @brief Constructs a prom_collector_registry_t* with one empty collector
 *	named \c default .
 * @param name The name of the collector registry. It MUST NOT be \c default.
 * @return The constructed prom_collector_registry_t*
 */
prom_collector_registry_t *prom_collector_registry_new(const char *name);

/**
 * @brief Destroy the given collector registry.
 * @param self The prom_collector_registry_t* to destroy.
 * @return A non-zero integer value upon failure, 0 otherwise. NOTE: no matter
 *	what is returned, one should always set the pointer of the given registry
 *	to \c NULL because it gets always passed to \c free() and thus points to
 *	an invalid memory location on return.
 */
int prom_collector_registry_destroy(prom_collector_registry_t *self);

/**
 * @brief Enable process metrics on the given collector registry
 * @param self The target prom_collector_registry_t*
 * @return A non-zero integer value upon failure
 */
int prom_collector_registry_enable_process_metrics(prom_collector_registry_t *self);

/**
 * @brief Create a scrape duration gauge metric and attach it to the given
 *	prom collector registry. If available, \c prom_collector_registry_bridge()
 *	measures the time needed to collect and export all metrics of the registry,
 *	updates the metric and appends it to the export.
 * @param self The target prom_collector_registry_t*
 * @return A non-zero integer if the given registry is \c NULL, or the metric
 *	could not be added to its \c default collector, 0 otherwise.
 */
int prom_collector_registry_enable_scrape_metrics(prom_collector_registry_t *self);

/**
 * @brief Registers a metric with the default collector on
 *	PROM_COLLECTOR_REGISTRY.
 *
 * The metric to be registered MUST NOT already be registered with the given.
 * If so, the function calls exit. It returns a prom_metric_t* to simplify
 * metric creation and registration. Furthermore, PROM_COLLECTOR_REGISTRY must
 * be registered via prom_collector_registry_init() prior to calling
 * this function. The metric will be added to the default registry's default
 * collector.
 *
 * @param metric The metric to register on PROM_COLLECTOR_REGISTRY*
 * @return The registered prom_metric_t*
 */
prom_metric_t *prom_collector_registry_must_register_metric(prom_metric_t *metric);

/**
 * @brief Registers a metric with the default collector on
 *	PROM_COLLECTOR_REGISTRY.
 *
 * See prom_collector_registry_must_register_metric.
 *
 * @param metric The metric to register on PROM_COLLECTOR_REGISTRY*
 * @return A non-zero integer value upon failure.
 */
int prom_collector_registry_register_metric(prom_metric_t *metric);

/**
 * @brief Register a collector with the given registry. If the registry already
 *	contains a collector with the same name, the registration will fail.
 * @param self The target prom_collector_registry_t* instance.
 * @param collector The prom_collector_t* to register onto the
 *	prom_collector_registry_t* as self.
 * @return A non-zero integer value upon failure.
 */
int prom_collector_registry_register_collector(prom_collector_registry_t *self, prom_collector_t *collector);

/**
 * @brief Get a reference to the prom collector with the given \a name from
 * the given prom collector registry.
 * @param self The target prom_collector_registry_t*.
 * @param name The name of the collector to lookup.
 * @return \c NULL if not found, a reference to the related prom collector
 *	otherwise.
 */
prom_collector_t *prom_collector_registry_get(prom_collector_registry_t *self, const char *name);

/**
 * @brief Returns a string in the default metric exposition format. The string MUST be freed to avoid unnecessary heap
 * memory growth.
 *
 * Reference: https://prometheus.io/docs/instrumenting/exposition_formats/
 *
 * @param self The target prom_collector_registry_t*
 * @return The string in the default metric exposition format.
 */
const char *prom_collector_registry_bridge(prom_collector_registry_t *self);

/**
 *@brief Validates that the given metric name complies with the specification:
 *
 * Reference: https://prometheus.io/docs/concepts/data_model/#metric-names-and-labels
 *
 * Returns a non-zero integer value on failure.
 *
 * @param self The target prom_collector_registry_t*
 * @param metric_name The metric name to validate
 * @return A non-zero integer value upon failure
 */
int prom_collector_registry_validate_metric_name(prom_collector_registry_t *self, const char *metric_name);

#endif  // PROM_H
