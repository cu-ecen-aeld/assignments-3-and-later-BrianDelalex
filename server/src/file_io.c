# include "file_io.h"

# include <unistd.h>
# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>

# include <errno.h>
# include <string.h>

int open_file()
{
    int fd = open(OUTPUT_PATH, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    return fd;
}

char *read_file(int fd)
{
    size_t buffer_size = 1024;
    size_t data_size = 0;
    char *buffer = malloc(sizeof(char) * buffer_size);
    char *data = NULL;

    memset(buffer, 0, buffer_size);

    int rc = read(fd, buffer, buffer_size);
    while (rc > 0) {
        data = realloc(data, sizeof(char) * (data_size + rc + 1));
        memcpy(&(data[data_size]), buffer, rc);
        data[data_size + rc] = 0;
        data_size += rc;
        memset(buffer, 0, buffer_size);
        rc = read(fd, buffer, buffer_size);
    }

    if (buffer)
        free(buffer);

    return data;
}


// char *read_file(int fd)
// {
//     char *data = NULL;

//     off_t fileSize = lseek(fd, 0, SEEK_END);
//     if (fileSize == -1) {
//         printf("%s", strerror(errno));
//         return NULL;
//     }

//     off_t offset = lseek(fd, 0, SEEK_SET);
//     if (offset != 0) {
//         printf("%s", strerror(errno));
//         return NULL;
//     }
//     data = malloc(sizeof(char) * fileSize + 1);
//     if (!data) {
//         printf("Out of memory!");
//         return NULL;
//     }
//     if (read(fd, data, fileSize) == -1) {
//         printf("%s", strerror(errno));
//         free(data);
//         return NULL;
//     }
//     data[fileSize] = 0;

//     return data;
// }