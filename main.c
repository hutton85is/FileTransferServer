#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define PORT "6500"

void writeIP(struct addrinfo *servinfo) {
    struct sockaddr_in *ipv = (struct sockaddr_in *)servinfo->ai_addr;
    struct in_addr *addr = &(ipv->sin_addr);
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(servinfo->ai_family, addr, ipstr, sizeof(ipstr));

    printf("Server IP: %s\nPort no.: %d\n", ipstr, ntohs(ipv->sin_port));
}

int main()
{
    struct addrinfo hints, *servinfo;
    int listenfd, connfd, res;
    struct sockaddr_in server_address, client_address;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;

    if ((res = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(res));
        return 2;
    }

    writeIP(servinfo);

    if ((listenfd = socket(servinfo->ai_family, servinfo->ai_socktype, 0)) == -1) {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        return 3;
    }

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*) 1, sizeof(int));

    freeaddrinfo(servinfo);

    server_address = *(struct sockaddr_in *)servinfo->ai_addr;

    if (bind(listenfd, (struct sockaddr*)&server_address, sizeof(server_address)) != 0) {
        fprintf(stderr, "bind error: %s\n", strerror(errno));
        return 4;
    }

    if (listen(listenfd, 1) != 0) {
        fprintf(stderr, "listen error: %s\n", strerror(errno));
        return 5;
    }


    connfd = accept(listenfd, NULL, NULL);
    if ((connfd == -1)) {
        fprintf(stderr, "accept error: %s\n", strerror(errno));
        return 6;
    }

    printf("Client connected with ip address: %s\n", inet_ntoa(client_address.sin_addr));

    unsigned int n;
    char buffer[1024];
    bzero(buffer, 1024);

    if ((n = recv(connfd, buffer, 1024, 0)) <= 0) {
        printf("nothing sent closing now");
        return 1;
    }

    FILE *fptr;
    fptr = fopen(buffer, "w");
    if (fptr == NULL) {
        fprintf(stderr, "Error in opening file: %s\n", strerror(errno));
        return 3;
    }

    while ((n = recv(connfd, buffer, 1024, 0)) > 0) {
        printf("received: %s\n", buffer);
        fprintf(fptr, "%s", buffer);
        bzero(buffer, 1024);
    }

    fclose(fptr);

    printf("Size is %d\nClosing socket\n", n);

    close(connfd);

    close(listenfd);

    return 0;
}
