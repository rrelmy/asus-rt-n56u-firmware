/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/***********************************************************************
*
* hash.h
*
* Hash table utilities
*
* Copyright (C) 2002 Roaring Penguin Software Inc.
*
* LIC: GPL
*
***********************************************************************/

#ifndef HASH_H
#define HASH_H

#include <stdlib.h>
/* Fixed-size hash tables for now */
#define HASHTAB_SIZE 67

/* A hash bucket */
typedef struct hash_bucket_t {
    struct hash_bucket_t *next;
    struct hash_bucket_t *prev;
    unsigned int hashval;
} hash_bucket;

/* A hash table */
typedef struct hash_table_t {
    hash_bucket *buckets[HASHTAB_SIZE];
    size_t hash_offset;
    unsigned int (*compute_hash)(void *data);
    int (*compare)(void *item1, void *item2);
    size_t num_entries;
} hash_table;

/* Functions */
void hash_init(hash_table *tab,
	       size_t hash_offset,
	       unsigned int (*compute)(void *data),
	       int (*compare)(void *item1, void *item2));
void hash_insert(hash_table *tab, void *item);
void hash_remove(hash_table *tab, void *item);
void *hash_find(hash_table *tab, void *item);
void *hash_find_next(hash_table *tab, void *item);
size_t hash_num_entries(hash_table *tab);

/* Iteration functions */
void *hash_start(hash_table *tab, void **cursor);
void *hash_next(hash_table *tab, void **cursor);

/* Utility function: hashpjw for strings */
unsigned int hash_pjw(char const *str);

#endif
