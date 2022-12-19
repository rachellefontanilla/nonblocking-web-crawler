
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>


/* 4. HASH TABLE: keep track of new URLs to add to URL queue */
int hash_tables_init(struct hsearch_data **visited_htab);
int hash_table_add(char *key, void *data, struct hsearch_data *htab, char** urls_to_free, int urls_count);
void* hash_table_get(char *key, struct hsearch_data *htab);