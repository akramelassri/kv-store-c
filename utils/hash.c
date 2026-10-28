#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define NBUCKETS 256

static struct hentry *buckets[NBUCKETS];

// fnv-1a
static unsigned hash_str(const char *s)
{
    unsigned h = 2166136261u;
    while (*s)
    {
        h ^= (unsigned char)*s++;
        h *= 16777619u;
    }
    return h;
}

void ht_init(void)
{
    memset(buckets, 0, sizeof buckets);
}

struct hentry *ht_find(const char *key)
{
    struct hentry *e = buckets[hash_str(key) % NBUCKETS];
    while (e)
    {
        if (strcmp(e->key, key) == 0)
            return e;
        e = e->next;
    }
    return NULL;
}

int ht_set(const char *key, const char *val)
{
    struct hentry *e = ht_find(key);
    if (e)
    {
        char *nv = strdup(val);
        if (!nv)
            return -1;
        free(e->val);
        e->val = nv;
        return 0;
    }

    e = malloc(sizeof *e);
    if (!e)
        return -1;
    e->key = strdup(key);
    e->val = strdup(val);
    if (!e->key || !e->val)
    {
        free(e->key);
        free(e->val);
        free(e);
        return -1;
    }
    unsigned b = hash_str(key) % NBUCKETS;
    e->next = buckets[b];
    buckets[b] = e;
    return 0;
}

int ht_del(const char *key)
{
    unsigned b = hash_str(key) % NBUCKETS;
    struct hentry **pp = &buckets[b];
    while (*pp)
    {
        struct hentry *e = *pp;
        if (strcmp(e->key, key) == 0)
        {
            *pp = e->next;
            free(e->key);
            free(e->val);
            free(e);
            return 1;
        }
        pp = &e->next;
    }
    return 0;
}
