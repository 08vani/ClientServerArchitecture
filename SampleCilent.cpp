
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
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


    ofstream ServerInfo("/home/vanisikka/Desktop/ClientProcInfo.txt",std::ios::out );
    for(auto it = Proc_Array.begin(); it != Proc_Array.end(); it++) {

        if(i==5)
        {
            break;
        }

        i=i+1;

        string data;

       // data= to_string(it -> id)+" "+string(it -> name)+" "+to_string((((double)it -> utime) / tickspersec))+" "+to_string((((double)it -> stime) / tickspersec))+ " ";

        data= to_string(it -> id)+" "+string(it -> name)+" "+to_string(((it -> utime)))+" "+to_string(((it -> stime)))+ " ";

        ServerInfo << data;

        
   }

   string  data="*";
   ServerInfo << data;

   ServerInfo.close();
}


int main ()
{
	int sockfd, portno, n;
	char *ip = "127.0.0.1";
	portno= 8989;

	struct sockaddr_in serv_addr;

	struct hostent *server;// variable used to store information about the given host used to store host name and host ip address

	char buffer[1024];


	sockfd= socket(AF_INET, SOCK_STREAM,0);

	if(sockfd<0)
		error("[-]Error in opening socket");

	server= gethostbyname(ip); // get data from the host ip address

	if(server==NULL)
	{
		fprintf(stderr, "Error, no such host");
	}

	bzero((char *)& serv_addr, sizeof(serv_addr));

	serv_addr.sin_family= AF_INET;
	serv_addr.sin_port= htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	printf("[+] Socket Created successfully\n");


	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
	{
		error("[-]Connection failed");
	}

	printf("[+] Client is now connected to server\n");

	bzero(buffer, 1024);

	string str="Please send the file name and the Server's Top 5 CPU consuming tasks";

	strcpy(buffer, str.c_str());

	write(sockfd, buffer, 1024);


	printf("[+] Request Sent to the Server \n");


	FILE * fp;

	int ch=0;

	read(sockfd, buffer, 1024);

	//printf(" * %s \n*", buffer);

	printf("[+] File name Recived from the server\n");

	fp= fopen (buffer, "w");

	if (fp == NULL) {
    printf("[-]Error in reading file.");
    exit(1);
  }


	int words;

	read(sockfd, &words, sizeof(int));

	printf("[+] Recived number of words in the file sent by client\n");

	int ii=0;

	while(ch != words)
	{
		if(ii==5)
		{
			fprintf(fp, "\n");
			ii=0;
		}

		ii=ii+1;

		if(ii==1)
		{
			read(sockfd, buffer, 1024);
			fprintf(fp, " PID: %s ", buffer);
			ch++;
		}
		else if(ii==2)
		{
			read(sockfd, buffer, 1024);
			fprintf(fp, " NAME: %s ", buffer);
			ch++;
		}
		else if(ii==3)
		{
			read(sockfd, buffer, 1024);
			fprintf(fp, " USER TIME: %s ", buffer);
			ch++;
		}
		else if(ii==4)
		{
			read(sockfd, buffer, 1024);
			fprintf(fp, " KERNEL TIME: %s ", buffer);
			ch++;
		}
		else 
		{
			read(sockfd, buffer, 1024);
			fprintf(fp, " CPU USAGE: %s ", buffer);
			ch++;
		}
		
	}

	printf("[+] The file has been recieved successfully\n");

	fclose(fp);

	getFile();

	FILE * f;

	f= fopen("/home/vanisikka/Desktop/ClientProcInfo.txt", "r");

	if (f == NULL) {
    perror("[-]Error in reading file.");
    exit(1);
  	}

  	for( int j=0;j<4;j++)
	{
		fscanf(f, "%s", buffer);
		write(sockfd, buffer, 1024);
	}

	fclose(f);

	printf("[+] Clients top CPU consuming process sent to the server\n");

	
	
	sleep(10);


	printf("[+] closing the client\n");
	close(sockfd);



	return 0;

}