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

#include <stdarg.h>
#include <stdlib.h>

// Public
#include "prom_alloc.h"
#include "prom_histogram_buckets.h"

// Private
#include "prom_assert.h"
#include "prom_log.h"

phb_t *prom_histogram_default_buckets = NULL;

phb_t *
phb_new(size_t count, double bucket, ...) {
	phb_t *self = (phb_t *) prom_malloc(sizeof(phb_t));
	if (self == NULL)
		return NULL;

	self->count = count;
	self->upper_bounds = NULL;
	double *upper_bounds = (double *) prom_malloc(sizeof(double) * count);
	if (upper_bounds == NULL) {
		phb_destroy(self);
		return NULL;
	}
	upper_bounds[0] = bucket;
	if (count == 1) {
		self->upper_bounds = upper_bounds;
		return self;
	}
	va_list arg_list;
	va_start(arg_list, bucket);
	for (int i = 1; i < count; i++) {
		upper_bounds[i] = va_arg(arg_list, double);
	}
	va_end(arg_list);
	self->upper_bounds = upper_bounds;
	return self;
}

phb_t *
phb_linear(double start, double width, size_t count) {
	if (count <= 1)
		return NULL;

	phb_t *self = (phb_t *) prom_malloc(sizeof(phb_t));
	if (self == NULL)
		return NULL;
	self->upper_bounds = NULL;
	double *upper_bounds = (double *) prom_malloc(sizeof(double) * count);
	if (upper_bounds == NULL) {
		phb_destroy(self);
		return NULL;
	}
	upper_bounds[0] = start;
	for (size_t i = 1; i < count; i++) {
		upper_bounds[i] = upper_bounds[i - 1] + width;
	}
	self->upper_bounds = upper_bounds;
	self->count = count;
	return self;
}

phb_t *
phb_exponential(double start, double factor, size_t count) {
	if (count < 1) {
		PROM_WARN("count must be less than %d", 1);
		return NULL;
	}
	if (start <= 0) {
		PROM_WARN("start must be less than or equal to %d", 0);
		return NULL;
	}
	if (factor <= 1) {
		PROM_WARN("factor must be less than or equal to %d", 1);
		return NULL;
	}

	phb_t *self = (phb_t *) prom_malloc(sizeof(phb_t));
	if (self == NULL)
		return NULL;
	self->upper_bounds = NULL;
	double *upper_bounds = (double *) prom_malloc(sizeof(double) * count);
	if (upper_bounds == NULL) {
		phb_destroy(self);
		return NULL;
	}
	upper_bounds[0] = start;
	for (size_t i = 1; i < count; i++) {
		upper_bounds[i] = upper_bounds[i - 1] * factor;
	}
	self->upper_bounds = upper_bounds;
	self->count = count;
	return self;
}

int
phb_destroy(phb_t *self) {
	if (self == NULL)
		return 0;
	prom_free((double *) self->upper_bounds);
	self->upper_bounds = NULL;
	prom_free(self);
	return 0;
}

size_t
phb_count(phb_t *self) {
	PROM_ASSERT(self != NULL);
	return self->count;
}
