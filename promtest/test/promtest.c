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

#include <pthread.h>
#include "promhttp.h"

#include "promtest_counter.h"
#include "promtest_gauge.h"
#include "promtest_histogram.h"
#include "promtest_helpers.h"
#include "unity.h"

pthread_mutex_t smoke_mutex = PTHREAD_MUTEX_INITIALIZER;

void
setUp(void) {
	pthread_mutex_lock(&smoke_mutex);
	// Initialize the default collector registry
	pcr_default_init();
	// Set the collector registry on the handler to the default registry
	promhttp_set_active_collector_registry(NULL);
	// Start the HTTP server
	promtest_daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY
		| MHD_USE_ERROR_LOG | MHD_USE_AUTO, PORT, NULL, NULL);
}

void
tearDown(void) {
	// Destroy the default registry. This effectively deallocates all metrics
	// registered to it, including itself
	pcr_destroy(PROM_COLLECTOR_REGISTRY);
	PROM_COLLECTOR_REGISTRY = NULL;

	// Stop the HTTP server
	if (promtest_daemon != NULL)
		MHD_stop_daemon(promtest_daemon);
	promtest_daemon = NULL;
	pthread_mutex_unlock(&smoke_mutex);
}

int
main(int argc, const char **argv) {
	UNITY_BEGIN();
	RUN_TEST(promtest_counter);
	RUN_TEST(promtest_gauge);
	RUN_TEST(promtest_histogram);
	return UNITY_END();
}
