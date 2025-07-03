# include <sys/socket.h>
# include <netdb.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>
# include <fcntl.h>

# include <syslog.h>
# include <errno.h>
# include <string.h>

# include <stdint.h>

# include <sys/wait.h>
# include <signal.h>

int server = -1;
int client = -1;
int fd = -1;

void signal_handler(int sig)
{
    syslog(LOG_INFO, "Caught signal, exiting");
    //if (fd != -1)
        //close(fd);
    if (client != -1)
        close(client);
    if (server != -1)
        close(server);
    unlink("/var/tmp/aesdsocketdata");
}

int open_socket()
{
    const int enable = 1;
    struct addrinfo hints;
    struct addrinfo *res;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        printf("%s\n", strerror(errno));
        return -1;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        printf("%s\n", strerror(errno));
        return -1;
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    int rc = getaddrinfo(NULL, "9000", &hints, &res);
    if (rc != 0) {
        printf("%s\n", gai_strerror(rc));
        return -1;
    }

    rc = bind(fd, res->ai_addr, sizeof(struct sockaddr));
    if (rc != 0) {
        printf("%s\n", strerror(errno));
        freeaddrinfo(res);
        return -1;
    }
    freeaddrinfo(res);
    return fd;
}

int listen_and_accept_con(int fd, char *addr)
{
    struct sockaddr clientAddr;
    socklen_t clientAddrLen;
    int rc = listen(fd, 1);
    if (rc != 0) {
        printf("%s\n", strerror(errno));
        return -1;
    }
    int client = accept(fd, &clientAddr, &clientAddrLen);
    if (client == -1) {
        printf("%s\n", strerror(errno));
        return -1;
    }
    memcpy(addr, clientAddr.sa_data, 14);
    syslog(LOG_INFO, "Accepted connection from %s", clientAddr.sa_data);
    return client;
}

int is_packet_complete(char *buffer, int size)
{
    for (int i = 0; i < size; i++)
        if (buffer[i] == '\n')
            return 1;
    return 0;
}



char *receive_data(int client)
{
    int bufferLength = 1024;
    ssize_t totalReadedByte = 0;
    char *buffer = malloc(sizeof(char) * bufferLength);

    if (!buffer) {
        printf("Out of memory!");
        return NULL;
    }

    memset(buffer, 0, bufferLength);
    ssize_t readedByte = recv(client, buffer, bufferLength, 0);
    totalReadedByte += readedByte;
    int i = 0;
    while (readedByte != 0) {
        if (is_packet_complete(buffer, totalReadedByte) == 1)
            break;

        bufferLength += 1024;
        buffer = realloc(buffer, sizeof(char) * bufferLength);
        if (buffer == NULL) {
            printf("Out of memory!");
            return NULL;
        }
        memset(&(buffer[totalReadedByte]), 0, 1024);
        readedByte = recv(client, &(buffer[totalReadedByte]), 1024, 0);
        totalReadedByte += readedByte;
        i++;
    }
    return buffer;
}

int open_file()
{
    int fd = open("/var/tmp/aesdsocketdata", O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    return fd;
}

char *read_file(int fd)
{
    char *data = NULL;

    off_t fileSize = lseek(fd, 0, SEEK_END);
    if (fileSize == -1) {
        printf("%s", strerror(errno));
        return NULL;
    }

    off_t offset = lseek(fd, 0, SEEK_SET);
    if (offset != 0) {
        printf("%s", strerror(errno));
        return NULL;
    }
    data = malloc(sizeof(char) * fileSize + 1);
    if (!data) {
        printf("Out of memory!");
        return NULL;
    }
    if (read(fd, data, fileSize) == -1) {
        printf("%s", strerror(errno));
        free(data);
        return NULL;
    }
    data[fileSize] = 0;

    return data;
}

int main(int ac, char **av)
{
    struct sigaction action;
    char *data;
    char *fileData;

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = signal_handler;
    if (sigaction(SIGTERM, &action, NULL) != 0) {
        printf("%s: %s", __FUNCTION__, strerror(errno));
        return -1;
    }
    if (sigaction(SIGINT, &action, NULL) != 0) {
        printf("%s: %s", __FUNCTION__, strerror(errno));
        return -1;
    }


    openlog(NULL, 0, LOG_USER);
    server = open_socket();
    if (server == -1) {
        printf("%s: error in open_socket().\n", __FUNCTION__);
        return -1;
    }

    if (ac == 2 && strcmp(av[1], "-d") == 0) {
        printf("Running as a daemon!\n");
        int pid = fork();
        if (pid != 0) { 
            exit(0);
        }
    }

    while (1) {
        char addr[15];
        memset(addr, 0, 15);
        client = listen_and_accept_con(server, addr);
        if (client == -1) {
            printf("%s: error in listen_and_accept_con().\n", __FUNCTION__);
            goto close_socket;
        }

        data = receive_data(client);
        if (!data) {
            printf("%s: error in receive_data().\n", __FUNCTION__);
            goto close_client;
        }

        fd = open_file();
        if (fd == -1) {
            printf("%s: %s", __FUNCTION__, strerror(errno));
            goto free_data;
        }

        if (write(fd, data, strlen(data)) == -1) {
            printf("%s: %s", __FUNCTION__, strerror(errno));
            goto close_file;
        }

        fileData = read_file(fd);
        if (!fileData) {
            printf("%s: error in read_file().\n", __FUNCTION__);
            goto close_file;
        }

        if (send(client, fileData, strlen(fileData), 0) == -1) {
            printf("%s: %s", __FUNCTION__, strerror(errno));
            goto free_fileData;
        }

        syslog(LOG_INFO, "Closed connection from %s", addr);
        free(fileData);
        close(fd);
        free(data);
        close(client);
    }
    close(server);
    return 0;

free_fileData:
    free(fileData);
close_file:
    close(fd);
free_data:
    free(data);
close_client:
    close(client);
close_socket:
    close(server);
    return -1;
}
