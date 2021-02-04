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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// Public
#include "prom_alloc.h"

// Private
#include "prom_assert.h"
#include "prom_log.h"
#include "prom_procfs_i.h"

static int
prom_procfs_ensure_buf_size(prom_procfs_buf_t *self) {
	PROM_ASSERT(self != NULL);
	if (self->allocated >= self->size + 1)
		return 0;
	while (self->allocated < self->size + 1)
		self->allocated <<= 1;
	self->buf = (char *) prom_realloc(self->buf, self->allocated);
	return 0;
}

prom_procfs_buf_t *
prom_procfs_buf_new(const char *path) {
	FILE *f = fopen(path, "r");

	if (f == NULL) {
		char errbuf[100];
		strerror_r(errno, errbuf, 100);
		PROM_WARN("%s", errbuf);
		return NULL;
	}

	unsigned short int initial_size = 32;
	prom_procfs_buf_t *self = prom_malloc(sizeof(prom_procfs_buf_t));
	if (self == NULL) {
		fclose(f);
		return NULL;
	}
	self->buf = prom_malloc(initial_size);
	if (self->buf == NULL)
		goto fail;
	self->size = 0;
	self->index = 0;
	self->allocated = initial_size;

	for (int c = getc(f), i = 0; c != EOF; c = getc(f), i++) {
		if (prom_procfs_ensure_buf_size(self))
			goto fail;
		self->buf[i] = c;
		self->size++;
	}
	if (prom_procfs_ensure_buf_size(self))
		goto fail;

	self->buf[self->size] = '\0';
	self->size++;
	fclose(f);
	return self;

fail:
	fclose(f);
	prom_procfs_buf_destroy(self);
	self = NULL;
	return NULL;
}

int
prom_procfs_buf_destroy(prom_procfs_buf_t *self) {
	if (self == NULL)
		return 0;
	prom_free(self->buf);
	self->buf = NULL;
	prom_free(self);
	return 0;
}
