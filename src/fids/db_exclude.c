/*
 * Copyright (C) 2014-2025 Firejail Authors
 *
 * This file is part of firejail project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include"fids.h"

typedef struct db_exclude_t {
	struct db_exclude_t *next;
	char *fname;
	int len;
} DB_EXCLUDE;
static DB_EXCLUDE *database = NULL;

void db_exclude_add(const char *fname) {
	assert(fname);

	DB_EXCLUDE *ptr = malloc(sizeof(DB_EXCLUDE));
	if (!ptr)
		errExit("malloc");

	ptr->fname = strdup(fname);
	if (!ptr->fname)
		errExit("strdup");
	ptr->len = strlen(fname);
	ptr->next = database;
	database = ptr;
}

int db_exclude_check(const char *fname) {
	assert(fname);

	DB_EXCLUDE *ptr = database;
	while (ptr != NULL) {
		if (strncmp(fname, ptr->fname, ptr->len) == 0)
			return 1;
		ptr = ptr->next;
	}

	return 0;
}
