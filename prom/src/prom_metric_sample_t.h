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

#ifndef PROM_METRIC_SAMPLE_T_H
#define PROM_METRIC_SAMPLE_T_H

#include "prom_metric_sample.h"
#include "prom_metric_t.h"

struct pms {
	prom_metric_type_t type;	/**< metric type for the sample */
	char *l_value;				/**< full metric name and label set as a str */
	_Atomic double r_value;		/**< value of the metric sample */
};

#endif  // PROM_METRIC_SAMPLE_T_H
