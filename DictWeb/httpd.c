#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define MAX 1024

void Usage(char arg[])
{
    printf("Usage:\n\t %s port\n",arg);
}

int startup(int port)
{
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        perror("sock");
        exit(2);
    }

    int opt=1;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in local;
    local.sin_family=AF_INET;
    local.sin_addr.s_addr=htonl(INADDR_ANY);
    local.sin_port=htons(port);

    if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
    {
        perror("bind");
        exit(3);
    }

    if(listen(sock,5)<0)
    {
        perror("listen");
        exit(4);
    }
    return sock;
}

static int get_line(int sock,char line[],int size)
{
    char c='A';
    int len=0;
    while(len<size-1 && c!='\n')
    {
        int s=recv(sock,&c,1,0);
        if(s>0)
        {
            if(c=='\r')
            {
                int tmp=recv(sock,&c,1,MSG_PEEK);
                if(tmp>0)
                {
                    if(c=='\n')
                    {
                        recv(sock,&c,1,0);
                    }
                    else
                    {
                        c='\n';
                    }
                }
            }
            line[len++]=c;
        }
        else
        {
            break;
        }
    }
    line[len]='\0';
    return len;
}

void echo_404(int sock)
{
    printf("not found\n");
    int fd=open("wwwroot/404.html",O_RDONLY);
    if(fd<0)
    {
        perror("open");
    }

    const char* msg="HTTP/1.0 200 OK\r\n";
    send(sock,msg,strlen(msg),0);
    const char* req_msg="Content-Type:text/html;charset=ISO-8859-1\r\n";
    send(sock,req_msg,strlen(req_msg),0);
    const char* null_line="\r\n";
    send(sock,null_line,strlen(null_line),0);

    struct stat st;
    fstat(fd,&st);
    if(sendfile(sock,fd,NULL,st.st_size)<0)
    {
        perror("sendfile");
    }

    close(fd); 
}

void echo_string(int errorcode,int sock)
{
    switch(errorcode)
    {
        case 404:
        echo_404(sock);
        break;
        case 403:
        break;
        case 503:
        break;
    }
}

static int echo_www(int sock,char path[],int size)
{
    int fd=open(path,O_RDONLY);
    if(fd<0)
    {
        perror("open");
        return 404;
    }

    const char* msg="HTTP1.0 200 OK\r\n";
    send(sock,msg,strlen(msg),0);
    const char* req_msg="Content-Type:text/html;charset=ISO-8859-1\r\n";
    send(sock,req_msg,strlen(req_msg),0);
    const char* null_line="\r\n";
    send(sock,null_line,strlen(null_line),0);

    if(sendfile(sock,fd,NULL,size)<0)
    {
        perror("sendfile");
        return 404;
    }
    close(fd);
    return 200;
}

static void clear_head(int sock)
{
    char buf[MAX];
    int ret=0;
    do
    {
        ret=get_line(sock,buf,sizeof(buf));
    }while(ret>0&&strcmp(buf,"\n"));
}

static int exec_cgi(int sock,char method[],char path[],char* query_string)
{
    char method_env[MAX];
    char query_string_env[MAX];
    char content_len_env[MAX/8];
    char line[MAX];
    int content_len=0;
    int ret=0;
    if(strcasecmp(method,"GET")==0)
    {
        clear_head(sock);
    }
    else
    { 
        do
        {
            ret = get_line(sock,line,sizeof(line));
            if(ret>0 && strncasecmp(line,"Content-Length: ",16)==0)
            {
                content_len=atoi(line+16);//得到POST方法参数的长度
            }
        }while(ret>0 && strcmp(line,"\n"));

        if(content_len==-1)
        {
            return 404;
        }
    }

    //创建一个子进程来执行path指向的程序
    //父进程要给子进程写入数据，子进程要把自己的输出结果返回给父进程
    //利用管道让父子进程进行通信
    
    int input[2];
    int output[2];

    pipe(input);//站在子进程的角度
    pipe(output);

    pid_t id=fork();
    if(id<0)
    {
        return 404;
    }
    else if(id==0)//child  利用环境变量将方法参数传递给子进程
    {
        //子进程要从input中读取数据，关闭写端
        //子进程通过output写，关闭读端
        close(input[1]);
        close(output[0]);

        sprintf(method_env,"METHOD=%s",method);
        putenv(method_env);
        if(strcasecmp(method,"GET")==0)
        {
            sprintf(query_string_env,"QUERY_STRING=%s",query_string);
            putenv(query_string_env);
            printf("%s\n",getenv("QUERY_STRING"));
        }
        else
        { 
            sprintf(content_len_env,"CONTENT_LEN=%d",content_len);
            putenv(content_len_env);
        }
        //将文件描述符进行重定向 标准输入重定向到input[0],标准输出重定向到output[1]
        dup2(input[0],0);
        dup2(output[1],1);
        //printf("----%s\n",path);
        execl(path,path,NULL);
        exit(1);
    }
    else
    {
        close(input[0]);
        close(output[1]);

        int i=0;
        char c='A';
        if(strcasecmp(method,"POST")==0)
        {
            for(;i<content_len;i++)
            {
                recv(sock,&c,1,0);
                write(input[1],&c,1);
            }
        }
        const char* msg="HTTP/1.0 200 OK\r\n";
        send(sock,msg,strlen(msg),0);
        const char* req_msg="Content-Type:text/html;charset=ISO-8859-1\r\n";
        send(sock,req_msg,strlen(req_msg),0);
        const char* null_line="\r\n";
        send(sock,null_line,strlen(null_line),0);


       /* char buf[content_len];
        c='\0';
        if(strcasecmp(method,"POST")==0)
        {
            recv(sock,buf,content_len,0);
            write(input[1],buf,content_len);
        }*/

        while(read(output[0],&c,1)>0)
        {
            send(sock,&c,1,0);
        }

        waitpid(id,NULL,0);
        close(input[1]);
        close(output[0]);
    }

}

