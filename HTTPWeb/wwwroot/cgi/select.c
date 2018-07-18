#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>

void select_data()
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
    sprintf(query,"select * from student");
    mysql_query(pmysql,query); 
    
    MYSQL_RES* res=mysql_store_result(pmysql);
    int row=mysql_num_rows(res);
    int col = mysql_num_fields(res);
    MYSQL_FIELD *field = mysql_fetch_fields(res); 
    int i = 0; 
    printf("<table border=\"1\">");
    for(; i < col; i++)
    {
        printf("<td>%s</td> ",field[i].name); 
    } 

    for(i=0;i<row;i++)
    {
        MYSQL_ROW rowdata=mysql_fetch_row(res);
        int j=0;
        printf("<tr>");
        for(;j<col;j++)
        {
            printf("<td>%s</td>",rowdata[j]);
        }
        printf("</tr>");
    }
    printf("</table>");

    mysql_close(pmysql);
}



int main()
{
    /*perror("call select");
    char buf[1024];
    //dup2(2,1);
    if(getenv("METHOD")!=NULL)
    {
        printf("fangfa:%s\n",getenv("METHOD"));
        if(strcasecmp("GET",getenv("METHOD"))==0)
        {
            strcpy(buf,getenv("QUERY_STRING"));
            printf("canshu:%s\n",getenv("QUERY_STRING"));
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
    printf("buf:%s\n",buf);
    
    perror("method finish");*/
    /*int id;
    char name[32];
    char sex[32];
    char phone[32];
    //sscanf(buf,"id=%d&name=%s&sex=%s&phone=%s",id,name,sex,phone);

    strtok(buf,"=&");
    id=atoi(strtok(NULL,"=&"));

    strtok(NULL,"=&");
    strcpy(name,strtok(NULL,"=&"));

    strtok(NULL,"=&");
    strcpy(sex,strtok(NULL,"=&"));

    strtok(NULL,"=&");
    strcpy(phone,strtok(NULL,"=&"));*/

    

    select_data();
    return 0;
}

