#ifndef FILE_IO_H
# define FILE_IO_H

int open_file(void);
char *read_file(int fd);

#ifdef USE_AESD_CHAR_DEVICE
    #define OUTPUT_PATH "/dev/aesdchar"
#else
    #define OUTPUT_PATH "/var/tmp/aesdsocketdata"
#endif//!USE_AESD_CHAR_DEVICE

#endif//!FILE_IO_H