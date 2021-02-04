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

#include <pthread.h>
#include <stdbool.h>

// Public
#include "prom_alloc.h"

// Private
#include "prom_assert.h"
#include "prom_errors.h"
#include "prom_linked_list_i.h"
#include "prom_linked_list_t.h"
#include "prom_log.h"
#include "prom_map_i.h"
#include "prom_map_t.h"

#define PROM_MAP_INITIAL_SIZE 32

static void
destroy_map_node_value_no_op(void *value) {
	// no op
}

//////////////////////////////////////////////////////////////////////////////
// prom_map_node
//////////////////////////////////////////////////////////////////////////////

prom_map_node_t *
prom_map_node_new(const char *key, void *value,
	prom_map_node_free_value_fn free_value_fn)
{
	prom_map_node_t *self = prom_malloc(sizeof(prom_map_node_t));
	if (self == NULL)
		return NULL;
	self->key = prom_strdup(key);
	self->value = value;
	self->free_value_fn = free_value_fn;
	return self;
}

int
prom_map_node_destroy(prom_map_node_t *self) {
	if (self == NULL)
		return 0;
	prom_free((void *)self->key);
	self->key = NULL;
	if (self->value != NULL)
		(*self->free_value_fn)(self->value);
	self->value = NULL;
	prom_free(self);
	return 0;
}

void
prom_map_node_free(void *item) {
	prom_map_node_destroy((prom_map_node_t *) item);
}

pll_compare_t
prom_map_node_compare(void *item_a, void *item_b) {
	prom_map_node_t *a = (prom_map_node_t *) item_a;
	prom_map_node_t *b = (prom_map_node_t *) item_b;
	if (a == b)
		return 0;
	if (a == NULL)
		return -1;
	if (b == NULL)
		return 1;

	if (a->key == b->key)
		return 0;
	if (a->key == NULL)
		return -1;
	if (b ->key == NULL)
		return 1;
	return strcmp(a->key, b->key);
}

//////////////////////////////////////////////////////////////////////////////
// prom_map
//////////////////////////////////////////////////////////////////////////////

prom_map_t *
prom_map_new() {
	prom_map_t *self = (prom_map_t *) prom_malloc(sizeof(prom_map_t));
	if (self == NULL)
		return NULL;

	self->size = 0;
	self->max_size = PROM_MAP_INITIAL_SIZE;
	self->free_value_fn = destroy_map_node_value_no_op;
	self->addrs = NULL;
	self->rwlock = NULL;

	self->keys = pll_new();
	if (self->keys == NULL) {
		prom_free(self);
		return NULL;
	}

	// These each key will be allocated once by prom_map_node_new and used here
	// as well to save memory. With that said we will only have to deallocate
	// each key once. That will happen on prom_map_node_destroy.
	if (pll_set_free_fn(self->keys, pll_no_op_free))
		goto fail;

	self->addrs = prom_malloc(sizeof(pll_t) * self->max_size);
	if (self->addrs == NULL)
		goto fail;

	for (int i = 0; i < self->max_size; i++) {
		self->addrs[i] = pll_new();
		if (pll_set_free_fn(self->addrs[i], prom_map_node_free))
			goto fail;
		if (pll_set_compare_fn(self->addrs[i], prom_map_node_compare))
			goto fail;
	}

	self->rwlock = (pthread_rwlock_t *) prom_malloc(sizeof(pthread_rwlock_t));
	if (self->rwlock == NULL)
		goto fail;
	if (pthread_rwlock_init(self->rwlock, NULL)) {
		PROM_WARN(PROM_PTHREAD_RWLOCK_INIT_ERROR, NULL);
		goto fail;
	}

	return self;

fail:
	prom_map_destroy(self);
	return NULL;
}

int
prom_map_destroy(prom_map_t *self) {
	if (self == NULL)
		return 0;

	pll_destroy(self->keys);
	self->keys = NULL;
	for (size_t i = 0; i < self->max_size; i++) {
		pll_destroy(self->addrs[i]);
		self->addrs[i] = NULL;
	}
	prom_free(self->addrs);
	self->addrs = NULL;
	pthread_rwlock_destroy(self->rwlock);
	prom_free(self->rwlock);
	self->rwlock = NULL;
	prom_free(self);
	return 0;
}

static size_t
prom_map_get_index_internal(const char *key, size_t *size, size_t *max_size) {
	PROM_ASSERT(key != NULL);
	size_t index;
	size_t a = 31415, b = 27183;
	for (index = 0; *key != '\0'; key++, a = a * b % (*max_size - 1)) {
		index = (a * index + *key) % *max_size;
	}
	return index;
}

