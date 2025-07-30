
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_NAME_LEN_MAX 255

int main(int argc, char *argv[]) {
    char server_name[SERVER_NAME_LEN_MAX + 1] = { 0 };
    int server_port, socket_fd;
    struct hostent *server_host;
    struct sockaddr_in server_address;

    if (argc > 1) {
        strncpy(server_name, argv[1], SERVER_NAME_LEN_MAX);
    } else {
        printf("Enter Server Name: ");
        scanf("%s", server_name);
    }

    server_port = argc > 2 ? atoi(argv[2]) : 0;
    if (!server_port) {
        printf("Enter Port: ");
        scanf("%d", &server_port);
    }

    server_host = gethostbyname(server_name);

    memset(&server_address, 0, sizeof server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
		perror("connect");
        exit(1);
	}

    int n;
    char buffer[256];
    while (1) {
        printf("You:");
        bzero(buffer,256);
        scanf("%s", buffer);

        n = write(socket_fd,buffer,strlen(buffer));

        if (n < 0){
            perror("ERROR while writing to socket");
            exit(1);
        }

        bzero(buffer,256);
        n = read(socket_fd, buffer, 255);

        if (n < 0){
            perror("ERROR while reading from socket");
            exit(1);
        }
        if(!bcmp(buffer, "Server", 6)) {
            printf("%s\n", buffer);
        } else {
            printf("User%s\n", buffer);
        }

        if (!bcmp(buffer, "Server:disconnected_from_server", 32))
            break;
    }

    close(socket_fd);
    return 0;
}