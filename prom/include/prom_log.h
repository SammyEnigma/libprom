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
	PLL_FATAL,		/**< fatal level */
	PLL_COUNT		/**< number of log levels */
} PROM_LOG_LEVEL;

/**
 * @brief If the given \c PROM_LOG_LEVEL is >= the log level set, generate and
 * log a related message, otherwise do nothing. Right now \c format and the
 * optional arguments get passed to \c printf() as is and the related string
 * pushed to \c stderr with a trailing linefeed.
 * The log level to compare to, if not already set, gets determined the
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

/**
 * @brief Set the current log level.
 * @param level	The loglevel to set. Use \c PLL_NONE to get the loglevel
 *	currently set returned and the current loglevel stays untouched.
 * @return The previously loglevel used (might be PLL_NONE, if not yet set).
 */
PROM_LOG_LEVEL prom_log_level(PROM_LOG_LEVEL level);

/**
 * @brief Parse the given log \c level name (for convenience, a single digit
 * gets recognized as well) and return its corresponding log level value.
 * @param level		Name of the log level or its corresponding value to parse.
 * @return \c PLL_NONE if the given \c level name could not be parsed
 *	successfully, the corresponding log level value otherwise.
 */
PROM_LOG_LEVEL prom_log_level_parse(const char *level);

/**
 * @brief Use the given \c stream for logging. This function duplicates the
 * stream's underlying file descriptor, opens it in \c APPEND mode and writes
 * from now on all messages to it. It gets automatically flushed and synced
 * linewise. The callee should close the given stream to avoid any garbage.
 * @param stream	Stream to use for logging. \c NULL or invalid streams are
 *	ignored.
 * @return \c NULL on failure, the previously used logging stream (which is
 * initially \c stderr ) otherwise. The callee should close it, if not needed
 * anymore.
 */
FILE *prom_log_use(FILE *stream);

#define PROM_LOG_PRIV(level, fmt, ...)	\
	prom_log(level, "%s:%d::%s(): " fmt , \
		__FILE__, __LINE__, __func__, __VA_ARGS__);

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

#define prom_log(x, fmt, ...)
#define prom_log_level(x) 0
#define prom_log_level_parse(x) 0
#define prom_log_use(x) NULL

#endif  // PROM_LOG_ENABLE

#endif  // PROM_LOG_H