void *thread_handler(void* arg)
{
    int sock=(int)arg;

    int errorcode=0;
    int cgi=0;
    char line[MAX];
    char method[MAX/32];
    char url[MAX];
    char path[MAX];
    char* query_string=NULL;

    if(get_line(sock,line,sizeof(line))<=0)
    {
        errorcode=404;
        goto end;
    }
    printf("%s\n",line);

    int i=0;
    int j=0;
    while(!isspace(line[i]))
    {
        if(i<sizeof(line)&&j<sizeof(method)-1)
        {
            method[j]=line[i];
            i++;
            j++;
        }
    }
    method[j]=0;//拿到了方法
    if(strcasecmp(method,"GET")==0)//GET方法
    {
        
    }
    else if(strcasecmp(method,"POST")==0)//POST方法
    {
        cgi=1;
    }
    else//不是GET方法也不是POST方法
    {
        errorcode=404;
        goto end;
    }
    while(isspace(line[i])&&i<sizeof(line))
    {
        i++;
    }
    j=0;
    while(j<sizeof(url)-1 && i<sizeof(line) && !isspace(line[i]))
    {
        url[j]=line[i];
        i++;
        j++;
    }
    url[j]=0;
    printf("method:%s,url:%s\n",method,url);

    query_string=url;
    while(*query_string!='\0')
    {
        if(*query_string=='?')
        {
            *query_string=0;
            ++query_string;//找到了GET方法的参数
            cgi=1;
            break;
        }
        ++query_string;
    }

    sprintf(path,"wwwroot%s",url);
    if(path[strlen(path)-1]=='/')
    {
        strcat(path,"index.html");
    }
    printf("%s\n",path);
    
    struct stat st;
    if(stat(path,&st)<0)
    {
        errorcode=404;
        goto end;
    }
    else
    {
        if(S_ISDIR(st.st_mode))
        {
            strcat(path,"/index.html");
        }
        else if((st.st_mode&S_IXUSR)||(st.st_mode&S_IXGRP)||(st.st_mode&S_IXOTH))
        {
            //拥有可执行权限，进入cgi模式
            cgi=1;
        }
        else
        {}

        if(cgi==1)
        {
            printf("path:%s\n",path);
            exec_cgi(sock,method,path,query_string);
        }
        else
        {
            clear_head(sock);
            printf("method : %s, url : %s ,path : %s,cgi : %d,query_string: %s\n",method,url,path,cgi,query_string);
            errorcode =  echo_www(sock,path,st.st_size);
        }
    }
end:
    if(errorcode!=200)
    {
        echo_string(errorcode,sock);
    }
    clear_head(sock);
    close(sock);
}


int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        Usage(argv[0]);
        return 1;
    }

    int listen_sock=startup(atoi(argv[1]));

    for(;;)
    {
        struct sockaddr_in client;
        socklen_t len=sizeof(client); 
        int newsock=accept(listen_sock,(struct sockaddr*)&client,&len);
        if(newsock<0)
        {
            perror("accept");
            continue;
        }
        pthread_t tid;
        pthread_create(&tid,NULL,thread_handler,(void*)newsock);
        pthread_detach(tid);
    }
    close(listen_sock);
    return 0;
}
