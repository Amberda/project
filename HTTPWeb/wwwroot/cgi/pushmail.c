#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mysql.h>

void send_mail(char* qq)
{
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

    //printf("connect success!\n");

    char query[1024];
    sprintf(query,"select qq from student");
    mysql_query(pmysql,query); 
    
    MYSQL_RES* res=mysql_store_result(pmysql);
    int row=mysql_num_rows(res);
    int col = mysql_num_fields(res);
   // MYSQL_FIELD *field = mysql_fetch_fields(res); 
    
    size_t num_field=mysql_field_count(pmysql);//获取结果集中的列数

    MYSQL_ROW fetch=mysql_fetch_row(res);
    int i=0;
    for(;i<row;i++)
    {
        int j=0;
        for(;j<num_field;j++)
        {
            strcpy(qq,fetch[j]);
            strcat(qq,"@qq.com");
            //printf("%s\n",qq);
        }
    }
    mysql_close(pmysql);
}
int main()
{   
    //dup2(2,1);
    /*char buf[1024];//="id=1&name=liu&sex=man&phone=12345678901"; 

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

    char name[32];
    char sex[32];
    char qq[32];

    strtok(buf,"=&");
    strcpy(name,strtok(NULL,"=&"));

    strtok(NULL,"=&");
    strcpy(sex,strtok(NULL,"=&"));

    strtok(NULL,"=&");
    strcpy(qq,strtok(NULL,"=&"));

    strcat(qq,"@qq.com");

    pid_t id=fork();
    if(id==0)                         //子进程去执行消息推送
    { 
        execl("./mail.sh",qq);
        exit(1);        
    }    */
    char qq[32];
    send_mail(qq);

    printf("%s\n",qq);
    char *path="./mail.sh";
    pid_t id=fork();
    if(id==0)                         //子进程去执行消息推送
    { 
        printf("<html>\n");
        printf("<h3>send_mail success! please check out the qqmail you fill in</h3>\n");
        printf("</html>\n");
        int num = execl(path,"./mail.sh",qq,NULL);
        printf("%d\n",num);
        perror("execl");
        exit(1);        
    }   
    
    printf("<h3>see you again</h3>\n");
    return 0;
}
