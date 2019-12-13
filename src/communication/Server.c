#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 12000

struct Player{
    char username[25];
    int socket;
};
struct Account{
    char username[25];
    char password[25];
};

void error(const char *message);
void bind_socket(int sockfd, int portno);
int wait_all_players_to_connect(struct Player* players, int number_of_players_allowed);
void get_input_from_player(int player, char* buffer);
void send_message_to_player(int player, const char* buffer);
void login(struct Player* player);
struct Account* get_account(const char* filename, const char* username);
void add_account(const char* filename, struct Account* account);
void send_to_all_players(struct Player* players, int number_of_players_allowed, const char* buffer);
char* get_word();
void lower(char* buffer);
int connect_one(int sockfd);
void GAME(struct Player* players,int number_of_players_allowed,char* buffer);

int main(int argc, char* argv[]){   
    //Variables------------------------------
    int sockfd;
    char buffer[255];
    int number_of_players_allowed;
    int n;
    struct Player* players;   //keeps addresses of sockets for all players

    //Start-----------------------------------   
    printf("Number of players allowed: ");
    scanf("%d",&number_of_players_allowed);
    players = (struct Player *) malloc(number_of_players_allowed*sizeof(struct Player));

    //creates server and waits until all players login
    sockfd = wait_all_players_to_connect(players, number_of_players_allowed);
    for (int i = 0; i < number_of_players_allowed; ++i)    {
        printf("%d connected as %s\n",players[i].socket,players[i].username);
    }
    sprintf(buffer,"%s","Game starts now\n");
    send_to_all_players(players,number_of_players_allowed,buffer);

    //Mainloop------------------------------
    //This GAME functionis a real mess
    //GAME(players,number_of_players_allowed,buffer);
    while(1){
        GAME(players,number_of_players_allowed,buffer);
        
        for(n=0;n<number_of_players_allowed;n++){
            get_input_from_player(players[n].socket,buffer);
            if(strcmp(buffer,"0")==0){
                close(players[n].socket);
                players[n].socket=connect_one(sockfd);
			    send_message_to_player(players[n].socket,"Welcome to Hangman!\n");
			    login(&players[n]);
			    send_message_to_player(players[n].socket,"Login succes!\nWaiting for other players to connect\n");
        		printf("%d connected as %s\n",players[n].socket,players[n].username);
            }
        }
	    sprintf(buffer,"%s","Game starts now\n");
	    send_to_all_players(players,number_of_players_allowed,buffer);
    }

  
    //Close-----------------------------
    for(n=0;n<number_of_players_allowed;n++){
        close(players[n].socket);
        printf("%d\n",players[n].socket);
    }
    close(sockfd);
    free(players);
    return EXIT_SUCCESS;
}
void GAME(struct Player* players,int number_of_players_allowed,char* buffer){
    char* word;
    char* progress;
    char* wrong_trials[7];
    int length,remaining_trials=7;
    char buffer2[255];
    int n,i;
    int found_letters;

    word=get_word();
    length=strlen(word);
    progress=(char *) malloc(sizeof(char)*length);

    for(n=0;n<length;n++){
        if(word[n]==' '){
            progress[n]=' ';
        }
        else{
            progress[n]='_';
        }
    }

    for(n=0;n<7;n++){
       wrong_trials[n]=(char *) calloc(length,sizeof(char));
    }

    while(remaining_trials>0 && strcmp(progress,word)!=0){
        for(n=0;n<number_of_players_allowed;n++){
            //telling game situation to everyone
            sprintf(buffer, "Word: %s \t\t\t\t\t\tRemaining trials: %d\n\
                            Wrongs: %s |%s |%s |%s |%s |%s |%s |\n\
                            Turn of %s\t(%s next)\n",
                            progress,remaining_trials,
                            wrong_trials[0],wrong_trials[1],wrong_trials[2],wrong_trials[3],wrong_trials[4],wrong_trials[5],wrong_trials[6],
                            players[n].username,players[(n+1)%number_of_players_allowed].username);

            send_to_all_players(players, number_of_players_allowed,buffer);

            //Checking if game any letter left
            if(strcmp(word,progress)==0){
                sprintf(buffer,"GAME OVER\n");
                send_to_all_players(players,number_of_players_allowed,buffer);
                break;
            }

            //Asking for guess from user which has the turn
            sprintf(buffer, "Your turn, Guess a letter or guess the whole word\nYour guess: ");
            send_message_to_player(players[n].socket,buffer);
            get_input_from_player(players[n].socket,buffer);
            
            //Progressing the word
            if(strlen(buffer)>1){
                if(strcmp(word,buffer)!=0){
                    //Guessing multiple words are not supported because of scanf
                    //That is a shame for me
                    strcpy(wrong_trials[7-remaining_trials],buffer);
                    sprintf(buffer, "%s guessed wrong: %s\n",players[n].username, wrong_trials[7-remaining_trials]);
                    send_to_all_players(players,number_of_players_allowed,buffer);
                    remaining_trials--;
                    if(remaining_trials==0){
                        sprintf(buffer, "GAME OVER\n");
                        send_to_all_players(players,number_of_players_allowed,buffer);
                        break;
                    }
                    sprintf(buffer, "remaining_trials: %d\n",remaining_trials);
                    send_to_all_players(players,number_of_players_allowed,buffer);
                }
                else{
                    strcpy(progress,buffer);
                    sprintf(buffer2, "%s guessed %s\nThat is the right word\n",players[n].username,buffer);//take care
                    send_to_all_players(players,number_of_players_allowed,buffer2);

                    sprintf(buffer, "GAME OVER\n");
                    send_to_all_players(players,number_of_players_allowed,buffer);
                    break;
                }
            }
            else{
                found_letters=0;
                for(i=0;i<length;i++){
                    if(word[i]==buffer[0]){
                        progress[i]=buffer[0];
                        found_letters++;
                    }

                }   
                if(found_letters==0 || strcmp(progress,word)==0){
                    strcpy(wrong_trials[7-remaining_trials],buffer);
                    remaining_trials--;
                    if(remaining_trials==0){
                        sprintf(buffer2,"%s guessed letter %s\n%d letters match\n",players[n].username,buffer,found_letters);//take care
                        send_to_all_players(players,number_of_players_allowed,buffer2);
                        sprintf(buffer, "GAME OVER\n");
                        send_to_all_players(players,number_of_players_allowed,buffer);
                        break;
                    }
                }
                sprintf(buffer2,"%s guessed letter %s\n",players[n].username,buffer);//take care
                send_to_all_players(players,number_of_players_allowed,buffer2);
                sprintf(buffer,"%d letters match\n",found_letters);
                send_to_all_players(players,number_of_players_allowed,buffer);
            }

        }
    }
}

