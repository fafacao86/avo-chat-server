#ifndef SERVER_PROTOCOL_SOCKET_BASE_H
#define SERVER_PROTOCOL_SOCKET_BASE_H
void get_peer_info(int connfd, char *addr, int addr_size,int *port);
int connect_to_client(int listen_fd, int* is_heartbeat_fd);
int bind_localhost_port();
void set_sockopt(int listen_fd);
void close_client_connection(int heartbeat_fd);
void close_on_SIGUSR1(int sig);
#endif
