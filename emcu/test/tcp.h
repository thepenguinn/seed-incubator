uint32_t tcp_recieve_uint32_t(void);
int tcp_send_packet(uint8_t cmd, uint32_t data);
int tcp_connect_to_server(const char *ip_addr, int port);
char *tcp_get_ip_addr_buf(void);
