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
 * @file prom_histogram_buckets.h
 * @brief https://prometheus.io/docs/concepts/metric_types/#histogram
 * @note To get rid off braindamaged long names, you may replace
 * \c prom_histogram_bucket with \c PHB .
 */

#include "stdlib.h"

#ifndef PROM_HISTOGRAM_BUCKETS_H
#define PROM_HISTOGRAM_BUCKETS_H

typedef struct phb {
	int count;						/**< Number of buckets */
	const double *upper_bound;		/**< count ascending upper limits */
	const char **key;				/**< count keys used to lookup samples. */
} phb_t;

/**
 * @brief Construct a new histogram bucket with the given values.
 * @param count	Number of buckets.
 * @param bucket	\c count values to add to the bucket.
 * @return The new histogram bucket, or \c NULL if an error occured.
 */
phb_t *phb_new(size_t count, double bucket, ...);

/**
 * @brief default histogram buckets: .005, .01, .025, .05, .1, .25, .5, 1, 2.5, 5, 10
 */
extern phb_t *prom_histogram_default_buckets;

/**
 * @brief Construct a linearly sized prom histogram bucket.
 * @param start The first inclusive upper bound.
 * @param width The distance between each upper bound.
 * @param count The total number of buckets. The final +Inf bucket is not
 *	counted and not included.
 * @return The constructed histogram bucket or \c NULL if an error occured.
 */
phb_t *phb_linear(double start, double width, size_t count);

/**
 * @brief Construct an exponentially sized prom histogram bucket.
 * @param start		The first inclusive upper bound. The value MUST be > 0.
 * @param factor	The factor to apply to the previous upper bound to produce
 *	the next upper bound. The value MUST be greater than 1.
 * @param count		The total number of buckets. The final +Inf bucket is not
 *	counted and not included. The value MUST be greater than or equal to 1.
 * @return The new histogram bucket, or \c NULL if an error occured.
 */
phb_t *phb_exponential(double start, double factor, size_t count);

/**
 * @brief Destroy the given prom histogram bucket.
 * @param self The bucket to destroy.
 * @return Non-zero integer value upon failure, otherwise \c 0 .
 * @note No matter what gets returned, you should never use any bucket
 *	passed to this function but set it to \c NULL .
 */
int phb_destroy(phb_t *self);

/**
 * @brief Get the number of valus in the given bucket.
 * @param self The bucket to query.
 * @return The number of values in the bucket.
 */
size_t phb_count(phb_t *self);

#endif  // PROM_HISTOGRAM_BUCKETS_H
