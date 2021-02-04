/**
 * Copyright 2019 DigitalOcean Inc.
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
#include "prom.h"

prom_counter_t *foo_counter;
prom_gauge_t *foo_gauge;

int
foo(int v, const char *label) {
	if (prom_counter_inc(foo_counter, NULL))
		return 1;
	return prom_gauge_add(foo_gauge, v, (const char *[]) { label });
}

void
foo_init(void) {
	foo_counter = pcr_must_register_metric(prom_counter_new("foo_counter",
		"counter for foo", 0, NULL));
	foo_gauge = pcr_must_register_metric(prom_gauge_new("foo_gauge",
		"gauge for foo", 1, (const char *[]) { "label" }));
}
