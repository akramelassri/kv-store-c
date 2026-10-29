#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include "hash.h"
#include "server.h"

#define MAX_LINE 8192

static char *next_word(char **rest)
{
    char *s = *rest;
    while (*s == ' ')
        s++;
    char *start = s;
    while (*s && *s != ' ')
        s++;
    if (*s)
    {
        *s = 0;
        s++;
    }
    *rest = s;
    return start;
}

/* returns int so we can act accordingly with each cmd */
int handle_line(int fd, char *line)
{
    size_t len = strlen(line);
    if (len && line[len - 1] == '\n')
        line[--len] = 0;
    if (len && line[len - 1] == '\r')
        line[--len] = 0;
    if (!len)
        return 0;

    char *rest = line;
    char *cmd = next_word(&rest);

    printf("[DEBUG]: %zu Bytes recieved\n", len);
    printf("[DEBUG]: Received Command: %s\n", cmd);

    if (strcmp(cmd, "SET") == 0)
    {
        char *key = next_word(&rest);
        while (*rest == ' ')
            rest++;
        char *val = rest;
        if (!*key || !*val)
        {
            reply(fd, "ERR usage: SET key value\n");
            return 0;
        }
        if (strlen(key) > 512 || strlen(val) > MAX_LINE - 64)
        {
            reply(fd, "ERR key or value too long\n");
            return 0;
        }
        if (ht_set(key, val) != 0)
        {
            reply(fd, "ERR out of memory\n");
            return 0;
        }
        reply(fd, "OK\n");
    }
    else if (strcmp(cmd, "GET") == 0)
    {
        char *key = next_word(&rest);
        if (!*key)
        {
            reply(fd, "ERR usage: GET key\n");
            return 0;
        }
        struct hentry *e = ht_find(key);
        if (!e)
        {
            reply(fd, "NOT_FOUND\n");
            return 0;
        }
        char out[MAX_LINE + 8];
        snprintf(out, sizeof out, "%s\n", e->val);
        reply(fd, out);
    }
    else if (strcmp(cmd, "DELETE") == 0)
    {
        char *key = next_word(&rest);
        if (!*key)
        {
            reply(fd, "ERR usage: DELETE key\n");
            return 0;
        }
        reply(fd, ht_del(key) ? "DELETED\n" : "NOT_FOUND\n");
    }
    else if (strcmp(cmd, "PING") == 0)
    {
        reply(fd, "PONG\n");
    }
    else if (strcmp(cmd, "QUIT") == 0)
    {
        reply(fd, "BYE\n");
        return 1;
    }
    else
    {
        reply(fd, "ERR unknown command\n");
    }
    return 0;
}
