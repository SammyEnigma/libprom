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

// Public
#include "prom_alloc.h"
#include "prom_gauge.h"

// Private
#include "prom_assert.h"
#include "prom_collector_t.h"
#include "prom_linked_list_t.h"
#include "prom_log.h"
#include "prom_map_i.h"
#include "prom_metric_formatter_i.h"
#include "prom_metric_sample_histogram_t.h"
#include "prom_metric_sample_t.h"
#include "prom_metric_t.h"
#include "prom_string_builder_i.h"

prom_metric_formatter_t *prom_metric_formatter_new() {
  prom_metric_formatter_t *self = (prom_metric_formatter_t *)prom_malloc(sizeof(prom_metric_formatter_t));
  self->string_builder = prom_string_builder_new();
  if (self->string_builder == NULL) {
    prom_metric_formatter_destroy(self);
    return NULL;
  }
  self->err_builder = prom_string_builder_new();
  if (self->err_builder == NULL) {
    prom_metric_formatter_destroy(self);
    return NULL;
  }
  return self;
}

int prom_metric_formatter_destroy(prom_metric_formatter_t *self) {
  PROM_ASSERT(self != NULL);
  if (self == NULL) return 0;

  int r = 0;
  int ret = 0;

  r = prom_string_builder_destroy(self->string_builder);
  self->string_builder = NULL;
  if (r) ret = r;

  r = prom_string_builder_destroy(self->err_builder);
  self->err_builder = NULL;
  if (r) ret = r;

  prom_free(self);
  self = NULL;
  return ret;
}

int
prom_metric_formatter_load_help(prom_metric_formatter_t *self,
	const char *prefix, const char *name, const char *help)
{
	if (self == NULL)
		return 1;
	if (help == NULL)
		return 0;

	if (prom_string_builder_add_str(self->string_builder, "# HELP ") != 0)
		return 2;
	if (prefix != NULL
		&& prom_string_builder_add_str(self->string_builder, prefix) != 0)
	{
		return 3;
	}
	if (prom_string_builder_add_str(self->string_builder, name) != 0)
		return 4;
	if (prom_string_builder_add_char(self->string_builder, ' ') != 0)
		return 5;
	if (prom_string_builder_add_str(self->string_builder, help) != 0)
		return 6;
	return prom_string_builder_add_char(self->string_builder, '\n') ? 7 : 0;
}

int
prom_metric_formatter_load_type(prom_metric_formatter_t *self,
	const char *prefix, const char *name, prom_metric_type_t metric_type)
{
	if (self == NULL)
		return 1;

	if (prom_string_builder_add_str(self->string_builder, "# TYPE ") != 0)
		return 2;
	if (prefix != NULL
		&& prom_string_builder_add_str(self->string_builder, prefix) != 0)
	{
		return 3;
	}
	if (prom_string_builder_add_str(self->string_builder, name) != 0)
		return 4;
	if (prom_string_builder_add_char(self->string_builder, ' ') != 0)
		return 5;
	if (prom_string_builder_add_str(self->string_builder,
		prom_metric_type_map[metric_type]) != 0)
	{
		return 6;
	}

	return prom_string_builder_add_char(self->string_builder, '\n') ? 7 : 0;
}

int
prom_metric_formatter_load_l_value(prom_metric_formatter_t *self,
	const char *name, const char *suffix,
	size_t label_count, const char **label_keys, const char **label_values)
{
	if (self == NULL || name == NULL)
		return 1;

	if (prom_string_builder_add_str(self->string_builder, name) != 0)
		return 3;
	if (suffix != NULL) {
		if (prom_string_builder_add_char(self->string_builder, '_') != 0)
			return 4;
		if (prom_string_builder_add_str(self->string_builder, suffix) != 0)
			return 5;
	}
	if (label_count == 0)
		return 0;

	if (prom_string_builder_add_char(self->string_builder, '{') != 0)
		return 6;
	for (int i = 0; i < label_count; i++) {
		if (prom_string_builder_add_str(self->string_builder,
			(const char *) label_keys[i]) != 0)
		{
			return 7;
		}
		if (prom_string_builder_add_str(self->string_builder, "=\"") != 0)
			return 9;
		if (prom_string_builder_add_str(self->string_builder,
			(const char *) label_values[i]) != 0)
		{
			return 10;
		}
		if (prom_string_builder_add_str(self->string_builder, "\",") != 0)
			return 11;
	}
	prom_string_builder_truncate(self->string_builder,
		prom_string_builder_len(self->string_builder) - 1);
	if (prom_string_builder_add_char(self->string_builder, '}') != 0)
		return 20;

	return 0;
}

