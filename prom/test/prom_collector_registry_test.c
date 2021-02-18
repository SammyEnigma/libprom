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

#include <unistd.h>
#include <pthread.h>

#include "prom_test_helpers.h"
#include "prom_log.h"

static void prom_registry_test_init(void);
static void prom_registry_test_destroy(void);

prom_counter_t *test_counter;
prom_gauge_t *test_gauge;
prom_histogram_t *test_histogram;

pthread_mutex_t fn_mutex = PTHREAD_MUTEX_INITIALIZER;

void
setUp(void) {
	pthread_mutex_lock(&fn_mutex);
}

void
tearDown(void) {
	PROM_COLLECTOR_REGISTRY = NULL;
	pthread_mutex_unlock(&fn_mutex);
}

void
test_large_registry(void) {
	pcr_default_init();
	for (int i = 0; i < 1000; i++) {
		char metric[6];
		sprintf(metric, "%d", i);
		pcr_must_register_metric(prom_counter_new(metric, metric, 0, NULL));
	}
	prom_registry_test_destroy();
}

void
test_pcr_must_register(void) {
	prom_registry_test_init();
	const char *labels[] = {"foo"};

	if (prom_counter_inc(test_counter, labels))
		TEST_FAIL();
	if (prom_gauge_add(test_gauge, 2.0, labels))
		TEST_FAIL();

	pms_t *test_sample_a = pms_from_labels(test_counter, labels);
	pms_t *test_sample_b = pms_from_labels(test_gauge, labels);

	TEST_ASSERT_EQUAL_DOUBLE(1.0, test_sample_a->r_value);
	TEST_ASSERT_EQUAL_DOUBLE(2.0, test_sample_b->r_value);
	prom_registry_test_destroy();
}

void
test_pcr_bridge(void) {
	prom_registry_test_init();
	if (test_histogram == NULL)
		TEST_FAIL_MESSAGE("histogram failed to initialize");

	const char *labels[] = {"foo"};
	prom_counter_inc(test_counter, labels);
	prom_gauge_set(test_gauge, 2.0, labels);
	if (prom_histogram_observe(test_histogram, 3.0, NULL))
		TEST_FAIL();
	if (prom_histogram_observe(test_histogram, 7.0, NULL))
		TEST_FAIL();

	const char *result = pcr_bridge(PROM_COLLECTOR_REGISTRY);

	const char *expected[] = {
		"# HELP test_counter counter under test",
		"# TYPE test_counter counter",
		"test_counter{label=\"foo\"}",
		"# HELP test_gauge gauge under test",
		"# TYPE test_gauge gauge",
		"test_gauge{label=\"foo\"}",
		"# HELP test_histogram histogram under test",
		"# TYPE test_histogram histogram\ntest_histogram{le=\"5.0\"}",
		"test_histogram{le=\"10.0\"}",
		"test_histogram{le=\"+Inf\"}",
		"test_histogram_count",
		"test_histogram_sum",
		"# HELP process_max_fds Max. number of open file descriptors "
			"(soft limit)",
		"# TYPE process_max_fds gauge",
		"process_max_fds",
	};

	fprintf(stderr, "%s\n", result);
	for (int i = 0; i < 15; i++) {
		TEST_ASSERT_NOT_NULL_MESSAGE(strstr(result, expected[i]), expected[i]);
	}

	free((char *)result);

	PROM_COLLECTOR_REGISTRY->features |= PROM_COMPACT;
	result = pcr_bridge(PROM_COLLECTOR_REGISTRY);
	TEST_ASSERT_NULL_MESSAGE(strstr(result, "# HELP"),"dump contains '# HELP'");
	TEST_ASSERT_NULL_MESSAGE(strstr(result, "# TYPE"),"dump contains '# TYPE'");

	prom_registry_test_destroy();
}

