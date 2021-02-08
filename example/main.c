/**
 * Copyright 2019 DigitalOcean Inc.
 * Copyright 2020 Jens Elkner <jel+libprom@cs.uni-magdeburg.de>
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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "microhttpd.h"
#include "prom.h"
#include "prom_histogram.h"
#include "promhttp.h"
#include "prom_log.h"
#include "foo.h"
#include "bar.h"


prom_histogram_t *test_histogram;

static void
init(void) {
	prom_histogram_t *metric;

	// Initialize the Default registry
	pcr_default_init();

	// Register file-based metrics for each file
	foo_init();
	bar_init();

	metric = prom_histogram_new("test_histogram",
		"histogram under test", phb_linear(5.0, 5.0, 2), 0, NULL);
	test_histogram = pcr_must_register_metric(metric);

	// Set the active registry for the HTTP handler
	promhttp_set_active_collector_registry(NULL);
}

#define PORT 8000

int
main(int argc, const char **argv) {
	init();

	const char *labels[] = { "one", "two", "three", "four", "five" };
	for (int i = 1; i <= 100; i++) {
		double hist_value = (i % 2 == 0) ? 3.0 : 7.0;

		if (prom_histogram_observe(test_histogram, hist_value, NULL))
			exit(1);

		for (int x = 0; x < 5; x++) {
			if (foo(i, labels[x]))
				exit(2);
			if (bar(i+x, labels[x]))
				exit(3);
		}
	}

	unsigned int flags;
	// Handle each request one after a another using the best event loop style
	// (SELECT, POLL, or EPOLL) for the current platform and log any errors
	// to stderr.
	flags = MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_AUTO | MHD_USE_ERROR_LOG;
	// Or answer requests in parallel:
//	flags = MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD;

	struct MHD_Daemon *daemon = promhttp_start_daemon(flags, PORT, NULL, NULL);
	if (daemon == NULL)
		return 1;

	int done = 0;

	auto void intHandler(int signal);
	void intHandler(int signal) {
		PROM_LOG("\nshutting down...");
		pcr_destroy(PROM_COLLECTOR_REGISTRY);
		MHD_stop_daemon(daemon);
		done = 1;
	}

	if (argc == 2) {
		unsigned int timeout = atoi(argv[1]);
		sleep(timeout);
		intHandler(0);
		return 0;
	}

	PROM_INFO("RUN 'curl localhost:%d/metrics'\n", PORT);
	signal(SIGINT, intHandler);

	// main application goes here - for simplicity it just waits for ^C
	while (! done) {
		fputc('.', stdout); fflush(stdout);
		sleep(1);
	}
	PROM_LOG("Done.");

	return 0;
}
