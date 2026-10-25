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

struct entry {
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

static void handle_line(int fd, char *line)
{
    size_t len = strlen(line);
    if (len && line[len - 1] == '\n') line[--len] = 0;
    if (len && line[len - 1] == '\r') line[--len] = 0;
    if (!len) return;

    char *cmd = strtok(line, " ");
    if (!cmd) return;

    if (strcmp(cmd, "SET") == 0) {
        char *key = strtok(NULL, " ");
        char *val = strtok(NULL, " ");
        if (!key || !val) {
            send(fd, "ERR need key and value\n", 22, 0);
            return;
        }
        struct entry *e = find_entry(key);
        if (!e) {
            if (nentries == MAX_ENTRIES) {
                send(fd, "ERR store full\n", 15, 0);
                return;
            }
            e = &store[nentries++];
            snprintf(e->key, sizeof e->key, "%s", key);
        }
        snprintf(e->val, sizeof e->val, "%s", val);
        send(fd, "OK\n", 3, 0);

    } else if (strcmp(cmd, "GET") == 0) {
        char *key = strtok(NULL, " ");
        if (!key) {
            send(fd, "ERR need key\n", 13, 0);
            return;
        }
        struct entry *e = find_entry(key);
        if (!e) {
            send(fd, "NOT_FOUND\n", 10, 0);
            return;
        }
        char out[1100];
        int n = snprintf(out, sizeof out, "%s\n", e->val);
        send(fd, out, n, 0);

    } else {
        send(fd, "ERR unknown command\n", 20, 0);
    }
}

int main(int argc, char **argv)
{
    int port = 7331;
    if (argc > 1) port = atoi(argv[1]);

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) {
        perror("socket");
        return 1;
    }

    int one = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    listen(lfd, 8);

    printf("listening on %d\n", port);

    char buf[4096];
    for (;;) {
        int cfd = accept(lfd, NULL, NULL);
        if (cfd < 0) {
            perror("accept");
            continue;
        }

        ssize_t n;
        while ((n = recv(cfd, buf, sizeof(buf) - 1, 0)) > 0) {
            // cmd limited to buff size
            buf[n] = '\0';
            handle_line(cfd, buf);
        }
        close(cfd);
    }
}
