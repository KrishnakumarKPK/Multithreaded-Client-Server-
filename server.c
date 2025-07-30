#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BACKLOG 10
#define MAX_USER 10
#define HSIZE 100

int cur_user_id = 1;
int session_id_ = 1;
int arr[HSIZE];
int addr[HSIZE];
int session[HSIZE];

typedef struct pthread_arg_t {
    int id;
    int new_socket_fd;
    struct sockaddr_in client_address;
} pthread_arg_t;

void *pthread_routine(void *arg);
void signal_handler(int signal_number);

void free_users() {
    arr[0] = 0;
    addr[0] = -1;
    for (int i = 1; i < HSIZE; i++) {
        arr[i] = -1;
        addr[i] = -1;
    }
}
void free_user_no(int n) {
    if (n > HSIZE) {
        return;
    }
    arr[n] = -1;
    addr[n] = -1;
}

int main(int argc, char *argv[]) {
    free_users();
    int port, socket_fd, new_socket_fd;
    struct sockaddr_in address;
    pthread_attr_t pthread_attr;
    pthread_arg_t *pthread_arg;
    pthread_t pthread;
    socklen_t client_address_len;

    port = argc > 1 ? atoi(argv[1]) : 0;
    if (!port) {
        printf("Enter Port: ");
        scanf("%d", &port);
    }

    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof address) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    if (pthread_attr_init(&pthread_attr) != 0) {
        perror("pthread_attr_init");
        exit(1);
    }
    if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) != 0) {
        perror("pthread_attr_setdetachstate");
        exit(1);
    }

    while (1) {
        pthread_arg = (pthread_arg_t *)malloc(sizeof *pthread_arg);
        if (!pthread_arg) {
            perror("malloc");
            continue;
        }

        client_address_len = sizeof pthread_arg->client_address;
        new_socket_fd = accept(socket_fd, (struct sockaddr *)&pthread_arg->client_address, &client_address_len);
        if (new_socket_fd == -1) {
            perror("accept");
            free(pthread_arg);
            continue;
        }

        pthread_arg->new_socket_fd = new_socket_fd;
        addr[cur_user_id] = new_socket_fd;
        pthread_arg->id = cur_user_id++;

        if (pthread_create(&pthread, &pthread_attr, pthread_routine, (void *)pthread_arg) != 0) {
            perror("pthread_create");
            free(pthread_arg);
            continue;
        }
    }

    return 0;
}

void print_users() {
    printf("currently active users are :[");
    for (int i = 1; i < HSIZE; i++) {
        if (arr[i] != -1) {
            printf("%d,", i);
        }
    }
    printf("]\n");
}

void *pthread_routine(void *arg) {
    int n;
    char buffer[256];
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int new_socket_fd = pthread_arg->new_socket_fd;
    struct sockaddr_in client_address = pthread_arg->client_address;
    int id = pthread_arg->id;

    free(arg);

    arr[id] = 0;
    addr[id] = new_socket_fd;
    printf("connected to user %d \n", id);
    print_users();

    while (1) {
        bzero(buffer, 256);
        read(new_socket_fd, buffer, 255);
        if (!bcmp(buffer, "~list", 5)) {
            bzero(buffer, 256);
            strcpy(buffer, "Server:Free_users_are:[");
            int ind = 23;
            for (int i = 1; i < HSIZE; i++) {
                if (arr[i] == 0) {
                    if (i < 10) {
                        buffer[ind++] = '0';
                        buffer[ind++] = (char)('0' + i);
                        buffer[ind++] = ',';
                    } else {
                        buffer[ind++] = (char)('0' + (i / 10));
                        buffer[ind++] = (char)('0' + (i % 10));
                        buffer[ind++] = ',';
                    }
                }
            }
            buffer[ind++] = ']';
            write(new_socket_fd, buffer, 255);
        } else if (!bcmp(buffer, "~connect_to_", 12)) {
            int to = ((int)(buffer[12] - '0')*10 + (int)(buffer[13] - '0'));
            if (arr[to] == -1) {
                char temp[256] = "Server:user_not_exist";
                write(new_socket_fd, temp, 255);
            } else if (arr[to] != 0) {
                char temp[256] = "Server:user_busy";
                write(new_socket_fd, temp, 255);
            } else if (to == id) {
                char temp[256] = "Server:connecting to self is not allowed";
                write(new_socket_fd, temp, 255);
            } else {
                arr[id] = to;
                arr[to] = id;
                char temp[256] = "Server:conected_to_user";
                temp[23] = '_';
                temp[24] = (char)('0' + (to / 10));
                temp[25] = (char)('0' + (to % 10));
                session[id] = session_id_;
                session[to] = session_id_;
                printf("secission created ID:%d between user %d and user %d\n", session_id_++, id, to);
                write(new_socket_fd, temp, 255);
            }
        } else if (buffer[0] != '~') {
            int to = arr[id];
            if (arr[to] == -1) {
                char temp[256] = "Server:user_not_exist";
                write(new_socket_fd, temp, 255);
            } else if (arr[to] == 0) {
                char temp[256] = "Server:user_logged_out_earlier";
                write(new_socket_fd, temp, 255);
            } else {
                int addres_to = addr[to];
                if (addres_to == -1) {
                    char temp[256] = "Server:user_not_exist_/_address_error";
                    write(new_socket_fd, temp, 255);
                } else {
                    char temp[256];
                    temp[0] = (char)('0' + (id / 10));
                    temp[1] = (char)('0' + (id % 10));
                    temp[2] = ':';
                    for (int i = 2; i < 256; i++) temp[i + 1] = buffer[i - 2];
                    write(addres_to, temp, 255);
                }
            }
        } else if (!bcmp(buffer, "~stop", 4)) {
            if (arr[id] == 0) {
                char temp[256] = "Server:you are not logged in any session";
                write(new_socket_fd, temp, 255);
            } else {
                int from = id, to = arr[id];
                arr[from] = 0;
                arr[to] = 0;
                char temp[256] = "Server:logged_out_sucessfully";
                printf("secission ended ID:%d between user %d and user %d\n", session[id], from, to);
                write(new_socket_fd, temp, 255);
            }
        } else if (!bcmp(buffer, "~quit", 4)) {
            if (arr[id] != 0) {
                int from = id, to = arr[id];
                arr[from] = 0;
                arr[to] = 0;
                printf("secission ended ID:%d between user %d and user %d\n", session[id], from, to);
            }
            arr[id] = -1;
            char temp[256] = "Server:disconnected_from_server";
            printf("user %d disconnected\n", id);
            print_users();
            write(new_socket_fd, temp, 255);
            break;
        } else if (!bcmp(buffer, "~my_id", 6)) {
            char temp[256] = "Server:";
            temp[7] = (char)('0' + (id / 10));
            temp[8] = (char)('0' + (id % 10));
            write(new_socket_fd, temp, 255);
        }
    }

    close(new_socket_fd);
    return NULL;
}
