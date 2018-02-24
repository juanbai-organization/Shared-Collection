#ifndef __HANDLE_H__
#define __HANDLE_H__
void getip(char *ipstr, int sock);
int open_server(int *sock_server, struct sockaddr_in* servaddr, char *port_in);
#endif


