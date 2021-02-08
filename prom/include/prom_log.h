/*
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

/**
 * @file prom_log.h
 * @brief logging
 */

#ifndef PROM_LOG_H
#define PROM_LOG_H

#ifdef PROM_LOG_ENABLE

#include <stdio.h>

/**
 * @brief Available log levels.
 */
typedef enum {
	PLL_NONE = 0,	/**< placeholder for \c 0 - implies nothing. Do not use! */
	PLL_DBG,		/**< debug level */
	PLL_INFO,		/**< info level */
	PLL_WARN,		/**< warning level */
	PLL_ERR,		/**< error level */
	PLL_FATAL		/**< fatal level */
} PROM_LOG_LEVEL;

/**
 * @brief If the given \c PROM_LOG_LEVEL is >= the log level set, generate and
 * log a related message, otherwise do nothing. Right now \c format and the
 * optional arguments get passed to \c printf() as is and the related string
 * pushed to \c stderr with a trailing linefeed.
 * The log level to compare to gets determined the
 * first time this function gets called: It evaluates the environment variable
 * **PROM_LOG_LEVEL**. If it is unset or has an unknown value, \c INFO will be
 * used. Otherwise the corresponding level for \c DEBUG, \c INFO, \c WARN,
 * \c ERROR, or \c FATAL.
 *
 * @param PROM_LOG_LEVEL	Log level to use to decide, whether to log.
 * @param format			Same as for \c printf(3).
 * @param ...				Optional format args.
 */
void prom_log(PROM_LOG_LEVEL level, const char* format, ...);


#define PROM_LOG_PRIV(level, fmt, ...)	\
	prom_log(level, "%s:%d::%s(): " fmt , \
		__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);

/**
 * @brief Log a debug message prefixed with the location of this macro
 * within the source code (file, line, function).
 */
#define PROM_DEBUG(fmt, ...)	PROM_LOG_PRIV(PLL_DBG, fmt, __VA_ARGS__);
/**
 * @brief Log an info message prefixed with the location of this macro
 * within the source code (file, line, function).
 */
#define PROM_INFO(fmt, ...)		PROM_LOG_PRIV(PLL_INFO, fmt, __VA_ARGS__);
/**
 * @brief Log a warn message prefixed with the location of this macro
 * within the source code (file, line, function).
 */
#define PROM_WARN(fmt, ...)		PROM_LOG_PRIV(PLL_WARN, fmt, __VA_ARGS__);
/**
 * @brief Log an error message prefixed with the location of this macro
 * within the source code (file, line, function).
 */
#define PROM_ERROR(fmt, ...)	PROM_LOG_PRIV(PLL_ERR, fmt, __VA_ARGS__);
/**
 * @brief Log a fatal message prefixed with the location of this macro
 * within the source code (file, line, function).
 */
#define PROM_FATAL(fmt, ...)	PROM_LOG_PRIV(PLL_FATAL, fmt, __VA_ARGS__);
/**
 * @brief Log a info message prefixed with the location of this macro
 * within the source code (file, line, function), which has no optional arg.
 */
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
