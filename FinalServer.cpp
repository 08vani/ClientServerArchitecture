#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>
#include <linux/limits.h>
#include <sys/times.h>
#include <dirent.h>
#include<iostream>
#include<algorithm>
#include<vector>
#include <bits/stdc++.h>
#include <cstring>
#include <fcntl.h>
 
using std :: string;
using std:: to_string;

using std::cerr;
using std::endl;
#include <fstream>
using std::ofstream;
using std::fstream;
using std::ios;
#include <cstdlib>

typedef long long int num;

struct procInfo
{
	long long int id;
	char *  name;
	long long int utime;
	long long int stime;
    long long int key;

    procInfo(long long int i, char * n, long long int u, long long int s) {
      this -> id = i;
      this -> name = n;
      this->  utime= u;
      this-> stime= s;
      this-> key= u+s;
   }

};

int inde;

std::vector<procInfo> Proc_Array;


num pid;
char tcomm[PATH_MAX];
char state;


num ppid;
num pgid;
num sid;
num tty_nr;
num tty_pgrp;

num flags;
num min_flt;
num cmin_flt;
num maj_flt;
num cmaj_flt;
num utime;
num stimev;



//long tickspersec;

int data;
int read_bytes;
char *c= (char *) calloc(1024, sizeof(char));



void error (const char * msg)
{
	perror(msg);
	exit(1);
}


void getAllInfo(char * p, int i)
{
	//tickspersec = sysconf(_SC_CLK_TCK);
  	data = -1;

  	chdir("/proc");

  	if(chdir(p)==0)
    { 
        data = open("stat", O_RDONLY); 
        read_bytes= read(data,c, 1024 );
    }
  	if(data<0)
  	{
  		return ;
  	}

    sscanf(c, " %lld %s %c %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",&pid, tcomm, &state, &ppid, &pgid, &sid, &tty_nr, &tty_pgrp, &flags, &min_flt, &cmin_flt, &maj_flt,&cmaj_flt, &utime, &stimev);

    Proc_Array.push_back(procInfo(pid, tcomm, utime,stimev));

    close(data);

}

void getFile()
{
	struct dirent *de;  


    
    DIR *dr = opendir("/proc");
  
    if (dr == NULL)  
    {
        printf("Could not open current directory" );
        //return 0;
    }

    int i=0;
  
    while ((de = readdir(dr)) != NULL)
    {
        getAllInfo(de->d_name,i);
        i=i+1;
        
    }
  
    closedir(dr);

    sort(Proc_Array.begin(), Proc_Array.end(), [](const procInfo& p1, const procInfo& p2) {
      return p1.key > p2.key;
   });

    i=0;


    ofstream ServerInfo("/home/vanisikka/Desktop/ServerProcInfo.txt",std::ios::out );
    for(auto it = Proc_Array.begin(); it != Proc_Array.end(); it++) {

        if(i==5)
        {
            break;
        }

        i=i+1;

        string data;

        //data= to_string(it -> id)+" "+string(it -> name)+" "+to_string((((double)it -> utime) / tickspersec))+" "+to_string((((double)it -> stime) / tickspersec))+ " ";

        data= to_string(it -> id)+" "+string(it -> name)+" "+to_string(it -> utime) +" "+to_string(it -> stime)+ " "+to_string(it -> key)+" ";


        ServerInfo << data;

        
   }

   string  data="*";
   ServerInfo << data;

   ServerInfo.close();
}


void * client_handler (void * shared_socket)
{
	int client_socket= *((int *) shared_socket);

	free(shared_socket);

	char buffer[1024];

	char* client_request;
	read(client_socket, buffer, 1024);

	client_request=buffer;

	printf("[+] Recived Client's request \n");

	printf(" CLIENT REQUESTS: %s \n", buffer);

	getFile();

	string str= "recv"+to_string(inde);

	strcpy(buffer, str.c_str());
	

	write(client_socket, buffer, 1024);

	printf("[+] Sent the filename to the client \n");

	FILE * f;

	int words=0;

	char c;

	f= fopen("/home/vanisikka/Desktop/ServerProcInfo.txt", "r");


	if (f == NULL) 
	{
    perror("[-]Error in reading file.");
    exit(1);
    }

	while((c=getc(f))!=EOF)
	{
		fscanf(f, "%s", buffer);
		if(isspace(c) || c=='\t')
		{
			words++;
		}
	}

	write(client_socket, &words, sizeof(int));

	rewind(f);

	printf("[+] Sent the number of words in the file to the client \n");

	char ch;

	for( int j=0;j<25;j++)
	{
		fscanf(f, "%s", buffer);
		write(client_socket, buffer, 1024);
		ch= fgetc(f);
	}

	printf("[+] The file has been successfully transferred \n");

	printf("[+] Successfully recived client's top CPU consuming process \n");

	printf("Client Top Process: \n");


	long long int u,s, CpuTime;
	
	read(client_socket, buffer, 1024);

	printf(" PID: %s\n", buffer);

	read(client_socket, buffer, 1024);

	printf(" NAME: %s\n", buffer);

	read(client_socket, buffer, 1024);

	u=atoll(buffer);

	printf(" UTIME: %s\n", buffer);

	read(client_socket, buffer, 1024);

	s=atoll(buffer);

	printf(" STIME: %s\n", buffer);

	CpuTime=u+s;

	printf(" CPU USAGE: %lld\n", CpuTime);
	

	


	close(client_socket);
	fclose(f);





}

int main ()
{
	int s_socket, c_socket, addr_size, portno;

	sockaddr_in s_addr, c_addr;

	s_socket=socket(AF_INET, SOCK_STREAM,0);
	portno= 8989;

	if(s_socket<0)
	{
		error("[-] Error in Creating Socket \n");
	}

	//intializing the address struct

	s_addr.sin_family= AF_INET;
	s_addr.sin_addr.s_addr= INADDR_ANY;
	s_addr.sin_port= htons(portno);

	printf("[+] The Server Socket is created successfully.\n");

	if(bind(s_socket, (sockaddr *) & s_addr, sizeof(s_addr))<0)
	{
		error("[-] Error in binding the socket.");
	}

	printf("[+] The socket is successfully binded \n");

	listen(s_socket, 5);

	printf("[+] Server is listening \n");

	while(true)
	{

		addr_size= sizeof(sockaddr_in);

		c_socket= accept(s_socket, (sockaddr *) &c_addr, (socklen_t *) &addr_size);

		if(c_socket<0)
		{
			error("[-] Error in accepting client connection\n");
		}

		printf("[+] Server Accepted Client Connection \n");

		pthread_t t;

		inde=inde+1;

		int *client_sock= (int *)malloc(sizeof(int));
		*client_sock= c_socket;

		pthread_create(&t, NULL, client_handler, client_sock);

		pthread_join(t,0);

		//pthread_exit(NULL);
	}

	

	return 0;

}

