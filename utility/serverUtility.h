#include "utility.h"


#define SERVER_IP "127.0.0.1"

void* client_handler(void* arg);
void get_nickname(int client_fd, char* name);
void init_session();
void show_overview();
bool insert_player(char* nickname);
void send_themes(int client_fd, char* nickname);
void playquiz(int themeChosen,char* nickname,int client_fd);
int updatePoints(int numq,char* bufR,int themeChosen, char* nickname);
struct Player* get_player(struct Player* current_player,char* nickname);
struct Player* find_last_player(struct Player* players);
struct Player* get_player_index(struct Player* current_player,int index) ;
struct Player** get_theme_rankings();
struct Player* copy_player(struct Player* current_player,int index_theme);
struct Player** get_final_rankings(struct Player** rankings,struct Player*** buckets,struct Player*** bucket_tails );
void print_rankings(struct Player** rankings);
void print_completed_quiz(struct Player** rankings);
void check_comand(int client_fd,char* msg);
void do_show_score(int client_fd);
void do_endquiz(int client_fd);