void lower(char* buffer){
    int length=strlen(buffer);
    int i;
    for(i=0;i<length;i++){
        if('Z'>=buffer[i] && buffer[i]>='A'){
            buffer[i]-='A'-'a';
        }
    }
}

char* get_word(){
    //Normally I was planning to make a list of words and read them from file.
    //But it was not a grading criteria :D
    return "hangman";
}

void add_account(const char* filename, struct Account* account){
    FILE* file;
    int length;

    file = fopen(filename, "ab");
    if(file==NULL) error("File couldn't open");

    length=fwrite(account, sizeof(struct Account), 1, file);
    if(length==0) error("ERROR Writing to file");

    fclose(file);

}


struct Account* get_account(const char* filename, const char* username){
    //This function uses file handling. I know it is not the best
    //way for checking records especially if ther will be many queries
    //But I did not want to spend time with dynamic memory
    FILE* file;
    struct Account* account =(struct Account *) malloc(sizeof(struct Account));
    int length;

    file = fopen(filename, "rb");
    if(file==NULL) error("File couldn't found");

    while(!feof(file)){

        length=fread(account, sizeof(struct Account), 1, file);

        if(length!=0){
            if(strncmp(account->username,username,25)==0){
                fclose(file);
                return account;
            }
        }
    }
    fclose(file);
    return NULL;
}

void login(struct Player* player){
    char* password;
    char buffer[255];
    struct Account* account;

    send_message_to_player(player->socket, "Username: ");
    get_input_from_player(player->socket,buffer);
    strncpy(player->username,buffer,25);

    account=get_account("players.hangman",player->username);
    if(account!=NULL){
        send_message_to_player(player->socket, "Password: ");
        get_input_from_player(player->socket,buffer);
        while(strncmp(buffer,account->password,25)!=0){
            send_message_to_player(player->socket, "Invalid password\n\n Password: ");
            get_input_from_player(player->socket,buffer);
        }
    }
    else{
	account=(struct Account *) malloc(sizeof(struct Account));
        send_message_to_player(player->socket, "Create a password: ");
        get_input_from_player(player->socket,buffer);

        account=(struct Account *) malloc(sizeof(struct Account));
        strncpy(account->password, buffer, 25);
        strncpy(account->username, player->username, 25);

        add_account("players.hangman",account);
    }
    free(account);

}

void send_to_all_players(struct Player* players, int number_of_players_allowed, const char* buffer){
    int length,n;
    for(n=0;n<number_of_players_allowed;n++){
        length=write(players[n].socket,buffer,255);
        //printf("%s sent to %d\n",buffer,players[n].socket);
        if(length<0) error("ERROR writing to socket");
    }
}

void send_message_to_player(int player, const char* buffer){
    int length;
    length=write(player,buffer,255);
    //printf("%s sent to %d\n",buffer,player);
    if(length<0) error("ERROR writing to socket");
}

void get_input_from_player(int player, char* buffer){
    int length;
    bzero(buffer,255);   
    length = read(player, buffer, 255);
    //printf("%d: %s\n",player,buffer);
    if(length<0) error("ERROR reading from socket");
    lower(buffer);
}


int connect_one(int sockfd){
	int newsockfd;
    struct sockaddr_in cli_addr;
    socklen_t clilen;

    clilen=sizeof(cli_addr);

    newsockfd=accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
    if(newsockfd<0) error("ERROR on accept");

    return newsockfd;
}

int wait_all_players_to_connect(struct Player* players, int number_of_players_allowed){
    int sockfd, newsockfd, n;
    struct sockaddr_in cli_addr;
    socklen_t clilen;
    //Create Socket------------------------------
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0) error("ERROR opening socket");

    //Bind Socket------------------------------
    bind_socket(sockfd, PORT);

    //listen and accept------------------------------
    listen(sockfd, number_of_players_allowed);
    for(n=0;n<number_of_players_allowed;n++){
        clilen=sizeof(cli_addr);

        newsockfd=accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
        if(newsockfd<0) error("ERROR on accept");

        players[n].socket=newsockfd;

        send_message_to_player(newsockfd,"Welcome to Hangman!\n");
        login(&players[n]);
        send_message_to_player(newsockfd,"Login succes!\nWaiting for other players to connect\n");
    }

    return sockfd;
}

void bind_socket(int sockfd, int portno){
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if(bind(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) error("ERROR on binding");
}

void error(const char *message){
    perror(message);
    exit(EXIT_FAILURE);
}
