/**
 * Copyright 2019-2020 DigitalOcean Inc.
 * Copyright 2021 Jens Elkner <jel+libprom@cs.uni-magdeburg.de>.
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

#include <pthread.h>
#include <stdio.h>

// Public
#include "prom_alloc.h"
#include "prom_histogram.h"

// Private
#include "prom_assert.h"
#include "prom_errors.h"
#include "prom_linked_list_i.h"
#include "prom_log.h"
#include "prom_map_i.h"
#include "prom_metric_formatter_i.h"
#include "prom_metric_sample_histogram_i.h"
#include "prom_metric_sample_i.h"

//////////////////////////////////////////////////////////////////////////////
// Static Declarations
//////////////////////////////////////////////////////////////////////////////

static const char *l_value_for_bucket(pms_histogram_t *self, const char *name, size_t label_count, const char **label_keys, const char **label_values, const char *bucket_key);

static const char *l_value_for_inf(pms_histogram_t *self, const char *name, size_t label_count, const char **label_keys, const char **label_values);

static void free_str_generic(void *gen);

static int init_bucket_samples(pms_histogram_t *self, const char *name, size_t label_count, const char **label_keys, const char **label_values);

static int init_inf(pms_histogram_t *self, const char *name, size_t label_count, const char **label_keys, const char **label_values);

static int init_count(pms_histogram_t *self, const char *name, size_t label_count, const char **label_keys, const char **label_values);

static int init_summary(pms_histogram_t *self, const char *name, size_t label_count, const char **label_keys, const char **label_values);

//////////////////////////////////////////////////////////////////////////////
// End static declarations
//////////////////////////////////////////////////////////////////////////////

pms_histogram_t *
pms_histogram_new(const char *name, phb_t *buckets, size_t label_count,
	const char **label_keys, const char **label_values)
{
	// Allocate and set self
	pms_histogram_t *self = (pms_histogram_t *)
		prom_malloc(sizeof(pms_histogram_t));
	if (self == NULL)
		return NULL;
	memset(self, 0, sizeof(pms_histogram_t));

	// Allocate and set the l_value_list
	if ((self->l_value_list = pll_new()) == NULL)
		goto fail;
	// Allocate and set the metric formatter
	if ((self->metric_formatter = pmf_new()) == NULL)
		goto fail;
	// Store map of l_value/pms_t
	if ((self->samples = prom_map_new()) == NULL)
		goto fail;
	// Set the free value function on the samples map
	if (prom_map_set_free_value_fn(self->samples, &pms_free_generic))
		goto fail;
	// Store map of bucket/l_value
	if ((self->l_values = prom_map_new()) == NULL)
		goto fail;
	// Set the free value function for thhe l_values map
	if (prom_map_set_free_value_fn(self->l_values, free_str_generic))
		goto fail;
	self->buckets = buckets;
	// Allocate and initialize the lock
	self->rwlock = (pthread_rwlock_t *) prom_malloc(sizeof(pthread_rwlock_t));
	if (self->rwlock == NULL || pthread_rwlock_init(self->rwlock, NULL))
		goto fail;
	// Allocate and initialize bucket metric samples
	if (init_bucket_samples(self, name, label_count, label_keys, label_values))
		goto fail;
	// Allocate and initialize the +Inf metric sample
	if (init_inf(self, name, label_count, label_keys, label_values))
		goto fail;
	// Allocate and initialize the count metric sample
	if (init_count(self, name, label_count, label_keys, label_values))
		goto fail;
	// Add summary sample
	if (init_summary(self, name, label_count, label_keys, label_values))
		goto fail;
	// The value of nodes in this map will be simple pms pointers
	if (prom_map_set_free_value_fn(self->samples, &pms_free_generic))
		goto fail;

	return self;

fail:
	pms_histogram_destroy(self);
	return NULL;
}

static int
init_bucket_samples(pms_histogram_t *self, const char *name, size_t label_count,
	const char **label_keys, const char **label_values)
{
	PROM_ASSERT(self);
	int bucket_count = phb_count(self->buckets);

	// For each bucket, create an pms_t with an appropriate
	// l_value and default value of 0.0. The l_value will contain the metric
	// name, user labels, and finally, the le label and bucket value.
	for (int i = 0; i < bucket_count; i++) {
		const char *bucket_key = self->buckets->key[i];
		if (bucket_key == NULL)
			return 3;
		const char *l_value = l_value_for_bucket(self, name, label_count,
			label_keys, label_values, bucket_key);
		if (l_value == NULL)
			return 1;
		if (pll_append(self->l_value_list, prom_strdup(l_value)))
			return 2;
		if (prom_map_set(self->l_values, bucket_key, (char *) l_value))
			return 4;
		pms_t *sample = pms_new(PROM_HISTOGRAM, l_value, 0.0);
		if (sample == NULL)
			return 5;
		if (prom_map_set(self->samples, l_value, sample))
			return 6;
	}
	return 0;
}

static int
init_inf(pms_histogram_t *self, const char *name, size_t label_count,
	const char **label_keys, const char **label_values)
{
	PROM_ASSERT(self != NULL);
	const char *inf_l_value =
		l_value_for_inf(self, name, label_count, label_keys, label_values);
	if (inf_l_value == NULL)
		return 1;

	if (pll_append(self->l_value_list, prom_strdup(inf_l_value)))
		return 2;
	if (prom_map_set(self->l_values, "+Inf", (char *) inf_l_value))
		return 3;
	pms_t *inf_sample = pms_new(PROM_HISTOGRAM, (char *) inf_l_value, 0.0);
	if (inf_sample == NULL)
		return 4;
	return prom_map_set(self->samples, inf_l_value, inf_sample);
}

static int
init_count(pms_histogram_t *self, const char *name,
	size_t label_count, const char **label_keys, const char **label_values)
{
	PROM_ASSERT(self != NULL);
	if (pmf_load_l_value(self->metric_formatter, name, "count", label_count,
			label_keys, label_values))
	{
		return 1;
	}

	const char *count_l_val = pmf_dump(self->metric_formatter);
	if (count_l_val == NULL)
		return 1;
	if (pll_append(self->l_value_list, prom_strdup(count_l_val)))
		return 2;
	if (prom_map_set(self->l_values, "count", (char *) count_l_val))
		return 3;
	pms_t *count_sample = pms_new(PROM_HISTOGRAM, count_l_val, 0.0);
	if (count_sample == NULL)
		return 1;
	return prom_map_set(self->samples, count_l_val, count_sample);
}

static int
init_summary(pms_histogram_t *self, const char *name, size_t label_count,
	const char **label_keys, const char **label_values)
{
	PROM_ASSERT(self != NULL);
	if (pmf_load_l_value(self->metric_formatter, name, "sum", label_count,
		label_keys, label_values))
	{
		return 1;
	}
	const char *sum_l_val = pmf_dump(self->metric_formatter);
	if (sum_l_val == NULL)
		return 2;

	if (pll_append(self->l_value_list, prom_strdup(sum_l_val)))
		return 3;
	if (prom_map_set(self->l_values, "sum", (char *) sum_l_val))
		return 4;

	pms_t *sum_sample = pms_new(PROM_HISTOGRAM, sum_l_val, 0.0);
	if (sum_sample == NULL)
		return 1;

	return prom_map_set(self->samples, sum_l_val, sum_sample);
}

int
pms_histogram_destroy(pms_histogram_t *self) {
	if (self == NULL)
		return 0;

	pll_destroy(self->l_value_list);
	self->l_value_list = NULL;

	prom_map_destroy(self->samples);
	self->samples = NULL;

	prom_map_destroy(self->l_values);
	self->l_values = NULL;

	pmf_destroy(self->metric_formatter);
	self->metric_formatter = NULL;

	pthread_rwlock_destroy(self->rwlock);
	prom_free(self->rwlock);
	self->rwlock = NULL;

	prom_free(self);
	return 0;
}

int
pms_histogram_destroy_generic(void *g) {
	return pms_histogram_destroy((pms_histogram_t *) g);
}

void
pms_histogram_free_generic(void *gen) {
	pms_histogram_destroy((pms_histogram_t *) gen);
}

int
pms_histogram_observe(pms_histogram_t *self, double value) {
	int err = 2;
	if (pthread_rwlock_wrlock(self->rwlock)) {
		PROM_WARN(PROM_PTHREAD_RWLOCK_LOCK_ERROR, NULL);
		return 1;
	}

	// Update the counter for the proper bucket if found
	int bucket_count = phb_count(self->buckets);
	for (int i = (bucket_count - 1); i >= 0; i--) {
		if (value > self->buckets->upper_bound[i])
			break;
		const char *bucket_key = self->buckets->key[i];
		if (bucket_key == NULL)
			goto end;
		const char *l_value = prom_map_get(self->l_values, bucket_key);
		if (l_value == NULL)
			goto end;

		pms_t *sample = prom_map_get(self->samples, l_value);
		if (sample == NULL)
			goto end;

		if (pms_add(sample, 1.0))
			goto end;
	}

	// Update the +Inf and count samples
	const char *inf_l_value = prom_map_get(self->l_values, "+Inf");
	if (inf_l_value == NULL)
		goto end;
	pms_t *inf_sample = prom_map_get(self->samples, inf_l_value);
	if (inf_sample == NULL)
		goto end;
	if (pms_add(inf_sample, 1.0))
		goto end;
	const char *count_l_value = prom_map_get(self->l_values, "count");
	if (count_l_value == NULL)
		goto end;
	pms_t *count_sample = prom_map_get(self->samples, count_l_value);
	if (count_sample == NULL)
		goto end;
	if (pms_add(count_sample, 1.0))
		goto end;

	// Update the sum sample
	const char *sum_l_value = prom_map_get(self->l_values, "sum");
	if (sum_l_value == NULL)
		goto end;
	pms_t *sum_sample = prom_map_get(self->samples, sum_l_value);
	if (sum_sample == NULL)
		goto end;
	if (pms_add(sum_sample, value))
		goto end;

	err = 0;

end:
	if (pthread_rwlock_unlock(self->rwlock)) {
		PROM_WARN(PROM_PTHREAD_RWLOCK_UNLOCK_ERROR, NULL);
		err = 3;
	}
	return err;
}

static const char *
l_value_for_bucket(pms_histogram_t *self, const char *name, size_t label_count,
	const char **label_keys, const char **label_values, const char *bucket_key)
{
	PROM_ASSERT(self != NULL);

	// Make new array to hold label_keys with le label key
	const char **new_keys = (const char **)
		prom_malloc((label_count + 1) * sizeof(char *));
	if (new_keys == NULL)
		return NULL;
	// Make new array to hold label_values with le label value
	const char **new_values = (const char **)
		prom_malloc((label_count + 1) * sizeof(char *));
	if (new_keys == NULL) {
		prom_free(new_keys);
		return NULL;
	}
	for (size_t i = 0; i < label_count; i++) {
		new_keys[i] = prom_strdup(label_keys[i]);
		new_values[i] = prom_strdup(label_values[i]);
	}
	new_keys[label_count] = prom_strdup("le");
	new_values[label_count] = prom_strdup(bucket_key);

	const char *ret = pmf_load_l_value(self->metric_formatter, name, "bucket",
		label_count + 1, new_keys, new_values)
		? NULL
		: (const char *) pmf_dump(self->metric_formatter);

	for (size_t i = 0; i < label_count + 1; i++) {
		prom_free((char *) new_keys[i]);
		prom_free((char *) new_values[i]);
	}
	prom_free(new_keys);
	prom_free(new_values);
	return ret;
}

static const char *
l_value_for_inf(pms_histogram_t *self, const char *name, size_t label_count,
	const char **label_keys, const char **label_values)
{
	PROM_ASSERT(self != NULL);

	// Make new array to hold label_keys with le label key
	const char **new_keys = (const char **)
		prom_malloc((label_count + 1) * sizeof(char *));
	if (new_keys == NULL)
		return NULL;
	// Make new array to hold label_values with le label value
	const char **new_values = (const char **)
		prom_malloc((label_count + 1) * sizeof(char *));
	if (new_keys == NULL) {
		prom_free(new_keys);
		return NULL;
	}

	for (size_t i = 0; i < label_count; i++) {
		new_keys[i] = prom_strdup(label_keys[i]);
		new_values[i] = prom_strdup(label_values[i]);
	}
	new_keys[label_count] = prom_strdup("le");
	new_values[label_count] = prom_strdup("+Inf");

	const char *ret = pmf_load_l_value(self->metric_formatter, name, "bucket",
		label_count + 1, new_keys, new_values)
		? NULL
		: (const char *) pmf_dump(self->metric_formatter);

	for (size_t i = 0; i < label_count + 1; i++) {
		prom_free((char *) new_keys[i]);
		prom_free((char *) new_values[i]);
	}
	prom_free(new_keys);
	prom_free(new_values);
	return ret;
}

static void
free_str_generic(void *gen) {
	prom_free((void *) gen);
}
