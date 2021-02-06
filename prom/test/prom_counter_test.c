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

#include <assert.h>

#include "prom_test_helpers.h"

const char *sample_labels_a[] = {"f", "b"};
const char *sample_labels_b[] = {"o", "r"};

void
test_counter_inc(void) {
	prom_counter_t *c = prom_counter_new("test_counter", "counter under test",
		2, (const char *[]) {"foo", "bar"});
	TEST_ASSERT(c);

	prom_counter_inc(c, sample_labels_a);
	pms_t *sample = pms_from_labels(c, sample_labels_a);
	TEST_ASSERT_EQUAL_DOUBLE(1.0, sample->r_value);

	sample = pms_from_labels(c, sample_labels_b);
	TEST_ASSERT_EQUAL_DOUBLE(0.0, sample->r_value);

	prom_counter_destroy(c);
}

void
test_counter_add(void) {
	prom_counter_t *c = prom_counter_new("test_counter", "counter under test",
		2, (const char *[]){"foo", "bar"});
	TEST_ASSERT(c);

	prom_counter_add(c, 100000000.1, sample_labels_a);
	pms_t *sample = pms_from_labels(c, sample_labels_a);
	TEST_ASSERT_EQUAL_DOUBLE(100000000.1, sample->r_value);

	sample = pms_from_labels(c, sample_labels_b);
	TEST_ASSERT_EQUAL_DOUBLE(0.0, sample->r_value);

	prom_counter_destroy(c);
}

void
test_counter_reset(void) {
	prom_counter_t *g = prom_counter_new("test_counter", "counter under test",
		2, (const char *[]) {"foo", "bar"});
	TEST_ASSERT(g);

	prom_counter_reset(g, 100000000.1, sample_labels_a);
	pms_t *sample = pms_from_labels(g, sample_labels_a);
	TEST_ASSERT_EQUAL_DOUBLE(100000000.1, sample->r_value);

	sample = pms_from_labels(g, sample_labels_b);
	TEST_ASSERT_EQUAL_DOUBLE(0.0, sample->r_value);

	prom_counter_reset(g, 1, sample_labels_a);
	sample = pms_from_labels(g, sample_labels_a);
	TEST_ASSERT_EQUAL_DOUBLE(1, sample->r_value);

	TEST_ASSERT_TRUE_MESSAGE(prom_counter_reset(g, -1, sample_labels_a) > 0,
		"prom_counter_reset() should not allow negative values");

	prom_counter_destroy(g);
}

int
main(int argc, const char **argv) {
	UNITY_BEGIN();
	RUN_TEST(test_counter_inc);
	RUN_TEST(test_counter_add);
	RUN_TEST(test_counter_reset);
	return UNITY_END();
}
