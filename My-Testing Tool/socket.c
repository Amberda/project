#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int my_Socket(const char* host,int port)
{
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        return sock;
    }
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    unsigned long inaddr=inet_addr(host);
    if(inaddr!=INADDR_NONE)
    {
        memcpy(&addr.sin_addr,&inaddr,sizeof(inaddr));
    }
    else
    {
        struct hostent* my_host=gethostbyname(host);
        if(my_host==NULL)
        {
            return -1;
        }
        memcpy(&addr.sin_addr,my_host->h_addr,my_host->h_length);
    }

    if(connect(sock,(struct sockaddr*)&addr,sizeof(addr))<0)
    {
        return -1;
    }
    return sock;
}

