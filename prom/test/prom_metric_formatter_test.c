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

#include "prom_test_helpers.h"
#include "prom_collector_registry_i.h"

void
test_pmf_load_l_value(void) {
	pmf_t *mf = pmf_new();
	const char *keys[] = {"foo", "bar", "bing"};
	const char *values[] = {"one", "two", "three"};
	pmf_load_l_value(mf, "test", NULL, 3, keys, values);
	char *actual = pmf_dump(mf);
	char *expected = "test{foo=\"one\",bar=\"two\",bing=\"three\"}";
	TEST_ASSERT_NOT_NULL(strstr(actual, expected));

	free(actual);
	actual = NULL;
	pmf_destroy(mf);
	mf = NULL;
}

void
test_pmf_load_sample(void) {
	pmf_t *mf = pmf_new();
	const char *l_value = "test{foo=\"one\",bar=\"two\",bing=\"three\"}";
	pms_t *sample = pms_new(PROM_COUNTER, l_value, 22.2);
	pmf_load_sample(mf, sample, NULL);
	char *actual = pmf_dump(mf);
	char *expected = "test{foo=\"one\",bar=\"two\",bing=\"three\"}";
	TEST_ASSERT_NOT_NULL(strstr(actual, expected));

	free(actual);
	actual = NULL;
	pms_destroy(sample);
	pmf_destroy(mf);
	mf = NULL;
}

void
test_pmf_load_metric(void) {
	pmf_t *mf = pmf_new();
	const char *counter_keys[] = {"foo", "bar"};
	const char *sample_a[] = {"f", "b"};
	const char *sample_b[] = {"o", "r"};
	prom_metric_t *m = prom_metric_new(PROM_COUNTER, "test_counter",
		"counter under test", 2, counter_keys);
	pms_t *s_a = pms_from_labels(m, sample_a);
	pms_add(s_a, 2.3);
	pms_t *s_b = pms_from_labels(m, sample_b);
	pms_add(s_b, 4.6);
	pmf_load_metric(mf, m, "", false);
	const char *result = pmf_dump(mf);

	char *substr = "# HELP test_counter counter under test\n"
		"# TYPE test_counter counter\ntest_counter{foo=\"f\",bar=\"b\"}";

	TEST_ASSERT_NOT_NULL(strstr(result, substr));

	substr = "\ntest_counter{foo=\"o\",bar=\"r\"}";
	TEST_ASSERT_NOT_NULL(strstr(result, substr));

	free((char *)result);
	result = NULL;
	prom_metric_destroy(m);
	m = NULL;
	pmf_destroy(mf);
	mf = NULL;
}

void
test_pmf_load_metrics(void) {
	pmf_t *mf = pmf_new();
	const char *counter_keys[] = {};
	pcr_init(PROM_NONE, "");
	pcr_enable_custom_process_metrics(PROM_COLLECTOR_REGISTRY,
#ifdef __sun
		"../test/fixtures/limits", "../test/fixtures/status"
#else	// assume Linux
		"../test/fixtures/limits", "../test/fixtures/stat"
#endif
	);
	prom_metric_t *m_a = prom_metric_new(PROM_COUNTER, "test_counter_a",
		"counter under test", 0, counter_keys);
	prom_metric_t *m_b = prom_metric_new(PROM_COUNTER, "test_counter_b",
		"counter under test", 0, counter_keys);
	pms_t *s_a = pms_from_labels(m_a, counter_keys);
	pms_add(s_a, 2.3);
	pms_t *s_b = pms_from_labels(m_b, counter_keys);
	pms_add(s_b, 4.6);
	pcr_register_metric(m_a);
	pcr_register_metric(m_b);
	pmf_load_metrics(mf, PROM_COLLECTOR_REGISTRY->collectors,
		NULL, "", false);

	const char *result = pmf_dump(mf);
	const char *expected[] = {
		// from "default" collector
		"# HELP test_counter_a counter under test",
		"# TYPE test_counter_a counter",
		"test_counter_a",
		"# HELP test_counter_b counter under test",
		"# TYPE test_counter_b counter",
		"test_counter_b",
		// from "process" collector
		"# HELP process_max_fds Max. number of open file descriptors "
			"(soft limit)",
		"# TYPE process_max_fds gauge",
		"process_max_fds 1048576",
	};

	for (int i = 0; i < 9; i++) {
		TEST_ASSERT_NOT_NULL_MESSAGE(strstr(result, expected[i]), expected[i]);
	}

	free((char *) result);
	result = NULL;

	if (pmf_destroy(mf))
	  TEST_FAIL_MESSAGE("Failed to destroy metric formatter");
	mf = NULL;
	if (pcr_destroy(PROM_COLLECTOR_REGISTRY))
		TEST_FAIL_MESSAGE("Failed to destroy default collector registry");
	PROM_COLLECTOR_REGISTRY = NULL;
}

int
main(int argc, const char **argv) {
	UNITY_BEGIN();
	RUN_TEST(test_pmf_load_l_value);
	RUN_TEST(test_pmf_load_sample);
	RUN_TEST(test_pmf_load_metric);
	RUN_TEST(test_pmf_load_metrics);
	return UNITY_END();
}
