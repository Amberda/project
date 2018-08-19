#include "socket.c"
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <rpc/types.h>

#define METHOD_GET 0
#define METHOD_POST 1
#define METHOD_HEAD 2
#define METHOD_OPTIONS 3
#define METHOD_TRACE 4
#define REQUEST_SIZE 2048
#define POSTDATA_SIZE 1024

//最终结果
int speed=0;
int failed=0;
int bytes=0;

//默认设置
volatile int timerexpired=0;//判断压测时间是否已经到达设置的时间
int method=METHOD_GET;//GET方法
int clients=1;//只模拟一个客户端
int force=0;//等待响应
int force_reload=0;//失败时重新请求
int proxyport=80;//访问端口
char* proxyhost=NULL;//代理服务器
int benchtime=30;//模拟请求时间
int http10=1;

//time.h中声明了MAXHOSTNAMELEN 64
char host[MAXHOSTNAMELEN];  //网络地址
char request[REQUEST_SIZE]; //要发送的HTTP请求

//POST方法,添加数据
char* postdataall=NULL;
int postdataallline=0;
char* requestall=NULL;
int requestallsize=0;
char postdata[POSTDATA_SIZE]={0};
char postdatalen[5]={0};

static const struct option long_options[]=
{
    {"force",no_argument,&force,1},
    {"reload",no_argument,&force_reload,1},
    {"time",required_argument,NULL,'t'},
    {"felp",no_argument,NULL,'?'},
    {"http10",no_argument,NULL,'1'},
    {"http11",no_argument,NULL,'2'},
    {"get",no_argument,&method,METHOD_GET},
    {"post",no_argument,&method,METHOD_POST},
    {"head",no_argument,&method,METHOD_HEAD},
    {"options",no_argument,&method,METHOD_OPTIONS},
    {"trace",no_argument,&method,METHOD_TRACE},
    {"file",required_argument,NULL,'F'},
    {"proxy",required_argument,NULL,'p'},
    {"clients",required_argument,NULL,'c'},
    {NULL,0,NULL,0}
};


static void usage()
{
    fprintf(stderr,
           "my-webbench [option]...URL\n"
           "  -f|--force               Don't wait for replr from server\n "
           "  -r|--reload              Send reload request - Pragma:no-cache\n"
           "  -t|--time <sec>          Run benchmark for <sec> second.Default 30\n"
           "  -p|--proxy <server:port> Use proxy server for request\n"
           "  -c|--clients <n>         Run <n> HTTP clients at once.Default one\n"
           "  -1|--http10              Use HTTP/1.0 protocol\n"
           "  -2|--http11              Use HTTP/1.1 protocol\n"
           "  --get                    Use GET request method\n"
           "  --head                   Use HEAD request method\n"
           "  --options                Use OPTIONS request method\n"
           "  --trace                  Use TRACE request method\n"
           "  -P|--post                Use POST request method\n"
           "  -F|--file                Use POST request method from file\n"
           "  -?|-h|--help             This information\n"
    );
}

int get_postdatafromfile(char* filename)
{
    int fd=open(filename,O_RDONLY);
    if(fd<0)
    {
        perror("open file failed");
        return 3;
    }
    struct stat st;
    if(fstat(fd,&st)||st.st_size==0)//文件无效
    {
        fprintf(stderr,"this filr is invalid");
        return 2;
    }

    //将文件映射进内存
    void* ptr=mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
    if(ptr==NULL)
        return 3;
    char* ch=(char*)ptr;

    int maxsize=0;
    int line=0;
    int offset=0;
    char* tmp=NULL;
    //统计文件的行数
    for(;offset<st.st_size;offset++)
    {
        ch++;
        if(offset>0&&(*ch)=='\n')
        {
            if(tmp==NULL)
                maxsize=offset;
            else if(maxsize<ch-tmp)
                maxsize=ch-tmp;
            tmp=ch;
            line++;
        }
    }
    if(line<1||line>1000)
        return 2;

    int count=0;
    char* data=calloc(POSTDATA_SIZE,line);
    if(data==NULL)
    {
        perror("calloc failed.");
        return 3;
    }
    ch=(char*)ptr;
    tmp=NULL;
    //拷贝文件中的数据
    for(offset=0;offset<st.st_size;offset++)
    {
        ch++;
        if(offset>0&&(*ch)=='\n')
        {
            if(tmp==NULL)
            {
                if(offset>1)
                {
                    memcpy(data,ch-offset,offset);
                    *(data+offset-1)='\0';
                    count++;
                }
            }
            else
            {
                if((ch-tmp)>1)
                {
                    memcpy(data+count*POSTDATA_SIZE,tmp+1,ch-tmp);
                    *(data+count*POSTDATA_SIZE+(ch-tmp)-1)='\0';
                    count++;
                }
            }
            tmp=ch;
        }
    }
    munmap(ptr,st.st_size);//取消映射
    close(fd);
    
    postdataall=data;
    postdataallline=count;
    free(data);
    data=NULL;
    return 0;
}

