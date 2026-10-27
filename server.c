#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

/*
    TODO:
    - implement delete
    - values with spaces problem
    - fix cmd limited to size buf
    - switch to hashtable
*/

#define MAX_ENTRIES 100

struct entry
{
    char key[128];
    char val[1024];
};

static struct entry store[MAX_ENTRIES];
static int nentries = 0;

static struct entry *find_entry(const char *key)
{
    for (int i = 0; i < nentries; i++)
        if (strcmp(store[i].key, key) == 0)
            return &store[i];
    return NULL;
}

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

static void reply(int fd, const char *s)
{
    send(fd, s, strlen(s), 0);
}

static void handle_line(int fd, char *line)
{
    size_t len = strlen(line);
    if (len && line[len - 1] == '\n')
        line[--len] = 0;
    if (len && line[len - 1] == '\r')
        line[--len] = 0;
    if (!len)
        return;

    char *rest = line;
    char *cmd = next_word(&rest);

    if (strcmp(cmd, "SET") == 0)
    {
        char *key = next_word(&rest);
        while (*rest == ' ')
            rest++;
        char *val = rest;
        if (!*key || !*val)
        {
            reply(fd, "ERR usage: SET key value\n");
            return;
        }
        if (strlen(key) >= 128 || strlen(val) >= 1024)
        {
            reply(fd, "ERR key or value too long\n");
            return;
        }
        struct entry *e = find_entry(key);
        if (!e)
        {
            if (nentries == MAX_ENTRIES)
            {
                reply(fd, "ERR store full\n");
                return;
            }
            e = &store[nentries++];
            strcpy(e->key, key);
        }
        strcpy(e->val, val);
        reply(fd, "OK\n");
    }
    else if (strcmp(cmd, "GET") == 0)
    {
        char *key = next_word(&rest);
        if (!*key)
        {
            reply(fd, "ERR usage: GET key\n");
            return;
        }
        struct entry *e = find_entry(key);
        if (!e)
        {
            reply(fd, "NOT_FOUND\n");
            return;
        }
        char out[1100];
        snprintf(out, sizeof out, "%s\n", e->val);
        reply(fd, out);
    }
    else if (strcmp(cmd, "DELETE") == 0)
    {
        char *key = next_word(&rest);
        if (!*key)
        {
            reply(fd, "ERR usage: DELETE key\n");
            return;
        }
        for (int i = 0; i < nentries; i++)
        {
            if (strcmp(store[i].key, key) == 0)
            {
                store[i] = store[nentries - 1];
                nentries--;
                reply(fd, "DELETED\n");
                return;
            }
        }
        reply(fd, "NOT_FOUND\n");
    }
    else if (strcmp(cmd, "PING") == 0)
    {
        reply(fd, "PONG\n");
    }
    else if (strcmp(cmd, "QUIT") == 0)
    {
        reply(fd, "BYE\n");
        exit(0);
    }
    else
    {
        reply(fd, "ERR unknown command\n");
    }
}

int main(int argc, char **argv)
{
    int port = 7331;
    if (argc > 1)
        port = atoi(argv[1]);

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0)
    {
        perror("socket");
        return 1;
    }

    int one = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }
    listen(lfd, 8);

    printf("listening on %d\n", port);

    char buf[4096];
    for (;;)
    {
        int cfd = accept(lfd, NULL, NULL);
        if (cfd < 0)
        {
            perror("accept");
            continue;
        }

        ssize_t n;
        while ((n = recv(cfd, buf, sizeof(buf) - 1, 0)) > 0)
        {
            // cmd limited to buff size
            buf[n] = '\0';
            handle_line(cfd, buf);
        }
        close(cfd);
    }
}
