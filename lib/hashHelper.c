#include "hashHelper.h"

/* 4. HASH TABLE: keep track of new URLs to add to URL queue */
    // | key | data |
    // | URL |  1   |
int hash_tables_init(struct hsearch_data **visited_htab){
    *visited_htab = calloc(1, sizeof(struct hsearch_data));
    int hash_res = hcreate_r(500, *visited_htab);
    if (hash_res == 0){
        perror("hcreate_r error");
        return -1;
    } else {
        return 0;
    }
}

// returns 0 on success else -1
int hash_table_add(char *key, void *data, struct hsearch_data *htab, char** urls_to_free, int urls_count){
    ENTRY e, *e_ret;
    char* url = malloc(sizeof(char)* (strlen(key) + 1));
    strcpy(url, key); // must allocate memory to make hashmap work
    e.key = url;
    e.data = (void*)1;
    int res = hsearch_r(e, ENTER, &e_ret, htab);

    if (res == 0) {
        // key unsuccessfully entered into hashtable
        // fprintf(stderr, "entry failed\n");
        free(url);
        return -1;
    } else {
        // key successfully entered into hashtable
        urls_to_free[urls_count] = url; // must free these urls later
        urls_count++;
        return 0;
    }
}

// returns key's data on success else NULL
void* hash_table_get(char *key, struct hsearch_data *htab){
    ENTRY e, *e_ret;
    e.key = key;
    int res = hsearch_r(e, FIND, &e_ret, htab);
    if (res == 0){
        // couldn't find key in hashtable
        return NULL;
    } else {
        // found key in hashtable
        return e_ret->data;
    }
    
}

