uint32_t tcp_recieve_uint32_t(int serverfd);
int tcp_send_packet(int serverfd, uint8_t cmd, uint32_t data);
int tcp_connect_to_server(const char *ip_addr, int port);
