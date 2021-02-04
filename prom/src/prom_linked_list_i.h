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

#ifndef PROM_LIST_I_INCLUDED
#define PROM_LIST_I_INCLUDED

// Private
#include "prom_linked_list_t.h"

/**
 * @brief PRIVATE Returns a pointer to a pll
 */
pll_t *pll_new(void);

/**
 * @brief PRIVATE removes all nodes from the given pll *
 */
int pll_purge(pll_t *self);

/**
 * @brief PRIVATE Destroys a pll
 */
int pll_destroy(pll_t *self);

/**
 * @brief PRIVATE Append an item to the back of the list
 */
int pll_append(pll_t *self, void *item);

/**
 * @brief PRIVATE Push an item onto the front of the list
 */
int pll_push(pll_t *self, void *item);

/**
 * @brief PRIVATE Pop the first item off of the list
 */
void *pll_pop(pll_t *self);

/**
 * @brief PRIVATE Get the item at the head of the list or NULL if not present
 */
void *pll_first(pll_t *self);

/**
 * @brief PRIVATE Get the item at the tail of the list or NULL if not present
 */
void *pll_last(pll_t *self);

/**
 * @brief PRIVATE Removes an item from the linked list
 */
int pll_remove(pll_t *self, void *item);

/**
 * @brief PRIVATE Compares two items within a linked list
 */
pll_compare_t pll_compare(pll_t *self, void *item_a, void *node_b);

/**
 * @brief PRIVATE Get the size
 */
size_t pll_size(pll_t *self);

/**
 * @brief PRIVATE Set the free_fn member on pll
 */
int pll_set_free_fn(pll_t *self, pll_free_item_fn free_fn);

/**
 * @brief PRIVATE Set the compare_fn member on the pll
 */
int pll_set_compare_fn(pll_t *self, pll_compare_item_fn compare_fn);

/**
 * @brief PRIVATE does nothing
 */
void pll_no_op_free(void *item);

#endif  // PROM_LIST_I_INCLUDED
