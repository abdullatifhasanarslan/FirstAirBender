/* A simple server in the internet domain using TCP
    The port number is passed as an argument*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 12000

void error(const char* msg);
int connect_to_host();
void get_from_host(int sockfd, char* buffer);
void send_to_host(int sockfd, const char* buffer);
void login(int sockfd);
int decide(char* buffer);
void PLAY(int sockfd, char* buffer);


int main(int argc, char* argv[]){
    //Variables------------------------------
    int sockfd;
    int will_play;

    char buffer[255];
    //Start-----------------------------------

    //Create socket and connect to host------------------------------
    sockfd = connect_to_host();

    //login-----------------------------
    login(sockfd);
    //"Game starts now\n"
    get_from_host(sockfd, buffer);
    printf("%s",buffer);

    do{
    	PLAY(sockfd,buffer);
    	printf("Do you want to play (1 for yes, 0 for no): ");
    	scanf("%d",&will_play);
    	sprintf(buffer,"%d",will_play);
    	send_to_host(sockfd,buffer);
    }while(will_play);
    
    
    close(sockfd);
    return EXIT_SUCCESS;
}

void PLAY(int sockfd, char* buffer){
    int n, decision;
    while(1){
        //Game situation
        get_from_host(sockfd,buffer);
        printf("%s\n",buffer);

        //Your turn or %s guessed
        get_from_host(sockfd,buffer);
        printf("%s",buffer);

        decision = decide(buffer);
        switch(decision){
            //GAME OVER
            case 0:
                return;
            //Your turn, Guess a letter or guess the whole word
            case 1:
                scanf("%s",buffer);
                send_to_host(sockfd,buffer);
                
                //Player which has the turn will get one extra message
                //%s guessed
                get_from_host(sockfd,buffer);
                printf("%s",buffer);
                //letters match       
                get_from_host(sockfd,buffer);
                printf("%s",buffer);
                break;
            //%s guessed ..
            case 2:         
                //letters match       
                get_from_host(sockfd,buffer);
                printf("%s",buffer);
                break;
        }
        if(decide(buffer)==0){
            return;
        }

    }
}


int decide(char* buffer){
    if(strcmp(buffer,"GAME OVER\n")==0) return 0;
    if(strcmp(buffer,"Your turn, Guess a letter or guess the whole word\nYour guess: ")==0) return 1;
    return 2;
}    

void login(int sockfd){
    char buffer[255];
    //"Welcome to Hangman!\n"
    get_from_host(sockfd, buffer);
    printf("%s",buffer);
    //"Username: "
    get_from_host(sockfd, buffer);
    printf("%s",buffer);

    scanf("%s",buffer);
    send_to_host(sockfd, buffer);
    //"Password: "
    get_from_host(sockfd, buffer);
    printf("%s",buffer);   
    
    do{
        scanf("%s",buffer);
        send_to_host(sockfd, buffer); 
        get_from_host(sockfd, buffer);
        printf("%s",buffer);
    }while(strcmp("Login succes!\nWaiting for other players to connect\n",buffer)!=0);
}


void get_from_host(int sockfd, char* buffer){
    int length;
    bzero(buffer,255);
    length = read(sockfd,buffer,255);
    //printf("host: %s\n",buffer);

    if(length<0) error("ERROR reading from socket");
}

void send_to_host(int sockfd, const char* buffer){
    int length;
    length=write(sockfd,buffer,255);
    if(length<0) error("ERROR writing to socket");
}

int connect_to_host(){
    struct sockaddr_in serv_addr;
    struct hostent* server;
    char hostname[50];
    int sockfd;

    sockfd=socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0) error("ERROR opening socket");

    //Find Host--------------------------------
    gethostname(hostname,50);
    server=gethostbyname(hostname);
    if(server==NULL){
        error("ERROR, no such host");
    }

    //Connect------------------------------------
    bzero((char*) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    bcopy(
          (char *) server->h_addr,
          (char *) &serv_addr.sin_addr.s_addr,
          server->h_length
         );
    serv_addr.sin_port = htons(PORT);

    if(connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr))<0){
        error("ERROR connecting");
    }

    return sockfd;
}

void error(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}
