#include "utility.h"


#define SERVER_IP "127.0.0.1"

void* client_handler(void* arg);
void get_nickname(int client_fd, char* name);
void init_session();
bool insert_player(char* nickname);
void send_themes(int client_fd, char* nickname);
void playquiz(int themeChosen,char* nickname,int client_fd);
int updatePoints(int numq,char* bufR,int themeChosen, char* nickname);
struct Player* get_player(struct Player* current_player,char* nickname);
struct Player* find_last_player(struct Player* players);
