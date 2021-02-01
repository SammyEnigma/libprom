/**
 * Copyright 2021 Jens Elkner <jel+libprom@cs.uni-magdeburg.de>.
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

#ifndef PROM_LOG_H
#define PROM_LOG_H

#ifdef PROM_LOG_ENABLE

typedef enum {
	PLL_NONE = 0,
	PLL_DBG,
	PLL_INFO,
	PLL_WARN,
	PLL_ERR,
	PLL_FATAL
} PROM_LOG_LEVEL;

void prom_log(PROM_LOG_LEVEL level, const char* format, ...);

#define PROM_LOG_PRIV(level, fmt, ...)	\
	prom_log(level, "%s:%d::%s(): " fmt , \
		__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);

#define PROM_DEBUG(fmt, ...)	PROM_LOG_PRIV(PLL_DBG, fmt, __VA_ARGS__);
#define PROM_INFO(fmt, ...)		PROM_LOG_PRIV(PLL_INFO, fmt, __VA_ARGS__);
#define PROM_WARN(fmt, ...)		PROM_LOG_PRIV(PLL_WARN, fmt, __VA_ARGS__);
#define PROM_ERROR(fmt, ...)	PROM_LOG_PRIV(PLL_ERR, fmt, __VA_ARGS__);
#define PROM_FATAL(fmt, ...)	PROM_LOG_PRIV(PLL_FATAL, fmt, __VA_ARGS__);
#define PROM_LOG(msg)			PROM_INFO("%s", msg);

#else

#define PROM_DEBUG(fmt, ...)
#define PROM_INFO(fmt, ...)
#define PROM_WARN(fmt, ...)
#define PROM_ERROR(fmt, ...)
#define PROM_FATAL(fmt, ...)
#define PROM_LOG(msg)

#endif  // PROM_LOG_ENABLE

#endif  // PROM_LOG_H
