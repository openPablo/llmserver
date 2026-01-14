#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>


int createConnection(char hostname[], char port[]) {
    int cfd;
    struct addrinfo hints = {0};
    struct addrinfo * addresses, * adress;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICSERV;
    int err = getaddrinfo(hostname, port, &hints, &addresses);
    if (err != 0) {
        printf("getaddrinfo error: %s\n", gai_strerror(err));
    }
    for (adress = addresses; adress != NULL;adress=adress->ai_next) {
        if ((cfd = socket(adress->ai_family, adress->ai_socktype,adress->ai_protocol)) == -1) {
            continue;
        }
        if (connect(cfd, adress->ai_addr, adress->ai_addrlen) == 0 ) {
            break;
        }
        close(cfd);
    }
    freeaddrinfo(addresses);
    return cfd;
}
void talk_through_pty(int cfd) {
    char buf[1024];
    ssize_t numRead;
    struct pollfd fds[2] = {
        {STDIN_FILENO, POLLIN, 0},
        {cfd, POLLIN, 0}
    };

    while (poll(fds, 2, -1) > 0) {
        if (fds[0].revents & POLLIN) {
            numRead = read(STDIN_FILENO, buf, sizeof(buf));
            if (numRead <= 0) break;
            write(cfd, buf, numRead);
        }
        if (fds[1].revents & POLLIN) {
            numRead = read(cfd, buf, sizeof(buf));
            if (numRead <= 0) break;
            write(STDOUT_FILENO, buf, numRead);
        }
    }
}


int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("usage: %s <hostname> <port>\n", argv[0]);
    }

    int cfd = createConnection(argv[1], argv[2]);
    talk_through_pty(cfd);

    if (close(cfd) == -1) {
        perror("close");
    }
    return 0;
}