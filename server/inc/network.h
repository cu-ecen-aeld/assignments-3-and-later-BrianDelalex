#ifndef NETWORK_H
# define NETWORK_H

int listen_and_accept_con(int fd, char *addr);
char *receive_data(int client);


#endif//!NETWORK_H