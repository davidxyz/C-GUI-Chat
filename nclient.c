//Modified by Omokolade Hunpatin and Jeremy Muoy
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <ncurses.h>
#define BUFFSIZE 500 
#define MAXUSERSIZE 20
#define TEXTSIZE 150
int kbhit();
int recvp(WINDOW*,int*,int,int,WINDOW*,char*,int,char*,size_t,int );
WINDOW *create_newwin(int,int,int,int);
int main(int argc, char** argv){
	int port,sd,row,col,crow,ccol,ctcol;
	char buffer[BUFFSIZE],tmp[BUFFSIZE];
	char username[MAXUSERSIZE],*user;//samething temp holder variable
	if(argc!=4){
		fprintf(stderr,"Usage: %s takes 3 arguments a server, a portname, and a username", argv[0]);
		exit(EXIT_FAILURE);
	}
	port= atoi(argv[2]);
	bzero(username,MAXUSERSIZE);
	memcpy(username,argv[3],strlen(argv[3]));
	user=username;
	//	their_addr.sin_family=AF_INET;
//	their_addr.sin_port = htons(port);
	//if(!inet_aton(argv[1], &their_addr.sin_addr.s_addr)){
	//	fprintf(stderr,"%s requires a valid server address", argv[0]);
	//	exit(EXIT_FAILURE);
	//}
	struct addrinfo* ai,*ac;
	struct addrinfo hints;
	hints.ai_family = AF_INET;    
	    hints.ai_socktype = SOCK_DGRAM; 
	    hints.ai_flags = 0;   
	    hints.ai_protocol = port;        
	    hints.ai_canonname = NULL;
	    hints.ai_addr = NULL;
	    hints.ai_next = NULL;
	if(getaddrinfo(argv[1],argv[2],NULL,&ai)<0){
		close(sd);
		fprintf(stderr,"please input a vaild server!\n",argv[1]);
		exit(EXIT_FAILURE);
	}
	for (ac = ai; ac != NULL; ac = ac->ai_next) {
		sd = socket(AF_INET, SOCK_DGRAM,0);
		fcntl(sd, F_SETFL, O_NONBLOCK);
		if (sd == -1)
		    continue;
	       if (connect(sd, ac->ai_addr, ac->ai_addrlen) != -1)
		    break;                  /* Success */
	       close(sd);
	    }
	
//	their_addr.sin_addr.s_addr=ai.ai_addr;
//	memset(&(their_addr.sin_zero),'\0',8);
	//char host[NI_MAXHOST],service[NI_MAXSERV];	
	WINDOW *chat_win,*text_win,*user_win;
//	if(connect(sd,ai->ai_addr,ai->ai_addrlen)){
//		close(sd);
//		fprintf(stderr,"Failed to connect to %s!\n",argv[1]);
//		exit(EXIT_FAILURE);
//	}else{ 
		initscr();
		crow=1;
		ccol=2;
		ctcol=2;
		getmaxyx(stdscr,row,col);
		chat_win=create_newwin(row-3,col-30,0,0);
		user_win=create_newwin(row-2,col,0,col-30);
		text_win=create_newwin(3,col,row-3,0);
		scrollok(chat_win, TRUE);
		idlok(chat_win,TRUE);
		scrollok(text_win, TRUE);
		noecho();
		if(sd!=-1){	
			mvwprintw(chat_win,crow++,ccol,"connected to server %s",argv[1]);
			wrefresh(chat_win);
		}else{
			mvwprintw(chat_win,crow++,ccol,"couldn't connect to server  %s ... exiting",argv[1]);
			wrefresh(chat_win);
			sleep(1);
			delwin(text_win);
			delwin(chat_win);
			delwin(user_win);
			endwin();	
			return 0;
		}
//	}
	
	int bsize=BUFFSIZE;
	int nread;
	//send user name to server
	bzero(buffer,BUFFSIZE);
	bzero(tmp,BUFFSIZE);
	memcpy(buffer,username,strlen(username));
	send(sd,buffer,strlen(username),0);
	int i=0;
	int j;
	char c;
	int getusers=1;
	wmove(text_win,1,ctcol);
	wrefresh(text_win);
	while(1){
		bzero(buffer,BUFFSIZE);
		if(kbhit()){
			c=getchar();
			if(c=='\r') continue;
			tmp[i]=c;
			i++;
			mvwaddch(text_win,1,ctcol,c);
			wrefresh(text_win);
			ctcol++;
			while(1){
				if(kbhit()){
					c=getchar();
					if(c=='\r') break;
					if(c==(char)127||c==(char)8){
						if(i>0){
						ctcol--;
						wmove(text_win,1,ctcol);
						wdelch(text_win);
						wrefresh(text_win);
						tmp[i]='\0';
						i--;
						}
						continue;
					} 	
					if(i>BUFFSIZE)
						continue;
					tmp[i]=c;
					i++;
					mvwaddch(text_win,1,ctcol,c);
					wrefresh(text_win);
					ctcol++;
				}	
			if((nread=recvp(chat_win,&crow,ccol,col-28,user_win,user,sd,buffer,BUFFSIZE,0))!=-1){
				 wmove(text_win,1,ctcol);
				wrefresh(text_win);
			}
			}
		}
		if(i>0){
			if(memcmp(tmp,"clear\0",6)==0){
				wmove(chat_win,1,ccol);
				wclrtobot(chat_win);
				box(chat_win, 0 , 0);
				wrefresh(chat_win);
				box(text_win,0,0);
				wrefresh(text_win);
				crow=1;
				ccol=1;
			}else{
				send(sd,tmp,i,0);
			}
			wmove(text_win,1,0);
			wclrtoeol(text_win);
			wrefresh(text_win);
			ctcol=1;
			i=0;
			bzero(tmp,BUFFSIZE);
		}
		if((nread=recvp(chat_win,&crow,ccol,col-28,user_win,user,sd,buffer,BUFFSIZE,0))!=-1){
			 wmove(text_win,1,ctcol);
			wrefresh(text_win);
			if(memcmp(buffer,"close\0",6)==0){
				sleep(1);
				break;
			}
			//onetime thing
			if(getusers){
				getusers=0;
				memcpy(tmp,":users:",7);
				send(sd,tmp,7,0);
				bzero(tmp,BUFFSIZE);
			}
		}	
	}
	close(sd);
	delwin(text_win);
	delwin(chat_win);
	delwin(user_win);
	endwin();
	return 0;
}
int recvp(WINDOW *chatwin,int* chy,int chx,int maxcol,WINDOW* usrwin,char* uname,int sd,char *buf,size_t len,int flags){
	int response= recv(sd,buf,len,flags);
	if(response!=-1){
		int i,x;
		int ux=2;
		int uy=2;
		x=0;
		char username[MAXUSERSIZE];
		bzero(username,MAXUSERSIZE);
//trying to add username displays and whether or not they are away
		if(memcmp(buf,":users:",7)==0){
		//clear the window and restore border
			wmove(usrwin,0,0);
			wclrtobot(usrwin);
			box(usrwin, 0 , 0);
			wrefresh(usrwin);
			attron(A_BOLD|A_UNDERLINE);
			mvwprintw(usrwin,1,10,"USERS");
			wrefresh(usrwin);
			attroff(A_BOLD|A_UNDERLINE);
			for(i=7;i<len&&buf[i]!='\0';i++){
				if(buf[i]==(char)6){ 
					x=0;
					mvwprintw(usrwin,uy,ux,"%s",username);
					wrefresh(usrwin);
					uy++;
					bzero(username,MAXUSERSIZE);
					continue;
				}
				username[x++]=buf[i];		
			}
			return response;
		}
		if(memcmp(buf,"close\0",6)==0){
			 mvwprintw(chatwin,*chy,chx,"%s has left the chat",uname);
			wrefresh(chatwin);
			(*chy)++;

		}else{
			mvwprintw(chatwin,*chy,chx,"%s\n",buf);
			wrefresh(chatwin);
			for(i=0;i<response/maxcol+1;i++)
				(*chy)++;

		}
		return response;
	}else return response;
}
WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);	
	wrefresh(local_win);		

	return local_win;
}
int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}
