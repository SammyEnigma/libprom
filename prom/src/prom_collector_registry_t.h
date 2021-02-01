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

#ifndef PROM_REGISTRY_T_H
#define PROM_REGISTRY_T_H

#include <pthread.h>
#include <stdbool.h>

// Public
#include "prom_collector_registry.h"

// Private
#include "prom_map_t.h"
#include "prom_metric_formatter_t.h"
#include "prom_string_builder_t.h"

struct prom_collector_registry {
	const char *name;				/**< name of the registry. Do not modify! */
	const char *mprefix;			/**< prefix each metric name with this */
	PROM_INIT_FLAGS features;		/**< enabled registry features */
	prom_metric_t *scrape_duration;	/**< scrape duration metric to use */
	prom_map_t *collectors;			/**< Map of collectors keyed by name */
	prom_string_builder_t *string_builder;		/**< string building */
	prom_metric_formatter_t *metric_formatter;	/**< export metric(s) */
	pthread_rwlock_t *lock;	/**< mutex to guard concurrent modfications */
};

#endif  // PROM_REGISTRY_T_H
