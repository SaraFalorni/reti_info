#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h> //toupper()
#include <pthread.h>

#define MAXCHAR_LINE 1024
#define MAXCHAR_NICKNAME 20
#define MAXCHAR_THEME 30
#define BACKLOG 5
#define MSG_LEN 2
#define MSG_OK "OK"
#define MSG_NO "NO"
#define NUM_Q 5 //numero di domande per ogni tema

typedef enum {false, true} bool;

struct Theme {
  char* name;
  int points; //-1 valore a cui è inizializzato, vuol dire che il giocatore non ci ha ancora giocato
  //struct Theme* next;
};
struct Player {
    char* nickname;
    struct Theme* theme;
    struct Player* next;
};

struct Session {
  struct Player* players;
  int num_players;
  char** availableThemes;
  int num_themes;
};

void read_line(const char* filename,char* buf, int line);
void read_q(const char* filename,char* buf, int numQ);
bool check_nickname(char* nickname);
bool check_answer(const char *theme, char* answer, int numQ);
void get_answ_from_line(char *buf);
void remove_spaces(char* str);
void get_theme_leaderboard(char* theme);
void get_theme_name(int num, char* buf);//ritorna il numero di temi disponibili
int quanti_temi();
int recv_all_bytes(int socket, void *buf, int len);
void get_filename_from_index(char* bufFile, int themeChosen,struct Session* current_session);
void print_session(struct Session* current_session);
