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

#include "promtest_counter.h"

#include <pthread.h>

#include "parson.h"
#include "prom.h"
#include "promhttp.h"
#include "promtest_helpers.h"
#include "unity.h"

static void *PTC_handler(void *data);
static int promtest_parse_counter_output(const char *output, char **value);
prom_counter_t *foo_counter;

#define METRIC "foo_counter"			// the counter to lookup

/**
 * @brief For each thread in a threadpool of 10 we increment a single counter
 * 1 million times
 *
 * The purpose of this test is to check for deadlock and race conditions
 */
void
promtest_counter(void) {
	// Set the counter
	foo_counter = pcr_must_register_metric(prom_counter_new(METRIC ,
		"counter for foo", 0, NULL));

	if (foo_counter == NULL || promtest_daemon == NULL)
		TEST_FAIL_MESSAGE("failed to setup promtest_counter");

	void *retvals[PROMTEST_THREAD_POOL_SIZE];
	pthread_t thread_pool[PROMTEST_THREAD_POOL_SIZE];

	// Start each thread
	for (int i = 0; i < PROMTEST_THREAD_POOL_SIZE; i++) {
		if (pthread_create(&(thread_pool[i]), NULL, PTC_handler, NULL))
			TEST_FAIL_MESSAGE("failed to create thread");
	}

	// Join each thread
	for (int i = 0; i < PROMTEST_THREAD_POOL_SIZE; i++) {
		if (pthread_join(thread_pool[i], (void **) &(retvals[i])))
			TEST_FAIL_MESSAGE("thread failed to join");
	}

	// verify clean exit for each thread
	for (int i = 0; i < PROMTEST_THREAD_POOL_SIZE; i++) {
		if (*((int *)retvals[i]) != 0)
			TEST_FAIL_MESSAGE("thread did not exit properly");
	}

	// scrape the endpoint
	FILE *f = popen("prom2json " ENDPOINT , "r");
	if (f == NULL)
		TEST_FAIL_MESSAGE("prom2json pipe setup failed");
	promtest_popen_buf_t *buf = promtest_popen_buf_new(f);
	if (promtest_popen_buf_read(buf))
		TEST_FAIL_MESSAGE("failed to scrape endpoint");

	const char *output = strdup(buf->buf);
	if (promtest_popen_buf_destroy(buf))
		TEST_FAIL_MESSAGE("failed to duplicate buf");

	// Parse the output
	char *value = (char *) malloc(sizeof(char) * 100);
	if (promtest_parse_counter_output(output, &value))
		TEST_FAIL_MESSAGE("failed to parse output");

	// Assert
	TEST_ASSERT_EQUAL_STRING_MESSAGE("5e+06", value,
		"expected for " METRIC_LABEL_SCRAPE "_" METRIC);

	free(value);
}

/**
 * @brief The entrypoint to a worker thread within the prom_counter_test
 */
static void *
PTC_handler(void *data) {
	for (int i = 0; i < 1000000; i++) {
		prom_counter_inc(foo_counter, NULL);
	}
	int *retval = (int *) malloc(sizeof(int));
	*retval = 0;
	return (void *) retval;
}

/**
 * @brief Parse the output and set the value of the foo_counter metric.
 *
 * We must past a pointer to a char* so the value gets updated
 */
static int
promtest_parse_counter_output(const char *output, char **value) {
	// Parse the JSON output
	JSON_Value *root = json_parse_string(output);
	if (json_value_get_type(root) != JSONArray)
		TEST_FAIL_MESSAGE("JSON: root container is not an []");
	*value = NULL;

	JSON_Array *collection = json_value_get_array(root);
	if (collection == NULL)
		TEST_FAIL_MESSAGE("JSON: Failed to extract root [] elements");

	for (int i = 0; i < json_array_get_count(collection); i++) {
		JSON_Object *obj = json_array_get_object(collection, i);
		if (obj == NULL)
			TEST_FAIL_MESSAGE("JSON: root [] element is not an {}");
		const char *name = json_object_get_string(obj, "name");
		if (strcmp(name, METRIC_LABEL_SCRAPE "_" METRIC))
			continue;

		*value = "";
		JSON_Array *samples = json_object_dotget_array(obj, "metrics");
		if (samples == NULL)
			TEST_FAIL_MESSAGE("JSON: root [] element contains no metric []");
		if (json_array_get_count(samples) < 1)
			TEST_FAIL_MESSAGE("JSON: metric [] contains no sample");
		JSON_Object *sample = json_array_get_object(samples, 0);
		if (sample == NULL)
			TEST_FAIL_MESSAGE("JSON: failed to get first metric sample");
		*value = (char *) json_object_get_string(sample, "value");
		if (*value == NULL) {
			TEST_FAIL_MESSAGE("JSON: failed to extract metric sample[0] value");
			*value = "";
		}
		break;
	}
	if (*value == NULL)
		TEST_FAIL_MESSAGE("JSON: metric '" METRIC_LABEL_SCRAPE "_" METRIC
			"' not found");

	return (strlen(*value) == 0) ? 1 : 0;
}
