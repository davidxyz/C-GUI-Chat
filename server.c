//modified by Omokolade Hunpatin and Jeremy Muoy
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#define MAXCLIENT 10
#define BUFFSIZE 500 
int main(int argc, char** argv){
	int myport,sockfd,their_addr_len;
	char buffer[BUFFSIZE],tmp[BUFFSIZE];
	struct sockaddr_in my_addr,their_addr;
	if(argc!=2){
		fprintf(stderr,"Usage: %s portnum\n", argv[0]);
		exit(EXIT_FAILURE);
	}
myport = atoi (argv[1]);

  sockfd = socket (AF_INET, SOCK_DGRAM, 0);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons (myport);
  my_addr.sin_addr.s_addr = INADDR_ANY;
  memset (&(my_addr.sin_zero), '\0', 8);

  if (bind (sockfd, (struct sockaddr *) &my_addr, sizeof (struct sockaddr)))
    {
      close (sockfd);
      fprintf (stderr, "Failed to bind socket!\n");
      exit (EXIT_FAILURE);
    }
  else
    {
      printf ("Server listening on port %d\n", myport);
    }

  their_addr_len = sizeof (struct sockaddr_in);

  struct sockaddr_in client_addrs[MAXCLIENT];

char usernames[MAXCLIENT][20];
  int result;
bzero(client_addrs,MAXCLIENT);
bzero(buffer,BUFFSIZE);
bzero(tmp,BUFFSIZE);
int foundAddr=0;
//found address and username index
int j=-1;
//empty space index in the buffer -1 means no empty space
int espace=-1;
int userlen;
int nread;
//indices
int i;
int n;
int x;
for(i=0;i<MAXCLIENT;i++) client_addrs[i].sin_addr.s_addr=0;
//updated variables
int updated=0;
char code[1]={(char)6};
for(i=0;i<MAXCLIENT;i++) bzero(usernames[i],20);
while(1){
//we got somethin from somebody
	if((nread=recvfrom(sockfd,buffer,BUFFSIZE,0,(struct sockaddr *)&their_addr,&their_addr_len))!=-1){
		//do we have this address already?
		if(memcmp(buffer,"exit\0",5)==0) {
			memcpy(tmp,"server has exited.",18);
			for(i=0;i<MAXCLIENT;i++){
				if(client_addrs[i].sin_addr.s_addr!=0){
					sendto(sockfd,tmp,18,0,(struct sockaddr *) &client_addrs[i],their_addr_len);
				}
			}

			break;
		}
		printf("got %s w/ %d bytes\n",buffer,nread);
		for(i=0;i<MAXCLIENT;i++){
			if(client_addrs[i].sin_addr.s_addr==0) {espace=i;continue;}

			if(!foundAddr&&client_addrs[i].sin_addr.s_addr==their_addr.sin_addr.s_addr){
				foundAddr=1;
				j=i;
			}
		}
		//we found the address in our saved address
		if(foundAddr){
			//close the connection if the user sends close
		userlen=strlen(usernames[j]);
		if(memcmp(buffer,"close\0",6)==0){
			printf("%s is leaving\n",usernames[j]);
			memcpy(buffer,usernames[j],userlen);	
			memcpy(buffer+userlen," has left the chat",18);
			memcpy(tmp,"close",5);
			sendto(sockfd,tmp,5,0,(struct sockaddr *) &client_addrs[j],their_addr_len);
			for(i=0;i<MAXCLIENT;i++){
				if(j!=i&&client_addrs[i].sin_addr.s_addr!=0){
					sendto(sockfd,buffer,18+userlen,0,(struct sockaddr *) &client_addrs[i],their_addr_len);
				}
			}
			client_addrs[j].sin_addr.s_addr=0;
			bzero(usernames[j],userlen);
			//enter into if statement below
			updated=1;
		}
		if(memcmp(buffer,":users:",7)==0||updated){
                        x=7;
			memcpy(tmp,":users:",7);
                        for(i=0;i<MAXCLIENT;i++){
				if(usernames[i][0]!='\0'){
					memcpy(tmp+x,usernames[i],strlen(usernames[i]));
					//printf("%s\n",usernames[i]);
					x+=strlen(usernames[i]);
					memcpy(tmp+x,code,1);
					x++;
				}
                        }
			//printf("%s\n",tmp);
			for(i=0;i<MAXCLIENT;i++){
				if(client_addrs[i].sin_addr.s_addr!=0){
					sendto(sockfd,tmp,x,0,(struct sockaddr *) &client_addrs[i],their_addr_len);
				}
			}
			//printf("sent\n");
			updated=0;
			//reset everything
			 foundAddr=0;
			j=-1;
			espace=-1;
			bzero(buffer,BUFFSIZE);
			bzero(tmp,BUFFSIZE);
			continue;
                }

		//we loop through our saved address and send them what we got
		//prepended with username
		memcpy(tmp,buffer,nread);
		bzero(buffer,BUFFSIZE);
		memcpy(buffer,usernames[j],userlen);
		memcpy(buffer+userlen,":",1);
		memcpy(buffer+userlen+1,tmp,nread);
			for(i=0;i<MAXCLIENT;i++){
				if(client_addrs[i].sin_addr.s_addr!=0){
					sendto(sockfd,buffer,nread+1+userlen,0,(struct sockaddr *) &client_addrs[i],their_addr_len);
				}
			}
		//first time seeing address
		}else{
			//if there is an empty space{
			if(espace!=-1){
				bzero(usernames[espace],20);
				client_addrs[espace]=their_addr;
				memcpy(usernames[espace],buffer,nread);
				memcpy(buffer+nread," has joined the chat",20);				
				//tell all clients a user has joined
				for(i=0;i<MAXCLIENT;i++){
					if(client_addrs[i].sin_addr.s_addr!=0){
						sendto(sockfd,buffer,nread+20,0,(struct sockaddr *) &client_addrs[i],their_addr_len);
					}
				}
			//no empty spaces tell client he can't get in on this
			}else{
				memcpy(buffer,"Chat Server is full",19);	
				sendto(sockfd,buffer,19,0,(struct sockaddr *) &their_addr,their_addr_len);
			}
		}
	//reset everything
	foundAddr=0;
	j=-1;
	espace=-1;
	bzero(buffer,BUFFSIZE);
	bzero(tmp,BUFFSIZE);
	}
}
 close (sockfd);
  return 0;
}
