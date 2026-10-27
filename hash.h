#ifndef HASH_H
#define HASH_H

struct hentry {
    char *key;
    char *val;
    struct hentry *next;
};

void ht_init(void);
struct hentry *ht_find(const char *key);
int ht_set(const char *key, const char *val); /* copies both, 0 on success */
int ht_del(const char *key);                  /* 1 if something was deleted */

#endif
