#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#define max 100 
#define BUFFER_SIZE 512

int Socket(int,int,int);
void Bind(int,const struct sockaddr *sa,socklen_t salen);
void Listen(int,int);
int Accept(int,struct sockaddr*,socklen_t*);
void handleAccept(int);
void handleHttp(int);
int getRequest(int);

string to_String(int n)  
{  
    int m=n;  
    int i=0,j=0;  
    char s[max];  
    char ss[max];  
    while(m>0)  
    {  
        s[i++]= m%10 + '0';  
        m/=10;  
    }  
    s[i]='\0';  
  
    i=i-1;  
    while(i>=0)  
    {  
        ss[j++]=s[i--];  
    }  
    ss[j]='\0';  
  
    return ss;  
} 

int main(int argc,char *argv[])
{

    const int port = 15240;
    int listenfd = Socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    Bind(listenfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    Listen(listenfd,5);

    while(true)
    {
        handleAccept(listenfd);
    }

    return 0;
}

int Socket(int family,int type,int protocol)
{
    int n;
    if((n=socket(family,type,protocol))<0)
    {
        printf("socket error\n");
        return -1;
    }
    return n;
}
void Bind(int fd,const struct sockaddr *sa,socklen_t salen)
{
    if(bind(fd,sa,salen)<0)
    {
        printf("bind error\n");
        exit(-1);
    }
}
void Listen(int fd,int backlog)
{
    char *ptr;

    if((ptr=getenv("LISTENQ"))!=NULL)
    {
        backlog = atoi(ptr);
    }
    if(listen(fd,backlog)<0)
    {
        printf("Listen error\n");
        return ;
    }
}
int Accept(int fd,struct sockaddr *sa,socklen_t *salenptr)
{
    int n ;
again:
    if((n = accept(fd,sa,salenptr))<0)
    {
#ifdef EPROTO
        if(errno == EPROTO || errno == ECONNABORTED)
#else
        if(errno == ECONNABORTED)
#endif
            goto again;
        else
        {
            printf("accept error\n");
            return -1;
        }
    }
    return n;
}
void handleAccept(int listenfd)
{
    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int connfd = Accept(listenfd,(sockaddr*)&clientAddr,&clientLen);
    handleHttp(connfd);
    close(connfd);
}
void handleHttp(int connfd)
{
    if(getRequest(connfd)<0)
    {
        perror("http request get error\n");
        exit(-1);
    }
}
int getRequest(int socket)
{
    int msgLen = 0;
    char  buffer[BUFFER_SIZE];
    memset(buffer,'\0',BUFFER_SIZE);
    if((msgLen = recv(socket,buffer,BUFFER_SIZE,0)) == -1)
    {
        printf("Error handling incoming request\n");
        return -1;
    }
     stringstream ss;
     ss << buffer;
     cout << "ss:"<<ss<<endl;
     string method;
     ss >> method;
     cout << "method:"<<method<<endl;
     string fristname;
     ss >> fristname;
     cout <<"fristname:"<<fristname<<endl;
     string uri;
     ss >> uri;
     cout << "uri:"<<uri<<endl;
     string version;
     ss >> version;
     cout << "version:"<<version<<endl;

		ifstream myfile("index.html");
		string line;
		string content;
		if(myfile.is_open())
		{
			while(getline(myfile,line))
			{
				content +=line;
			}		
			myfile.close();
		}
    string statusCode("200 OK");
    string contentType("text/html");
    /*string content("<html><head><title>my simple httpserver</title></head><h1>hello zx world</h1><form action=\"/demo/demo_form.asp\">Name:<br><input type=\"text\" name=\"name\" value=\"Mickey\"><br>\
            age:<br><input type=\"text\" name=\"age\" value=\"Mouse\"><br><br><input type=\"submit\" value=\"Submit\"></body></html>");
            */
    string contentSize(to_String(content.size()));
    string head("\r\nHTTP/1.1 ");
    string ContentType("\r\nContent-Type: ");
    string ServerHead("\r\nServer: localhost");
    string ContentLength("\r\nContent-Length: ");
    string Date("\r\nDate: ");
    string Newline("\r\n");
    time_t rawtime;
    time(&rawtime);
    string message;
    message+=head;
    message+=statusCode;
    message+=ContentType;
    message+=contentType;
    message+=ServerHead;
    message+=ContentLength;
    message+=contentSize;
    message+=Date;
    message+=(string)ctime(&rawtime);
    message+=Newline;

    int messageLength=message.size();
    int n;
    n=send(socket,message.c_str(),messageLength,0);
    n=send(socket,content.c_str(),content.size(),0);   
    
    return n;
}
