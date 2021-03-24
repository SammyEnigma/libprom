/**
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

#include "prom_test_helpers.h"
#include "prom_log.h"

// modifies "global" var, so prevent running these tests concurrently.
pthread_mutex_t fn_mutex = PTHREAD_MUTEX_INITIALIZER;

void
setUp(void) {
	pthread_mutex_lock(&fn_mutex);
}

void
tearDown(void) {
	pthread_mutex_unlock(&fn_mutex);
}


void
test_prom_loglevels(void) {
	char buf[8];
	PROM_LOG_LEVEL a, b = PLL_NONE, c;
	FILE *zero = prom_log_use(NULL);
	TEST_ASSERT_NULL_MESSAGE(zero, "Initial stream != NULL");
	a = prom_log_level(PLL_NONE);
	TEST_ASSERT_TRUE_MESSAGE(a == b, "Initial level != PLL_NONE");
	PROM_INFO("Testing levels %s", "...");
	a = prom_log_level(PLL_NONE);
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_INFO, "First time level != PLL_INFO");
	c = prom_log_level(PLL_WARN);
	TEST_ASSERT_TRUE_MESSAGE(c == PLL_INFO, "Returned log level != PLL_INFO");
	a = prom_log_level(PLL_NONE);
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_WARN, "log level != PLL_WARN");
	a = prom_log_level(PLL_INFO);
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_WARN, "old log level != PLL_WARN");

	a = prom_log_level_parse("DEBUG");
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_DBG, "parse != PLL_DEBUG");
	sprintf(buf, "%u", PLL_DBG);
	a = prom_log_level_parse(buf);
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_DBG, "parse != PLL_DEBUG");

	a = prom_log_level_parse("INFO");
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_INFO, "parse != PLL_INFO");
	sprintf(buf, "%u", PLL_INFO);
	a = prom_log_level_parse(buf);
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_INFO, "parse != PLL_INFO");

	a = prom_log_level_parse("WARN");
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_WARN, "parse != PLL_WARN");
	sprintf(buf, "%u", PLL_WARN);
	a = prom_log_level_parse(buf);
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_WARN, "parse != PLL_WARN");

	a = prom_log_level_parse("ERROR");
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_ERR, "parse != PLL_ERROR");
	sprintf(buf, "%u", PLL_ERR);
	a = prom_log_level_parse(buf);
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_ERR, "parse != PLL_ERROR");

	a = prom_log_level_parse("FATAL");
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_FATAL, "parse != PLL_FATAL");
	sprintf(buf, "%u", PLL_FATAL);
	a = prom_log_level_parse(buf);
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_FATAL, "parse != PLL_FATAL");

	a = prom_log_level_parse(NULL);
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_NONE, "parse != PLL_NONE");
	a = prom_log_level_parse("xxx");
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_NONE, "parse != PLL_NONE");
	sprintf(buf, "%u", PLL_COUNT + 1);
	a = prom_log_level_parse(buf);
	TEST_ASSERT_TRUE_MESSAGE(a == PLL_NONE, "parse != PLL_NONE");
}

#define TEST_MSG "Logging to stream"
void
test_prom_logstream(void) {
	FILE *f, *fold;
	int fd = -1, a, b;
	char *template = strdup("/tmp/promLogXXXXXX"), pc[128], *p;

	prom_log_level(PLL_INFO);
	PROM_INFO("Testing streams %s", "...");
	f = prom_log_use(NULL);
	TEST_ASSERT_NULL_MESSAGE(f, "NULL arg should cause NULL result");
	fd = mkstemp(template);
	TEST_ASSERT_TRUE_MESSAGE(fd != -1, "Failed to create temp file");
	f = fdopen(fd, "a");
	TEST_ASSERT_NOT_NULL_MESSAGE(f, "Failed to open tempfile");
	fold = prom_log_use(f);
	a = fileno(stderr);
	b = fileno(fold);
	TEST_ASSERT_TRUE_MESSAGE(a == b, "Default log stream != stderr");
	fclose(f);
	fprintf (stderr, "tempfile = '%s'\n", template);
	PROM_INFO(TEST_MSG, "");
	f = fopen(template, "r");
	TEST_ASSERT_NOT_NULL_MESSAGE(f, "Unable to open tempfile");
	p = fgets(pc, sizeof(pc), f);
	TEST_ASSERT_NOT_NULL_MESSAGE(p, "Reading tempfile failed");
	TEST_ASSERT_TRUE_MESSAGE(strncmp(TEST_MSG,
		pc + strlen(pc) - strlen(TEST_MSG) - 1, strlen(TEST_MSG)) == 0,
		"Msg not logged to tmpfile");
	unlink(template);
}

int
main(int argc, const char **argv) {
	putenv("PROM_LOG_LEVEL=");
	UNITY_BEGIN();
	RUN_TEST(test_prom_loglevels);
	RUN_TEST(test_prom_logstream);
	return UNITY_END();
}
