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

static void die(const char *s) {perror(s); exit(1);}

struct entry{
    char name[16];
    char url[256];
    int claps;
};

int main (int argc, char **argv){
        
        //Check arguments
        if (argc != 3) {
                fprintf(stderr, "usage: %s <server_port> <web_root>\n", argv[0]);
                exit(1);
        }

        //Create listening socket
        int sock_lis;
        struct sockaddr_in servaddr_lis;
        if ((open_server(&sock_lis, &servaddr_lis,  argv[1])==0)){
                die("Failed to create server");
        }

        //Client Socket
        int clntsock;
        socklen_t clntlen;
        struct sockaddr_in clntaddr;

        //Giving service to client side
        for (;;){
                clntlen = (sizeof(clntaddr));
                if((clntsock = accept(sock_lis, (struct sockaddr *) &clntaddr, &clntlen))<0){
                        close(sock_lis);
                        die("accept failed");
                }
                int result = handle(clntsock);
                op(result, clntsock);
                close(clntsock);
        }
}

int handle(int sock){
        FILE *client = fdopen(sock, "rb");
        if (client == NULL) {return -2;} //Check if connection interrupted
        char buf[4096];
        char *check_client;
        check_client = fgets(buf, sizeof(buf), client); //Get the request
        if (check_client == NULL) {return -2;} //Check if connection interrupted

        char buf_bu[strlen(buf)+1];
        if (strlen(buf)>0){
            if (buf[strlen(buf)-1]=='\n') {buf[strlen(buf)-1]='\0';}}
        if (strlen(buf)>0){
            if (buf[strlen(buf)-1]=='\r') {buf[strlen(buf)-1]='\0';}}
        strcpy(buf_bu,buf);
        printf("%s\n",buf_bu);
        char file[1000]; //File to open

        //      -2            Sock Failed
        //      -1    501     Not Implemented
        //      0     400     bad request
        //      1     200     index
        //      2     404     submit form
        //      3             css
        //      4             submit
        char *get;
        char *path;
        char *http;
        FILE *tosend;
        
        char *token_separators = "\t \r\n";
        get = strtok(buf, token_separators);
        path = strtok(NULL, token_separators);
        http = strtok(NULL, token_separators);
        if ((get==NULL) || (path==NULL) || (http==NULL)){ return -1;}

        if(strcmp(get, "GET")!=0) { return -1;}
        if(http == NULL) { return 0;;}
        else if((strcmp(http, "HTTP/1.0")) && (strcmp(http, "HTTP/1.1"))){
            return -1;}
        if (strlen(path) == 0) { return 0;}
        if (path[0]!='/') { return 0;}

        if (strcmp(path, "/index.html") ==0){ return 1;}
        if (strcmp(path, "/submit.html") ==0){return 2;}
        if (strcmp(path, "/style.css") ==0){return 3;}
        //printf("%s\n",path);
        if (strstr( path, "/?name=")==path){
            addToDB( path, sock);
            return 4;}

        if (strstr(path, "/../")!=NULL) {return 0;}
        if ((strlen(path)>=3) && (strcmp(path +(strlen(path)-3),"/..")==0)){
                                return 0;}
        return 0;
}