/**
 * @brief PRIVATE hash function that returns an array index from the given
 *	key and prom_map.
 *
 * The algorithm is based off of Horner's method. In a simpler version, you set
 * the return value to 0. Next, for each character in the string, you add the
 * integer value of the current character to the product of the prime number and
 * the current return value, set the result to the return value, then finally
 * return the return value.
 *
 * In this version of the algorithm, we attempt to achieve a probabily of key
 * to index conversion collisions to 1/M (with M being the max_size of the map).
 * This optimizes dispersion and consequently, evens out the performance for
 * gets and sets for each item. Instead of using a fixed prime number, we
 * generate a coefficient for each iteration through the loop.
 *
 * Reference:
 *   * Algorithms in C: Third Edition by Robert Sedgewick, p579
 */
size_t
prom_map_get_index(prom_map_t *self, const char *key) {
  return prom_map_get_index_internal(key, &self->size, &self->max_size);
}

static void *
prom_map_get_internal(const char *key, size_t *size, size_t *max_size,
	pll_t *keys, pll_t **addrs, prom_map_node_free_value_fn free_value_fn)
{
	if (key == NULL)
		return NULL;

	size_t index = prom_map_get_index_internal(key, size, max_size);
	pll_t *list = addrs[index];
	prom_map_node_t *temp_map_node = prom_map_node_new(key, NULL,free_value_fn);

	for (pll_node_t *current_node = list->head;
		current_node != NULL; current_node = current_node->next)
	{
		prom_map_node_t *current_map_node = (prom_map_node_t *)
			current_node->item;
		pll_compare_t result =
			pll_compare(list, current_map_node, temp_map_node);
		if (result == PROM_EQUAL) {
			prom_map_node_destroy(temp_map_node);
			temp_map_node = NULL;
			return current_map_node->value;
		}
	}
	prom_map_node_destroy(temp_map_node);
	return NULL;
}

void *
prom_map_get(prom_map_t *self, const char *key) {
	if (key == NULL)
		return NULL;

	if (pthread_rwlock_wrlock(self->rwlock)) {
		PROM_WARN(PROM_PTHREAD_RWLOCK_LOCK_ERROR, NULL);
		return NULL;
	}

	void *value = prom_map_get_internal(key, &self->size, &self->max_size,
		self->keys, self->addrs, self->free_value_fn);

	if (pthread_rwlock_unlock(self->rwlock))
		PROM_WARN(PROM_PTHREAD_RWLOCK_UNLOCK_ERROR, NULL);

	return value;
}

static int
prom_map_set_internal(const char *key, void *value, size_t *size,
	size_t *max_size, pll_t *keys, pll_t **addrs,
	prom_map_node_free_value_fn free_value_fn, bool destroy_current_value)
{
	prom_map_node_t *map_node = prom_map_node_new(key, value, free_value_fn);
	if (map_node == NULL)
		return 1;

	size_t index = prom_map_get_index_internal(key, size, max_size);
	pll_t *list = addrs[index];
	for (pll_node_t *current_node = list->head;
		current_node != NULL; current_node = current_node->next)
	{
		prom_map_node_t *current_map_node = (prom_map_node_t *)
			current_node->item;
		pll_compare_t result =
			pll_compare(list, current_map_node, map_node);
		if (result != PROM_EQUAL)
			continue;

		if (destroy_current_value) {
			free_value_fn(current_map_node->value);
			current_map_node->value = NULL;
		}
		prom_free((char *) current_map_node->key);
		current_map_node->key = NULL;
		prom_free(current_map_node);
		current_map_node = NULL;
		current_node->item = map_node;
		return 0;
	}
	pll_append(list, map_node);
	pll_append(keys, (char *) map_node->key);
	(*size)++;
	return 0;
}

