#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "./utils/server.h"
#include "./utils/hash.h"

int main(int argc, char **argv)
{
    int port = 7331;

    if (argc > 1)
        port = atoi(argv[1]);

    signal(SIGPIPE, SIG_IGN);

    ht_init();

    int lfd = server_listen(port);
    if (lfd < 0)
        return 1;

    printf("listening on %d\n", port);

    server_run(lfd);

    close(lfd);
    return 0;
}