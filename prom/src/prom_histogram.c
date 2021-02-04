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

// Public
#include "prom_histogram.h"

#include "prom_alloc.h"
#include "prom_histogram_buckets.h"

// Private
#include "prom_assert.h"
#include "prom_errors.h"
#include "prom_log.h"
#include "prom_map_i.h"
#include "prom_metric_i.h"
#include "prom_metric_sample_histogram_i.h"
#include "prom_metric_sample_histogram_t.h"
#include "prom_metric_t.h"

prom_histogram_t *
prom_histogram_new(const char *name, const char *help, phb_t *buckets,
	size_t label_key_count, const char **label_keys)
{
	prom_histogram_t *self = (prom_histogram_t *)
		prom_metric_new(PROM_HISTOGRAM,name, help, label_key_count, label_keys);
	if (buckets == NULL) {
		if (prom_histogram_default_buckets == NULL) {
			prom_histogram_default_buckets = phb_new(11, 0.005, 0.01, 0.025,
				0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0);
		}
		self->buckets = prom_histogram_default_buckets;
	} else {
		// Ensure the bucket values are increasing
		for (int i = 1; i < buckets->count; i++) {
			if (buckets->upper_bounds[i - 1] > buckets->upper_bounds[i])
				return NULL;
		}
		self->buckets = buckets;
	}
	return self;
}

int
prom_histogram_destroy(prom_histogram_t *self) {
	return (self == NULL) ? 0 : prom_metric_destroy(self);
}

int
prom_histogram_observe(prom_histogram_t *self, double val,
	const char **label_vals)
{
	if (self == NULL)
		return 1;
	if (self->type != PROM_HISTOGRAM) {
		PROM_WARN(PROM_METRIC_INCORRECT_TYPE " (%d)", self->type);
		return 1;
	}
	pms_histogram_t *s = pms_histogram_from_labels(self, label_vals);
	return (s == NULL) ? 1 : pms_histogram_observe(s, val);
}