void build_HttpRequest(const char* url)
{
    memset(host,0x00,MAXHOSTNAMELEN);
    memset(request,0x00,REQUEST_SIZE);

    if(force_reload&&proxyhost!=NULL&&http10<1)
        http10=1;
    if(method==METHOD_HEAD&&http10<1)
        http10=1;
    if(method==METHOD_OPTIONS&&http10<2)//options和trace方法只适合HTTP/1.1
        http10=2;
    if(method==METHOD_TRACE&&http10<2)
        http10=2;
    //填写method方法
    switch(method)
    {
    default:
    case METHOD_GET:
        strcpy(request,"GET");
        break;
    case METHOD_POST:
        strcpy(request,"POST");
        break;
    case METHOD_HEAD:
        strcpy(request,"HEAD");
        break;
    case METHOD_OPTIONS:
        strcpy(request,"options");
        break;
    case METHOD_TRACE:
        strcpy(request,"trace");
        break;
    }
    strcat(request," ");
    
    //对传进来的url进行合法性判断
    if(strstr(url,"://")==NULL)
    {
        fprintf(stderr,"\n%s is not a valid URL!\n",url);
        exit(2);
    }
    if(strlen(url)>1500)
    {
        fprintf(stderr,"URL is too long!\n");
        exit(2);
    }
    if(proxyhost==NULL)
    {
        if(strncasecmp("http://",url,7)!=0)
        {
            fprintf(stderr,"\nOnly HTTP protocol is supported,set --proxy for others!\n");
            exit(2);
        }
    }
    //找到主机名开始的地方
    int i=strstr(url,"://")-url+3;
    char tmp[10];
    //必须以/结尾
    if(strchr(url+i,'/')==NULL)
    {
        fprintf(stderr,"\nInvalid URL syntax - hostname don't ends with '/'!\n");
        exit(2);
    }
    if(proxyhost==NULL)
    {
        //从主机名得到端口号
        if(strchr(url+i,':')!=NULL&&strchr(url+i,':')<strchr(url+i,'/'))
        {
            strncpy(host,url+i,strchr(url+i,':')-url-i);
            memset(tmp,0x00,10);
            strncpy(tmp,strchr(url+i,':')+1,strchr(url+i,'/')-strchr(url+i,':')-1);
            //得到端口号 在tmp中
            proxyport=atoi(tmp);
            if(proxyport==0)
                proxyport=80;
        }
        else
            strncpy(host,url+i,strcspn(url+i,"/"));

        strcat(request+strlen(request),url+i+strcspn(url+i,"/"));
    }
    else
        strcat(request,url);

    //加请求头首行
    if(http10==1)
        strcat(request," HTTP/1.0");
    if(http10==2)
        strcat(request," HTTP/1.1");

    strcat(request,"\r\n");

    //添加请求首部
    //客户端程序信息
    if(http10>0)
        strcat(request,"User-Agent: my-webbench\r\n");

    //请求资源所在的服务器
    if(proxyhost==NULL&&http10>0)
    {
        strcat(request,"Host: ");
        strcat(request,host);
        strcat(request,"\r\n");
    }
    //报文指令  表示客户端不接受缓存
    if(force_reload&&proxyhost!=NULL)
        strcat(request,"Pragma: no=cache\r\n");

    //控制不再转发给代理的首部字段，还可以管理持久连接
    if(http10>0)
        strcat(request,"Connection: close\r\n");//断开连接

    //如果时post方法,特殊处理
    if(method==METHOD_POST)
    {
        if(postdataall==NULL)
        {
            strcat(request,"Accept: */*\r\n");
            strcat(request,"Content-Length: ");
            strcat(request,postdatalen);
            strcat(request,"\r\nContent-Type: application/x-www-form-urlencoded\r\n");
            strcat(request,"\r\n");//加空行
            strcat(request,postdata);//正文数据
        }
        else//从文件中获取参数
        {
            requestall=calloc(POSTDATA_SIZE,postdataallline);
            if(requestall==NULL)
            {
                perror("calloc failed.");
                return 3;
            }
            int i=0;
            for(;i<postdataallline;i++)
            {
                snprintf(requestall+i*POSTDATA_SIZE,POSTDATA_SIZE,
                        "%s""Accept: */*\r\n""Content-Length: ""%d"
                        "\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n""%s",
                        request,strlen(postdataall+i*POSTDATA_SIZE),
                        postdataall+i*POSTDATA_SIZE);
            }
            requestallsize=i;
            if(i<clients)
                clients=i;
        }
    }
    else if(http10>0)
        strcat(request,"\r\n");
}

