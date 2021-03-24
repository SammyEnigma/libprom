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

#ifndef PROM_METRIC_T_H
#define PROM_METRIC_T_H

#include <pthread.h>

// Public
#include "prom_histogram_buckets.h"
#include "prom_metric.h"

// Private
#include "prom_map_i.h"
#include "prom_map_t.h"
#include "prom_metric_formatter_t.h"

/**
 * @brief PRIVATE Contains metric type constants
 */
typedef enum prom_metric_type {
	PROM_COUNTER,
	PROM_GAUGE,
	PROM_HISTOGRAM,
	PROM_SUMMARY,
	PROM_UNTYPED
} prom_metric_type_t;

/**
 * @brief PRIVATE Maps metric type constants to human readable string values
 */
extern const char *prom_metric_type_map[5];

/**
 * @brief PRIVATE An opaque struct to users containing metric metadata; one or
 *	more metric samples; and a metric formatter for locating metric samples
 *	and exporting metric data
 */
struct prom_metric {
	prom_metric_type_t type;	/**< metric type */
	const char *name;			/**< metric name */
	const char *help;			/**< metric help */
	prom_map_t *samples;		/**< collected samples */
	phb_t *buckets;				/**< histogram bucket upper bound values */
	size_t label_key_count;		/**< number of labels */
	pmf_t *formatter;			/**< metric formatter  */
	pthread_rwlock_t *rwlock;	/**< lock support non-atomic ops */
	const char **label_keys;	/**< labels **/
};

#endif  // PROM_METRIC_T_H