int
prom_metric_formatter_load_sample(prom_metric_formatter_t *self,
	prom_metric_sample_t *sample, const char *prefix)
{
	if (self == NULL)
		return 1;
	if (prefix != NULL)
		prom_string_builder_add_str(self->string_builder, prefix);
	if (prom_string_builder_add_str(self->string_builder, sample->l_value) != 0)
		return 2;
	char buffer[64];
	sprintf(buffer, " %.17g", sample->r_value);
	if (prom_string_builder_add_str(self->string_builder, buffer) != 0)
		return 3;
	return prom_string_builder_add_char(self->string_builder, '\n') ? 4 : 0;
}

int
prom_metric_formatter_clear(prom_metric_formatter_t *self) {
	PROM_ASSERT(self != NULL);
	return prom_string_builder_clear(self->string_builder);
}

char *
prom_metric_formatter_dump(prom_metric_formatter_t *self) {
	if (self == NULL)
		return NULL;
	char *data = prom_string_builder_dump(self->string_builder);
	prom_string_builder_clear(self->string_builder);
	return data;
}

int
prom_metric_formatter_load_metric(prom_metric_formatter_t *self,
	prom_metric_t *metric, const char *prefix, bool compact)
{
	if (self == NULL)
		return 1;
	const char *p = (prefix != NULL && strlen(prefix) == 0) ? NULL : prefix;

	if (!compact) {
		if (prom_metric_formatter_load_help(self,p,metric->name,metric->help))
			return 2;
		if (prom_metric_formatter_load_type(self,p,metric->name,metric->type))
			return 3;
	}
	for (prom_linked_list_node_t *current_node = metric->samples->keys->head;
		current_node != NULL; current_node = current_node->next)
	{
		const char *key = (const char *) current_node->item;
		if (metric->type == PROM_HISTOGRAM) {
			prom_metric_sample_histogram_t *hist_sample =
				(prom_metric_sample_histogram_t *)
				prom_map_get(metric->samples, key);
			if (hist_sample == NULL)
				return 4;
			for (prom_linked_list_node_t *current_hist_node =
				hist_sample->l_value_list->head; current_hist_node != NULL;
				current_hist_node = current_hist_node->next)
			{
				const char *hist_key = (const char *) current_hist_node->item;
				prom_metric_sample_t *sample = (prom_metric_sample_t *)
					prom_map_get(hist_sample->samples, hist_key);
				if (sample == NULL)
					return 5;
				if (prom_metric_formatter_load_sample(self, sample, p))
					return 6;
			}
		} else {
			prom_metric_sample_t *sample = (prom_metric_sample_t *)
				prom_map_get(metric->samples, key);
			if (sample == NULL)
				return 7;
			if (prom_metric_formatter_load_sample(self, sample, p))
				return 8;
		}
	}
	return prom_string_builder_add_char(self->string_builder, '\n') ? 9 : 0;
}

int
prom_metric_formatter_load_metrics(prom_metric_formatter_t *self,
	prom_map_t *collectors, prom_metric_t *scrape_metric, const char *mprefix,
	bool compact)
{
	PROM_ASSERT(self != NULL);
	int r = 0;
	struct timespec start, end;
	static const char *labels[] = { "" };

	for (prom_linked_list_node_t *current_node = collectors->keys->head;
		current_node != NULL; current_node = current_node->next)
	{
		if (scrape_metric != NULL)
			clock_gettime(CLOCK_MONOTONIC, &start);

		const char *cname = (const char *) current_node->item;
		prom_collector_t *c = (prom_collector_t *)
			prom_map_get(collectors, cname);
		if (c == NULL) {
			PROM_WARN("Collector '%s' not found.", cname);
			r++;
			continue;
		}

		prom_map_t *metrics = c->collect_fn(c);
		if (metrics == NULL)
			continue;

		for (prom_linked_list_node_t *current_node = metrics->keys->head;
			current_node != NULL; current_node = current_node->next)
		{
			const char *mname = (const char *) current_node->item;
			prom_metric_t *metric = (prom_metric_t *)
				prom_map_get(metrics, mname);
			if (metric == NULL) {
				PROM_WARN("Collector '%s' has no metric named '%s'.", cname,
					mname);
				r++;
				continue;
			}
			r += prom_metric_formatter_load_metric(self,metric,mprefix,compact);
		}
		if (scrape_metric != NULL) {
			int r = clock_gettime(CLOCK_MONOTONIC, &end);
			time_t s = (r == 0) ? end.tv_sec - start.tv_sec : 0;
			long ns = (r == 0) ? end.tv_nsec - start.tv_nsec : 0;
			double duration = s + ns*1e-9;
			labels[0] = cname;
			prom_gauge_set(scrape_metric, duration, labels);
		}
	}
	return r;
}
