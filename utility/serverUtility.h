#include "utility.h"


#define SERVER_IP "127.0.0.1"

void get_nickname(int client_fd,struct Session* current_session, char* name);
void init_session(struct Session* current_session);
bool insert_player(char* nickname,struct Session* current_session);
void send_themes(int client_fd,struct Session* current_session, char* nickname);
void playquiz(int themeChosen,char* nickname,struct Session* current_session,int client_fd);
int updatePoints(int numq,char* bufR,int themeChosen, char* nickname,struct Session* current_session);
struct Player* get_player(struct Session* current_session,char* nickname);
