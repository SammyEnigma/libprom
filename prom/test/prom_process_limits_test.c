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
#include <stdlib.h>
#include <string.h>

#include "prom_map_i.h"
#include "prom_metric_sample_t.h"
#include "prom_process_limits_i.h"
#include "prom_process_limits_t.h"
#include "unity.h"

const char *path = "../test/fixtures/limits";

void
test_ppl_file_parsing(void) {
	TEST_ASSERT_EQUAL_INT(0, ppl_init());
	TEST_ASSERT_EQUAL_INT(0, ppl_init());	// a 2nd time does no harm
	TEST_ASSERT_NOT_NULL(prom_process_max_fds);

	TEST_ASSERT_EQUAL_INT(0, ppl_update(path));
	pms_t *sample = pms_from_labels(prom_process_max_fds, NULL);
	TEST_ASSERT_EQUAL_INT(sample->r_value, 1048576);

	ppl_cleanup();
	TEST_ASSERT_NULL(prom_process_max_fds);
}

int
main(int argc, const char **argv) {
	UNITY_BEGIN();
	RUN_TEST(test_ppl_file_parsing);
	return UNITY_END();
}
