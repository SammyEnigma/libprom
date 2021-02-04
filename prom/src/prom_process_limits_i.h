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

#ifndef PROM_PROCESS_I_H
#define PROM_PROCESS_I_H

#include <stdbool.h>

#include "prom_map_t.h"
#include "prom_process_limits_t.h"

/**
 * @brief Initialize the process gauge metrics
 */
int prom_process_init(void);

ppl_row_t *ppl_row_new(const char *limit, const int soft, const int hard, const char *units);

int ppl_row_destroy(ppl_row_t *self);

ppl_current_row_t *ppl_current_row_new(void);

int ppl_current_row_set_limit(ppl_current_row_t *self, char *limit);

int ppl_current_row_set_units(ppl_current_row_t *self, char *units);

int ppl_current_row_clear(ppl_current_row_t *self);

int ppl_current_row_destroy(ppl_current_row_t *self);

ppl_file_t *ppl_file_new(const char *path);

int ppl_file_destroy(ppl_file_t *self);

prom_map_t *ppl(ppl_file_t *f);

bool ppl_rdp_file(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

bool ppl_rdp_first_line(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

bool ppl_rdp_character(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

bool ppl_rdp_letter(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

bool ppl_rdp_digit(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

bool ppl_rdp_data_line(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

bool ppl_rdp_limit(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

bool ppl_rdp_word_and_space(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

bool ppl_rdp_word(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

bool ppl_rdp_soft_limit(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

bool ppl_rdp_hard_limit(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

bool ppl_rdp_units(ppl_file_t *f, prom_map_t *data, ppl_current_row_t *current_row);

int ppl_rdp_next_token(ppl_file_t *f);

bool ppl_rdp_match(ppl_file_t *f, const char *token);

int ppl_init(void);

void ppl_cleanup(void);

#endif  // PROM_PROCESS_I_H