int
prom_map_ensure_space(prom_map_t *self) {
	PROM_ASSERT(self != NULL);

	if (self->size <= self->max_size >> 1)
		return 0;

	// Increase the max size
	size_t new_max = self->max_size << 1;
	size_t new_size = 0;

	// Create a new list of keys
	pll_t *new_keys = pll_new();
	if (new_keys == NULL)
		return 1;

	if (pll_set_free_fn(new_keys, pll_no_op_free))
		return 2;

	// Create a new array of addrs
	pll_t **new_addrs =
		prom_malloc(sizeof(pll_t) * new_max);

	// Initialize the new array
	for (int i = 0; i < new_max; i++) {
		new_addrs[i] = pll_new();
		if (pll_set_free_fn(new_addrs[i], prom_map_node_free))
			return 3;
		if (pll_set_compare_fn(new_addrs[i],prom_map_node_compare))
			return 4;
	}

	// Iterate through each linked-list at each memory region in the map's
	// backbone
	for (int i = 0; i < self->max_size; i++) {
		// Create a new map node for each node in the linked list and insert it
		// into the new map. Afterwards, deallocate the old map node
		pll_t *list = self->addrs[i];
		pll_node_t *current_node = list->head;
		while (current_node != NULL) {
			prom_map_node_t *map_node = (prom_map_node_t *) current_node->item;
			if (prom_map_set_internal(map_node->key, map_node->value,
				&new_size, &new_max, new_keys, new_addrs, self->free_value_fn,
				false))
			{
				return 5;
			}
			pll_node_t *next = current_node->next;
			prom_free(current_node);
			current_node = NULL;
			prom_free((void *) map_node->key);
			map_node->key = NULL;
			prom_free(map_node);
			map_node = NULL;
			current_node = next;
		}
		// We're done deallocating each map node in the linked list, so
		// deallocate the linked-list object
		prom_free(self->addrs[i]);
		self->addrs[i] = NULL;
	}
	// Destroy the collection of keys in the map
	pll_destroy(self->keys);
	self->keys = NULL;

	// Deallocate the backbone of the map
	prom_free(self->addrs);
	self->addrs = NULL;

	// Update the members of the current map
	self->size = new_size;
	self->max_size = new_max;
	self->keys = new_keys;
	self->addrs = new_addrs;
	return 0;
}

int
prom_map_set(prom_map_t *self, const char *key, void *value) {
	PROM_ASSERT(self != NULL);
	if (pthread_rwlock_wrlock(self->rwlock)) {
		PROM_WARN(PROM_PTHREAD_RWLOCK_LOCK_ERROR, NULL);
		return 1;
	}

	if (prom_map_ensure_space(self)) {
		if (pthread_rwlock_unlock(self->rwlock))
			PROM_WARN(PROM_PTHREAD_RWLOCK_UNLOCK_ERROR, NULL);
		return 2;
	}
	if (prom_map_set_internal(key, value, &self->size, &self->max_size,
		self->keys, self->addrs, self->free_value_fn, true))
	{
		if (pthread_rwlock_unlock(self->rwlock))
			PROM_WARN(PROM_PTHREAD_RWLOCK_UNLOCK_ERROR, NULL);
		return 3;
	}
	if (pthread_rwlock_unlock(self->rwlock))
		PROM_WARN(PROM_PTHREAD_RWLOCK_UNLOCK_ERROR, NULL);
	return 0;
}

static int
prom_map_delete_internal(const char *key, size_t *size, size_t *max_size,
	pll_t *keys, pll_t **addrs, prom_map_node_free_value_fn free_value_fn)
{
	PROM_ASSERT(key != NULL);
	size_t index = prom_map_get_index_internal(key, size, max_size);
	pll_t *list = addrs[index];
	prom_map_node_t *temp_map_node =
		prom_map_node_new(key, NULL, free_value_fn);

	for (pll_node_t *current_node = list->head;
		current_node != NULL; current_node = current_node->next)
	{
		prom_map_node_t *current_map_node = (prom_map_node_t *)
			current_node->item;
		pll_compare_t result =
			pll_compare(list, current_map_node, temp_map_node);
		if (result != PROM_EQUAL)
			continue;

		if (pll_remove(list, current_node))
			return 1;
		if (pll_remove(keys, (char *)current_map_node->key))
			return 2;
		(*size)--;
		break;
	}
	return prom_map_node_destroy(temp_map_node);
}

int
prom_map_delete(prom_map_t *self, const char *key) {
	PROM_ASSERT(self != NULL);
	if (pthread_rwlock_wrlock(self->rwlock)) {
		PROM_WARN(PROM_PTHREAD_RWLOCK_LOCK_ERROR, NULL);
		return 1;
	}
	prom_map_delete_internal(key, &self->size, &self->max_size, self->keys,
		self->addrs, self->free_value_fn);
	if (pthread_rwlock_unlock(self->rwlock))
		PROM_WARN(PROM_PTHREAD_RWLOCK_UNLOCK_ERROR, NULL);
	return 0;
}

int
prom_map_set_free_value_fn(prom_map_t *self,
	prom_map_node_free_value_fn free_value_fn)
{
	PROM_ASSERT(self != NULL);
	self->free_value_fn = free_value_fn;
	return 0;
}

size_t
prom_map_size(prom_map_t *self) {
	PROM_ASSERT(self != NULL);
	return self->size;
}