// -2 connection interrupted
int op(int result, int sock){
        if (result == -2){
                printf("Connection Interrupted.\n");
        }
        if (result == -1){
                printf("Not Implemented.\n");
        }
        if (result == 0){
                printf("Bad Request.\n");
        }
        if (result == 1){
            FILE *tosend = fopen("./index.html", "rb");
            if (tosend==NULL){ printf("ERROR - Can't handle index.html\n");}
            else{
                char ms[] = "HTTP/1.0 200 OK\r\n\r\n";
                char res[] = "200 OK";
                if (send(sock, ms, strlen(ms), 0)!=strlen(ms)){ return op(-2, sock);}
                char tmpbuf[1000];
                tmpbuf[999]='\n';
                int i=0;
                while ((((tmpbuf == fgets(tmpbuf, 999, tosend))))&&(i<50)){
                    if (send(sock, tmpbuf, strlen(tmpbuf), 0)!=strlen(tmpbuf)){ 
                        return op(-2, sock);
                    }
                    i++; 
                }
                sendLeader(sock);
                while ((((tmpbuf == fgets(tmpbuf, 999, tosend))))){
                    if (send(sock, tmpbuf, strlen(tmpbuf), 0)!=strlen(tmpbuf)){
                        return op(-2, sock);
                    }
                }
                fclose(tosend);
            }
        }
        if (result == 2){
            FILE *tosend = fopen("./submit.html", "rb");
            if (tosend==NULL){ printf("ERROR - Can't handle index.html\n");}
            else{
                char ms[] = "HTTP/1.0 200 OK\r\n\r\n";
                char res[] = "200 OK";
                if (send(sock, ms, strlen(ms), 0)!=strlen(ms)){ return op(-2, sock);}
                char tmpbuf[1000];
                tmpbuf[999]='\n';
                while (((tmpbuf == fgets(tmpbuf, 999,  tosend)))){
                    if (send(sock, tmpbuf, strlen(tmpbuf), 0)!=strlen(tmpbuf)){ return op(-2, sock);}
                }
                fclose(tosend);
            }
        }
        if (result == 3){
            FILE *tosend = fopen("./style.css", "rb");
            if (tosend==NULL){ printf("ERROR - Can't handle index.html\n");}
            else{
                char ms[] = "HTTP/1.0 200 OK\r\n\r\n";
                char res[] = "200 OK";
                if (send(sock, ms, strlen(ms), 0)!=strlen(ms)){ return op(-2, sock);}
                char tmpbuf[1000];
                tmpbuf[999]='\n';
                while (tmpbuf == fgets(tmpbuf, 999, tosend)){
                    if (send(sock, tmpbuf, strlen(tmpbuf), 0)!=strlen(tmpbuf)){ return op(-2, sock);}
                }
                fclose(tosend);
            }
        }
        if (result == 4){
            op(1, sock);
        }


        return 0;
}

int addToDB( char *path, int sock){
        struct entry *ent = malloc(sizeof(*ent));
        path+=7;
        char *string[2];
        printf("%s\n", path);
        string[0] = strtok(path,"&");
        string[1] = strtok(NULL,"")+4;
        strcpy((char *)&ent->name,string[0]);
        strcpy((char *)&ent->url, string[1]);
        uint32_t init = 0;
        uint32_t init_n = htonl(init);
        strcpy((char *)&ent->claps, (char *)&init_n);

        FILE *db = fopen("./DB.txt", "ab");
        int i = fwrite(ent, sizeof(*ent), 1, db);
        printf("%d\n", i);

        

        printf("%s-%s\n",ent->name, ent->url);
        fclose(db);
}

int sendLeader(int sock){
    struct entry ent;
    FILE *db = fopen("./DB.txt", "rb");
    if (db==NULL) {printf("ERROR - Can't handle DB.txt\n");}
    else{
        char s[] = "<table>\r\n<tr>\r\n<th>Contributer</th>\r\n<th>Link</th>\r\n<th>Claps</th></tr>";
        send(sock, s,sizeof(s),0);
        int num;
        char buf[1000];
        while (fread(&ent, sizeof(ent), 1, db)>0){ 
            printf("reading\n");
            strcpy(buf, "<tr><td>");
            strcat(buf, ent.name);
            strcat(buf, "</td><td>");
            strcat(buf, ent.url);
            strcat(buf, "</td><td>");
            char tmp[10];
            snprintf(tmp, 10, "%d", ent.claps);
            strcat(buf, tmp);
            strcat(buf,"</td></tr>");
            printf("%s\n", buf);
            send(sock, buf, strlen(buf), 0);
        }
        char t[] = "</table>";
        send(sock, t, sizeof(t), 0);
    }
}