void
test_pcr_default_init(void) {
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, pcr_default_init(),
		"default_init() failed");
	pcr_t *pr = PROM_COLLECTOR_REGISTRY;
	TEST_ASSERT_NOT_NULL_MESSAGE(pr, "PROM_COLLECTOR_REGISTRY not set");
	TEST_ASSERT_EQUAL_INT_MESSAGE(PROM_PROCESS | PROM_SCRAPETIME, pr->features,
		"Unexpected pr->features value");
	TEST_ASSERT_EQUAL_STRING_MESSAGE(pr->mprefix, METRIC_LABEL_SCRAPE "_",
		"pr->mprefix == NULL");
	TEST_ASSERT_NOT_NULL_MESSAGE(pr->scrape_duration,
		"pr->scrape_duration == NULL");
	TEST_ASSERT_NOT_NULL_MESSAGE(pcr_get(pr, COLLECTOR_NAME_DEFAULT),
		"default collector not found");
	TEST_ASSERT_NOT_NULL_MESSAGE(pcr_get(pr, COLLECTOR_NAME_PROCESS),
		"process collector not found");
	pcr_destroy(pr);
	PROM_COLLECTOR_REGISTRY = NULL;


	TEST_ASSERT_EQUAL_INT_MESSAGE(0, pcr_init(PROM_NONE|PROM_COMPACT, NULL),
		"default_init() failed");
	pr = PROM_COLLECTOR_REGISTRY;
	TEST_ASSERT_NOT_NULL_MESSAGE(pr, "PROM_COLLECTOR_REGISTRY not set");
	TEST_ASSERT_EQUAL_INT_MESSAGE(PROM_COMPACT, pr->features,
		"Unexpected pr->features value");
	TEST_ASSERT_NULL_MESSAGE(pr->mprefix, "pr->mprefix");
	TEST_ASSERT_NULL_MESSAGE(pr->scrape_duration,
		"pr->scrape_duration != NULL");
	TEST_ASSERT_NOT_NULL_MESSAGE(pcr_get(pr, COLLECTOR_NAME_DEFAULT),
		"default collector not found");
	TEST_ASSERT_NULL_MESSAGE(pcr_get(pr, COLLECTOR_NAME_PROCESS),
		"process collector found");
	pcr_destroy(pr);
	PROM_COLLECTOR_REGISTRY = NULL;


	TEST_ASSERT_EQUAL_INT_MESSAGE(0, pcr_init(PROM_SCRAPETIME_ALL, ""),
		"default_init() failed");
	pr = PROM_COLLECTOR_REGISTRY;
	TEST_ASSERT_NOT_NULL_MESSAGE(pr, "PROM_COLLECTOR_REGISTRY not set");
	TEST_ASSERT_EQUAL_INT_MESSAGE(PROM_SCRAPETIME | PROM_SCRAPETIME_ALL,
		pr->features, "Unexpected pr->features value");
	TEST_ASSERT_NULL_MESSAGE(pr->mprefix, "pr->mprefix");
	TEST_ASSERT_NOT_NULL_MESSAGE(pr->scrape_duration,
		"pr->scrape_duration == NULL");
	TEST_ASSERT_NOT_NULL_MESSAGE(pcr_get(pr, COLLECTOR_NAME_DEFAULT),
		"default collector not found");
	TEST_ASSERT_NULL_MESSAGE(pcr_get(pr, COLLECTOR_NAME_PROCESS),
		"process collector found");
	pcr_destroy(pr);
	PROM_COLLECTOR_REGISTRY = NULL;
}

void
test_pcr_validate_metric_name(void) {
	prom_registry_test_init();

	TEST_ASSERT_EQUAL_INT(0, pcr_validate_metric_name(PROM_COLLECTOR_REGISTRY,
		"this_is_a_name09"));
	prom_registry_test_destroy();
}

static void
prom_registry_test_init(void) {
	if (pcr_init(PROM_PROCESS | PROM_SCRAPETIME, NULL)) {
		PROM_WARN("prom collector registry '%s' init() failed.",
			COLLECTOR_NAME_DEFAULT);
	}
	const char *label[] = { "label" };
	test_counter = pcr_must_register_metric(prom_counter_new("test_counter",
		"counter under test", 1, label));

	test_gauge = pcr_must_register_metric(prom_gauge_new("test_gauge",
		"gauge under test", 1, label));

	test_histogram =
		pcr_must_register_metric(prom_histogram_new("test_histogram",
			"histogram under test", phb_linear(5.0, 5.0, 2), 0, NULL));

}

static void
prom_registry_test_destroy(void) {
	pcr_destroy(PROM_COLLECTOR_REGISTRY);
	PROM_COLLECTOR_REGISTRY = NULL;
}

int
main(int argc, const char **argv) {
	UNITY_BEGIN();
	RUN_TEST(test_pcr_must_register);
	RUN_TEST(test_pcr_default_init);
	RUN_TEST(test_pcr_bridge);
	RUN_TEST(test_pcr_validate_metric_name);
	RUN_TEST(test_large_registry);
	return UNITY_END();
}
