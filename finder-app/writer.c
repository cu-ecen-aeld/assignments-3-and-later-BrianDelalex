# include <syslog.h>
# include <fcntl.h>
# include <string.h>
# include <unistd.h>

int main(int ac, char **av)
{
    openlog(NULL, 0, LOG_USER);
    if (ac != 3) {
        syslog(LOG_ERR, "writer need 2 arguments %d was specified.", ac);
        return 1;
    }

    int fd = open(av[1], O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        syslog(LOG_ERR, "unable to open file %s", av[1]);
        return 1;
    }
    syslog(LOG_DEBUG, "Writing %s to %s", av[2], av[1]);
    if (write(fd, av[2], strlen(av[2])) == -1) {
        syslog(LOG_ERR, "Error while writting.");
        return 1;
    } 
    return 0;
}