static int bench()
{
    int sock;
    //建立网络连接：先测试一次，看服务器是否能连接成功
    if(proxyhost==NULL)
        sock=my_Socket(host,proxyport);
    else
        sock=my_Socket(proxyhost,proxyport);
    if(sock<0)
    {
        fprintf(stderr,"\nCOnnect to server failed.\n");
        return 1;
    }
    close(sock);//测试成功，关闭

    int mypipe[2];//管道
    if(pipe(mypipe))
    {
        perror("create pipe failed!");
        return 3;
    }
    //派生子进程,有几个用户就创建几个
    int i=0;
    pid_t pid;
    for(;i<clients;i++)
    {
        pid=fork();
        if(pid<=0)
        {
            sleep(1);
            break;    //让子进程立即跳出循环，防止子进程继续fork
        }
    }
    if(pid<0)
    {
        fprintf(stderr,"fork child failed\n");
        perror("fork");
        return 3;
    }
    if(pid==0)//子进程
    {
        if(proxyhost==NULL)
            benchcore(host,proxyport,request);
        else
            benchcore(proxyhost,proxyport,request);

        //打开管道写：连接请求状态的信息
        FILE* f=fdopen(mypipe[1],"w");//将文件描述符转为文件指针
        if(f==NULL)
        {
            perror("open pipe for write failed");
            return 3;
        }
        fprintf(f,"%d %d %d\n",speed,failed,bytes);
        fclose(f);
        return 0;
    }
    else//父进程
    {
        FILE* f=fdopen(mypipe[0],"r");
        if(f==NULL)
        {
            perror("open pipe for reading failed");
            return 3;
        }
        //设置f的缓冲区为无缓冲
        setvbuf(f,NULL,_IONBF,0);
        int j;
        int k;
        while(1)
        {
            pid=fscanf(f,"%d %d %d",&i,&j,&k);
            if(pid<2)
            {
                fprintf(stderr,"Some of our childrens died\n");
                break;
            }
            speed+=i;
            failed+=j;
            bytes+=k;
            if(--clients==0)
                break;
        }
        close(f);
        printf("\nSpeed=%d pages/min,%d bytes/sec.\nRequests: %d susceed, %d failed.\n",\
               (int)((speed+failed)/(benchtime/60.0f)),\
               (int)(bytes/(float)benchtime),speed,failed);
    }
    return i;
}

static void* alarm_handler(int signal)
{
    timerexpired=1;
}

