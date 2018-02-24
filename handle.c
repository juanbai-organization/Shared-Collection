#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include "handle.h"

//Create listening socket
//0     on error
//1     on success
int open_server(int *sock_server, struct sockaddr_in* servaddr, char *port_in){
        unsigned short port = atoi(port_in);

        if ((*sock_server = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                return 0;
        }
        //Construct local address structure
        memset(servaddr, 0, sizeof(*servaddr));
        servaddr->sin_family = AF_INET;
        servaddr->sin_addr.s_addr = htonl(INADDR_ANY); //any network interface
        servaddr->sin_port = htons (port);
        //bind to local address
        if (bind(*sock_server, (struct sockaddr *) servaddr, sizeof(struct sockaddr_in)) <0){
                return 0;
        }
        //start listening
        if (listen(*sock_server, 5) < 0){
                return 0;
        }
        return 1;
}

//Gets the ip of a socket
void getip(char *ipstr, int sock){
        socklen_t len;
        struct sockaddr_storage addr;
        len = sizeof(addr);
        getpeername(sock, (struct sockaddr*)&addr, &len);
        //resource - beej guide
        //deal with both IPv4 and IPv6
        if (addr.ss_family == AF_INET) {
                struct sockaddr_in *s = (struct sockaddr_in *)&addr;
                inet_ntop(AF_INET, &s->sin_addr, ipstr, INET6_ADDRSTRLEN);
        }
        else {
                struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
                inet_ntop(AF_INET6, &s->sin6_addr, ipstr, INET6_ADDRSTRLEN);
        }
}
