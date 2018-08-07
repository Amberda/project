#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <map>
#include <sys/types.h>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
using namespace std;

class Dict
{
public:
    int open_file(const char* filename)
    {
        ifstream file(filename);
        if (!file.is_open())
        {
            perror("fstream");
            return 1;                    
        }
        size_t len = 0;
        int count = 0;
        char key[1024] = { 0  };
        char con[1024] = { 0  };

        while (!file.eof())
        {
            memset(key, 0x00, sizeof(key));
            file.getline(key, 1024);                        
            memset(con, 0x00, sizeof(con));
            file.getline(con, 1024); 
            std::pair< std::map< string,string>::iterator,bool > ret;
            ret=_dict.insert(pair<string, string>(key + 1, con + 6)); 
            if(ret.second)
            {
                ;
            }
            else
                cout<<"fail "<<key<<endl;
        }
        file.close();
        return count;    
    }

    int find_word(string key, string& content)
    {
        cout<<key<<endl;
        map<string, string>::iterator it = _dict.find(key);
        if (_dict.end() != it)
        {
            content = it->second;
            return 0;        
        }
        else
            return 1;    
    }

private:
    map<string,string> _dict;
};

void urldecode(char *p)  
{  
    register int i=0;  
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
    dup2(2,1);
    /*char buf[1024];
    int index = 0;
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
            char c;
            for(index=0;index<content_len;index++)
            {
                read(0,&c,1);
                buf[index]=c;
            }
            buf[index]=0;
        }
    }
    perror("get method string");

    char key1[1024];
    
    strtok(buf,"=&");
    strcpy(key1,strtok(NULL,"&="));
    urldecode(key1);*/

    Dict d;
    d.open_file("dict.txt");

    string content;
    string key="abc";

    if (key=="command=exit")
    {
        return 0;    
    }
   if (d.find_word(key, content) == 0)
    {
        cout << "result: " << content << endl;    
    }
   else
    {
        cout << "not found" << endl;    
    }
    return 0;
}
