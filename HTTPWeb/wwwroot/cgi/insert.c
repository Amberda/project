#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>

void insert_data(char *name,char *sex,char *qq)
{
    perror("start");
    MYSQL* pmysql=mysql_init(NULL);
    if(pmysql==NULL)
    {
        printf("init failed\n");
        return;
    }
    pmysql=mysql_real_connect(pmysql,"127.0.0.1","root","","class_25",3306,NULL,0);
    if(pmysql==NULL)
    {
        printf("connect failed\n");
        return;
    }

    printf("connect success!\n");

    char query[1024];
    sprintf(query,"insert into student values(\"%s\",\"%s\",\"%s\")",name,sex,qq);
    printf("%s\n",query);

    mysql_query(pmysql,query); 
    perror("inset finally");
    mysql_close(pmysql);
}



int main()
{
    char buf[1024];//="id=1&name=liu&sex=man&phone=12345678901"; 
    perror("call insert");
    //dup2(2,1);

    if(getenv("METHOD")!=NULL)
    {
        printf("%s\n",getenv("QUERY_STRING"));
        if(strcasecmp("GET",getenv("METHOD"))==0)
        {
            printf("%s\n",getenv("QUERY_STRING"));
            strcpy(buf,getenv("QUERY_STRING"));
        }
       else if(strcasecmp("POST",getenv("METHOD"))==0)
        {
            int content_len=atoi(getenv("CONTENT_LEN"));
            int i=0;
            char c;
            for(;i<content_len;i++)
            {
                read(0,&c,1);
                buf[i]=c;
            }
            buf[i]=0;
        }
    }
    perror("get method string");
    printf("%s\n",buf);

    char name[32];
    char sex[32];
    char qq[32];

    perror("id");

    strtok(buf,"=&");
    strcpy(name,strtok(NULL,"=&"));

    strtok(NULL,"=&");
    strcpy(sex,strtok(NULL,"=&"));

    strtok(NULL,"=&");
    strcpy(qq,strtok(NULL,"=&"));

    perror("tiquzifuchuan");
    insert_data(name,sex,qq);

    /*char *path="./mail.sh";
    strcat(qq,"@qq.com");
    pid_t id=fork();
    if(id==0)                         //子进程去执行消息推送
    { 
        printf("<html>\n");
        printf("<h3>send_mail success! please check out the qqmail you fill in</h3>\n");
        printf("</html>\n");
        execl(path,"./mail.sh",qq,NULL);
        exit(1);        
    }   
    
    printf("<h3>see you again</h3>\n");*/

    perror("finally");
    return 0;
}

