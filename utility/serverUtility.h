#include "utility.h"


#define SERVER_IP "127.0.0.1"

void get_nickname(int client_fd,struct Session* current_session);
void init_session(struct Session* current_session);
bool insert_player(char* nickname,struct Session* current_session);
void send_themes(int client_fd,struct Session* current_session);