void benchcore(const char* host,const int port,const char* req)
{
    struct sigaction sa;
    sa.sa_handler=alarm_handler;
    sa.sa_flags=0;
    if(sigaction(SIGALRM,&sa,NULL))
        exit(3);

    alarm(benchtime);
    int rlen=strlen(req);
next:
    while(1)
    {
        if(timerexpired==1)
        {
            if(failed>0)
                failed--;
            return;
        }
        int sock=my_Socket(host,port);
        if(sock<0)  //连接失败
        {
            failed++;
            continue;
        }
        if(rlen!=write(sock,req,rlen))
        {
            failed++;//写失败
            close(sock);
            continue;
        }
        int i;
        char buf[1500];
        if(force==0)//不读取服务器回复
        {
            while(1)
            {
                if(timerexpired)
                    break;
                i=read(sock,buf,1500);
                if(i<0)
                {
                    failed++; //读失败
                    close(sock);
                    goto next;
                }
                else
                {
                    if(i==0) //读完退出
                        break;
                    else
                        bytes+=i; //统计服务器回复字节数
                }
            }
        }
        if(close(sock))
        {
            failed++; //关闭失败
            continue;
        }
        speed++;  //成功连接
    }
}

int main(int argc,char* argv[])
{
    if(argc==1)
    {
        usage();
        return 2;
    }
    int opt=0;
    int option_index=0;
    char* tmp=NULL;

    while((opt=getopt_long(argc,argv,"12frt:p:P:F:c:?h",long_options,&option_index))!=EOF)
    {
        switch(opt)
        {
        case 0:
            break;
        case 'f':
            force=1;
            break;
        case 'r':
            force_reload=1;
            break;
        case '1':
            http10=1;
            break;
        case '2':
            http10=2;
            break;
        case 't':
            benchtime=atoi(optarg);//指向当前选项参数的指针
            break;
        case 'p':
            tmp=strrchr(optarg,':');
            proxyhost=optarg;
            if(tmp==NULL)
                break;
            if(tmp==optarg)
            {
                fprintf(stderr,"Error in option --proxy %s:Missing hostname\n",optarg);
                return 2;
            }
            if(tmp==optarg+strlen(optarg)-1)
            {
                fprintf(stderr,"Error in option --proxy %s:Port number is missing\n",optarg);
                return 2;
            }
            *tmp='\0';
            proxyport=atoi(tmp+1);
            break;
        case 'P':
            snprintf(postdata,sizeof(postdata),"%s",optarg);
            snprintf(postdatalen,sizeof(postdatalen),"%ld",strlen(postdata));
            method=METHOD_POST;
            break;
        case 'F':
            if(get_postdatafromfile(optarg))
                return 3;
            method=METHOD_POST;
            break;
        case ':':
        case 'h':
        case '?':
            usage();
            return 2;
        case 'c':
            clients=atoi(optarg);
            break;
        }
    }

    //optind 再次调用getopt时的下一个argv指针的索引
    if(optind==argc)
    {
        fprintf(stderr,"my-webbench:Missing URL!\n");
        usage();
        return 2;
    }
    if(clients==0)
        clients=1;
    if(benchtime==0)
        benchtime=60;

    fprintf(stderr,"Start the pressure measurement!\n");

    //构造HTTP请求到request数组
    build_HttpRequest(argv[optind]);

    printf("Testing: ");
    switch(method)
    {
    case METHOD_GET:
    default:
        printf("GET");
        break;
    case METHOD_POST:
        printf("POST");
        break;
    case METHOD_HEAD:
        printf("HEAD");
        break;
    case METHOD_OPTIONS:
        printf("OPTIONS");
        break;
    case METHOD_TRACE:
        printf("TRACE");
        break;
    }
    printf(" %s\n",argv[optind]);

    if(http10==2)
        printf(" (using HTTP/1.1)\n");

    printf("%d clients",clients);
    printf(",running %d sec",benchtime);

    if(force)
        printf(",early socket close");
    if(proxyhost!=NULL)
        printf(",via proxy server %s:%d",proxyhost,proxyport);
    if(force_reload)
        printf(",forcing reload");
    printf("\n");

    return bench();
}
