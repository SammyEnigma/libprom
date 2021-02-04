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

#ifndef PROM_LIST_T_H
#define PROM_LIST_T_H

#include "prom_linked_list.h"

typedef enum {
	PROM_LESS = -1,
	PROM_EQUAL = 0,
	PROM_GREATER = 1
} pll_compare_t;

/**
 * @brief PRIVATE Frees an item in a pll_node
 */
typedef void (*pll_free_item_fn)(void *);

/**
 * @brief PRIVATE Compares two items within a pll
 */
typedef pll_compare_t (*pll_compare_item_fn)(void *item_a, void *item_b);

/**
 * @brief PRIVATE A struct containing a generic item, represented as a
 *	void pointer, and next, a pointer to the next pll_node*
 */
typedef struct pll_node {
	struct pll_node *next;
	void *item;
} pll_node_t;

/**
 * @brief PRIVATE A linked list comprised of pll_node* instances
 */
struct pll {
	pll_node_t *head;
	pll_node_t *tail;
	size_t size;
	pll_free_item_fn free_fn;
	pll_compare_item_fn compare_fn;
};

#endif  // PROM_LIST_T_H
