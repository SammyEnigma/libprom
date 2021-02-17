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

#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>

// Public
#include "prom_alloc.h"
#include "prom_collector.h"

// Private
#include "prom_collector_t.h"
#include "prom_log.h"
#include "prom_metric_i.h"
#include "prom_string_builder.h"

prom_map_t *
prom_collector_metrics_get(prom_collector_t *self) {
	return self->metrics;
}

prom_collector_t *
prom_collector_new(const char *name) {
	prom_collector_t *self = (prom_collector_t *)
		prom_malloc(sizeof(prom_collector_t));
	if (self == NULL)
		return NULL;

	self->name = prom_strdup(name);
	self->data = NULL;
	self->free_data_fn = NULL;
	self->collect_fn = &prom_collector_metrics_get;
	self->string_builder = NULL;

	self->metrics = prom_map_new();
	if (self->metrics == NULL)
		goto fail;
	if (prom_map_set_free_value_fn(self->metrics, &prom_metric_free_generic))
		goto fail;
	if ((self->string_builder = psb_new()) == NULL)
		goto fail;
	return self;

fail:
    prom_collector_destroy(self);
    return NULL;
}

int
prom_collector_destroy(prom_collector_t *self) {
	if (self == NULL)
		return 0;

	int r = 0;

	self->collect_fn = NULL;
	if (self->free_data_fn != NULL) {
		self->free_data_fn(self);
		self->free_data_fn = NULL;
	}
	self->data = NULL;
	r += prom_map_destroy(self->metrics);
	self->metrics = NULL;
	r += psb_destroy(self->string_builder);
	self->string_builder = NULL;
	prom_free((char *)self->name);
	self->name = NULL;
	prom_free(self);
	return r;
}

int
prom_collector_destroy_generic(void *gen) {
	if (gen == NULL)
		return 0;
	prom_collector_t *self = (prom_collector_t *)gen;
	return prom_collector_destroy(self);
}

void
prom_collector_free_generic(void *gen) {
	if (gen == NULL)
		return;
	prom_collector_t *self = (prom_collector_t *)gen;
	prom_collector_destroy(self);
}

int
prom_collector_set_collect_fn(prom_collector_t *self, prom_collect_fn *fn) {
	if (self == NULL)
		return 1;
	self->collect_fn = fn;
	return 0;
}

int
prom_collector_add_metric(prom_collector_t *self, prom_metric_t *metric) {
	if (self == NULL)
		return 1;
	if (prom_map_get(self->metrics, metric->name) != NULL) {
		PROM_LOG("metric already found in collector");
		return 1;
	}
	return prom_map_set(self->metrics, metric->name, metric);
}

void *
prom_collector_data_set(prom_collector_t *self, void *data,
	prom_collector_free_data_fn *fn)
{
	if (self == NULL)
		return NULL;
	void *old = self->data;
	self->data = data;
	self->free_data_fn = fn;
	return old;
}

void *
prom_collector_data_get(prom_collector_t *self) {
	if (self == NULL)
		return NULL;
	return self->data;
}
