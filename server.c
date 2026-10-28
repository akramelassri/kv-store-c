#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include "hash.h"

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

static void reply(int fd, const char *s)
{
    send(fd, s, strlen(s), 0);
}

/* returns int so we can act accordingly with each cmd */
static int handle_line(int fd, char *line)
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

static void serve_client(int fd)
{
    char buf[MAX_LINE];
    size_t len = 0;

    for (;;)
    {
        if (len >= sizeof buf)
        {
            reply(fd, "ERR line too long\n");
            return;
        }
        ssize_t n = recv(fd, buf + len, sizeof buf - len, 0);
        if (n <= 0)
            return;
        len += (size_t)n;

        char *start = buf;
        for (;;)
        {
            char *nl = memchr(start, '\n', len - (size_t)(start - buf));
            if (!nl)
                break;
            *nl = 0;
            if (handle_line(fd, start))
                return;
            start = nl + 1;
        }
        len -= (size_t)(start - buf);
        memmove(buf, start, len);
    }
}

int main(int argc, char **argv)
{
    int port = 7331;
    if (argc > 1)
        port = atoi(argv[1]);

    signal(SIGPIPE, SIG_IGN);

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

    ht_init();

    for (;;)
    {
        int cfd = accept(lfd, NULL, NULL);
        if (cfd < 0)
        {
            perror("accept");
            continue;
        }
        serve_client(cfd);
        close(cfd);
    }
}
