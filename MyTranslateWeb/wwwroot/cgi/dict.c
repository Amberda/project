#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#define BURSIZE 2048
struct dic
{
    char* key;
    char* content;
    struct dic* next;
};

int open_file(struct dic** p,const char* filename)
{

    FILE* pfile=fopen(filename,"r");
    if(pfile==NULL)
    {
        return 0;
    }
    char buf[1024]={0};
    size_t len=0;
    int count=0;

    *p=(struct dic*)malloc(sizeof(struct dic));
    memset(*p,0x00,sizeof(struct dic));

    struct dic *pd=*p;
    while(!feof(pfile))
    {
        memset(buf,0x00,sizeof(buf));
        fgets(buf,sizeof(buf),pfile);
        len=strlen(buf);
        if(len>0)
        {
            pd->key=(char*)malloc(len);
            memset(pd->key,0x00,len);
            strcpy(pd->key,&buf[1]);
        }
        memset(buf,0x00,sizeof(buf));
        fgets(buf,sizeof(buf),pfile);
        len=strlen(buf);
        if(len>0)
        {
            pd->content=(char*)malloc(len);
            memset(pd->content,0x00,len);
            strcpy(pd->content,&buf[6]);
        }
        pd->next=(struct dic*)malloc(sizeof(struct dic));
        memset(pd->next,0x00,sizeof(pd->next));
        pd=pd->next;
        count++;
    }
    fclose(pfile);
    return count;
}

int find_word(struct dic* p, int size,const char* key,char* content)
{
    const struct dic* pd=p;
    while(pd)
    {
        if(pd->key && pd->content)
        {
            if(strncmp(pd->key,key,strlen(key))==0)
            {
                strcpy(content,pd->content);
                return 1;
            }
        }
        pd=pd->next;
    }
    return 0;
}

void free_dic(struct dic* p,int size)
{
    struct dic* pd=p;
    while(size>0 && pd)
    {
        if(pd->key)
        {
            free(pd->key);
            pd->key=NULL;
        }
        if(pd->content)
        {
            free(pd->content);
            pd->content=NULL;
        }
        struct dic* tmp=pd->next;
        free(pd);
        pd=NULL;
        pd=tmp;
        size--;
    }
}

void urldecode(char *p)  
{  
    register i=0;  
    while(*(p+i))  
    {  
        if ((*p=*(p+i)) == '%')  
        {  
            *p=*(p+i+1) >= 'A' ? ((*(p+i+1) & 0XDF) - 'A') + \
            10 : (*(p+i+1) - '0');  
            *p=(*p) * 16;  
            *p+=*(p+i+2) >= 'A' ? ((*(p+i+2) & 0XDF) - 'A') + \
            10 : (*(p+i+2) - '0');  
            i+=2;  
        }
        else if (*(p+i)=='+')  
        {  
            *p=' ';  
        }  
        p++;  
    }  
    *p='\0';  
} 

int main()
{
    char buf[1024];
    perror("call insert");
 
    if(getenv("METHOD")!=NULL)
    {
        if(strcasecmp("GET",getenv("METHOD"))==0)
        {
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

    char key[1024];
    
    strtok(buf,"=&");
    strcpy(key,strtok(NULL,"&="));
    //printf("%s\n",key);
    urldecode(key);
    //printf("key=%s\n",key);

    struct dic* p=NULL;
    int size=open_file(&p,"dict.txt");

    char content[1024];
    memset(content,0x00,sizeof(content));
  
    if(strncmp(key,"command=exit",12)==0)
    {
        return 0;
    }
    if(find_word(p,size,key,content)==1)
    {
        printf("%s",content);
    }
    else
    {
        printf("not found\n");
    }
    
    free_dic(p,size);

    return 0;
}
