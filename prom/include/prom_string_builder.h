/*
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

/**
 * @file prom_string_builder.h
 * A StringBuilder: uses an internal buffer to append strings and characters
 * as needed and keeps track of the constructed string, grows the buffer by
 * factor 2 automagically.
 *
 * @note	All \c psb_t parameter in the related functions are expected to
 *	be != \c NULL, otherwise be prepared for core dumps.
 *
 */
#ifndef PROM_STRING_BUILDER_I_H
#define PROM_STRING_BUILDER_I_H

#include <stddef.h>

/** @brief A string builder. */
struct psb;
typedef struct psb psb_t;

/**
 * @brief	Setup a new string builder.
 * @return A new string builder or \c NULL on error.
 */
psb_t *psb_new(void);

/**
 * @brief	Destroys the given string builder.
 * @param self	String builder to destroy.
 * @return \c 0 .
 */
int psb_destroy(psb_t *self);

/**
 * @brief Append the given string to the buffered string of the given
 *	string builder.
 * @param self	Where to append the string.
 * @param str	String to append.
 * @return \c 0 on success, a number > 0 otherwise.
 */
int psb_add_str(psb_t *self, const char *str);

/**
 * @brief Append the given character to the buffered string of the given
 *	string builder.
 * @param self	Where to append the character.
 * @param c		Character to append.
 * @return \c 0 on success, a number > 0 otherwise.
 */
int psb_add_char(psb_t *self, char c);

/**
 * @brief Free the allocated string buffer area of the given string builder,
 * allocate a new one with a default initial size and set its length to \c 0.
 * @param self	String builder to free.
 * @return \c 0 on success, a number > 0 otherwise.
 */
int psb_clear(psb_t *self);

/**
 * @brief Set the length of the buffered string of the given string builder
 *	to the given length and append \c '\0'.
 * @param self	String builder to truncate.
 * @param len	The new length of the buffered string. If it is bigger than
 *	its current length, this function is a no-op.
 * @return \c 0 on success, a number > 0 otherwise.
 */
int psb_truncate(psb_t *self, size_t len);

/**
 * @brief Get the length of the buffered string of the given string builder.
 * @param self	String builder to query.
 * @return The length of the buffered string.
 */
size_t psb_len(psb_t *self);

/**
 * @brief Get a copy of the buffered string of the given string builder.
 * @param self	String builder to ask.
 * @return Metric as string in Prometheus exposition format.
 * @note	The returned string must be freed when no longer needed.
 */
char *psb_dump(psb_t *self);

/**
 * @brief Get a reference to the buffered string. One should **NOT** modify
 * the string, treat it as a read-only and never ever free() it, otherwise
 * be prepared for buffer overflows and core dumps. It has been made public
 * solely to avoid the strdup() overhead of \c psb_dump().
 * @return a string, or \c NULL if not initialized yet.
 */
char *psb_str(psb_t *self);

#endif  // PROM_STRING_BUILDER_I_H
