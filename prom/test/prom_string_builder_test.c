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

void
test_psb_add_str(void) {
	psb_t *sb = psb_new();
	psb_add_str(sb, "fooooooooooooooooooooooooooooooooo");
	psb_add_str(sb, " baaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaar");
	TEST_ASSERT_EQUAL_STRING(psb_str(sb),
	"fooooooooooooooooooooooooooooooooo baaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaar");

	psb_destroy(sb);
}

void
test_psb_add_char(void) {
	psb_t *sb = psb_new();
	const char *foobar = "foo bar";
	for (int i = 0; i < strlen(foobar); i++) {
		psb_add_char(sb, foobar[i]);
	}
	TEST_ASSERT_EQUAL_STRING(psb_str(sb), "foo bar");

	psb_destroy(sb);
}

void
test_psb_dump(void) {
	psb_t *sb = psb_new();
	const char *original = "foo bar";
	psb_add_str(sb, original);
	const char *result = psb_dump(sb);
	psb_clear(sb);
	TEST_ASSERT(original != result);
	TEST_ASSERT_EQUAL_STRING(original, result);

	psb_destroy(sb);
	free((char *) result);
}

int
main(int argc, const char **argv) {
	UNITY_BEGIN();
	RUN_TEST(test_psb_add_str);
	RUN_TEST(test_psb_add_char);
	RUN_TEST(test_psb_dump);
	return UNITY_END();
}
