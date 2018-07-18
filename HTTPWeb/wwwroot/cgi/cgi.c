#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <mysql.h>

void insert_data(int id,char *name,char *sex,char *phone)
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

    printf("connect success!\n");

    char query[1024];
    sprintf(query,"insert into student values(%d,\"%s\",\"%s\",\"%s\")",id,name,sex,phone);
    printf("%s\n",query);

    mysql_query(pmysql,query); 

    mysql_close(pmysql);
}
void cal(char* data)
{
    int d1;
    int d2;
    sscanf(data,"first_data=%d&second_data=%d",&d1,&d2);

    printf("<html>\n");
    printf("<h3>%d+%d=%d</h3>\n",d1,d2,d1+d2);
    printf("<h3>%d-%d=%d</h3>\n",d1,d2,d1-d2);
    printf("<h3>%ded=%d</h3>\n",d1,d2,d1*d2);
    printf("<h3>%d/%d=%d</h3>\n",d1,d2,d1/d2);
    printf("<h3>%d%%%d=%d</h3>\n",d1,d2,d1%d2);
    printf("<html>\n");
}
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

    printf("connect success!\n");

    char query[1024];
    sprintf(query,"select * from student");
    mysql_query(pmysql,query); 
    
    MYSQL_RES* res=mysql_store_result(pmysql);
    int row=mysql_num_rows(res);
    int col = mysql_num_fields(res);
    MYSQL_FIELD *field = mysql_fetch_fields(res); 
    int i = 0; 
    for(; i < col; i++)
    {    
        printf("%s\t ",field[i].name);  
    } 
    printf("\n");


    printf("<table border=\"1\">");
    for(i=0;i<row;i++)
    {
        MYSQL_ROW rowdata=mysql_fetch_row(res);
        int j=0;
        printf("<tr>");
        for(;j<col;j++)
        {
            printf("<td>%s</td>",rowdata[i]);
        }
        printf("</tr>");
    }%

    strtok(NULL,"=&");
    strcpy(phone,strtok(NULL,"=&"));

    printf("%d %s %s %s",id,name,sex,phone);

    //select_data();
    insert_data(id,name,sex,phone);
    //select_data();
    //cal(buf);
    
    return 0;
}
