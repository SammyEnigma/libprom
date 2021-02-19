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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "prom_gauge.h"

#include "prom_metric_sample_t.h"
#include "prom_process_collector_t.h"
#include "prom_process_limits_i.h"
#include "unity.h"

const char *path = "../test/fixtures/limits";

void
test_ppc_limits(void) {
	prom_metric_t *m[PM_COUNT];
	memset(m, 0, sizeof(m));
	TEST_ASSERT_NULL(m[PM_MAX_FDS]);
	TEST_ASSERT_EQUAL_INT(1 << PM_MAX_FDS, ppc_limits_new(m, NULL));
	TEST_ASSERT_NOT_NULL(m[PM_MAX_FDS]);

	prom_metric_t *n[PM_COUNT];
	memset(n, 0, sizeof(n));
	TEST_ASSERT_NULL(n[PM_MAX_FDS]);
	TEST_ASSERT_EQUAL_INT(1 << PM_MAX_FDS, ppc_limits_new(n, NULL));
	TEST_ASSERT_NOT_NULL(n[PM_MAX_FDS]);
	TEST_ASSERT_TRUE(m[PM_MAX_FDS] != n[PM_MAX_FDS]);

	int fd[FD_COUNT];
	fd[FD_LIMITS] = open(path, O_RDONLY, 0666);
	TEST_ASSERT_TRUE(fd[FD_LIMITS] >= 0);
	TEST_ASSERT_EQUAL_INT(1 << PM_MAX_FDS, ppc_limits_update(fd, m, NULL));
	pms_t *sample = pms_from_labels(m[PM_MAX_FDS], NULL);
	TEST_ASSERT_EQUAL_INT(1048576, sample->r_value);

	prom_gauge_destroy(m[PM_MAX_FDS]);
	prom_gauge_destroy(n[PM_MAX_FDS]);
	close(fd[FD_LIMITS]);
}

int
main(int argc, const char **argv) {
	UNITY_BEGIN();
	RUN_TEST(test_ppc_limits);
	return UNITY_END();
}
