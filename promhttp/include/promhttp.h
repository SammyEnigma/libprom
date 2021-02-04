/*
 Copyright 2019-2020 DigitalOcean Inc.
 Copyright 2021 Jens Elkner <jel+libprom@cs.uni-magdeburg.de>

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

/**
 * @file promhttp.h
 * @brief Provides a HTTP endpoint for metric exposition
 * References:
 *   * MHD_FLAG: https://www.gnu.org/software/libmicrohttpd/manual/libmicrohttpd.html#microhttpd_002dconst
 *   * MHD_AcceptPolicyCallback:
 * https://www.gnu.org/software/libmicrohttpd/manual/libmicrohttpd.html#index-_002aMHD_005fAcceptPolicyCallback
 */

#include <string.h>

#include "microhttpd.h"
#include "prom_collector_registry.h"

/**
 * @brief	Sets the active registry for metric scraping.
 * @param registery	The target prom registry to generate the report to send in
 *	Prometheus exposition format. If NULL is passed, the default registry will
 *	be used.
 * @note	The registry MUST be initialized.
 */
void promhttp_set_active_collector_registry(pcr_t *registry);

/**
 *  @brief Start a daemon in the background and return a reference to it.
 *
 * References:
 *  * https://www.gnu.org/software/libmicrohttpd/manual/libmicrohttpd.html#microhttpd_002dinit
 *
 * @return A reference to the started daemon.
 */
struct MHD_Daemon *promhttp_start_daemon(unsigned int flags, unsigned short port, MHD_AcceptPolicyCallback apc, void *apc_cls);
