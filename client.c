#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    struct addrinfo hints = {0};
    struct addrinfo * addresses, * adress;
    int cfd;
    ssize_t numread;
    char buf[1024];
    if (argc != 3) {
        printf("usage: %s <hostname> <port>\n", argv[0]);
    }

    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICSERV;

    int err = getaddrinfo(argv[1], argv[2], &hints, &addresses);
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
    while ((numread = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        if (write(cfd, buf, numread) != numread) {
            perror("write");
        }
    }
    if ((numread = read(cfd, buf, sizeof(buf))) > 0) {
        if (write(STDOUT_FILENO, buf, numread) != numread) {
            perror("write");
        }
    }
    if (close(cfd) == -1) {
        perror("close");
    }
    return 0;
}