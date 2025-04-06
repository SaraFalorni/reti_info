#include "serverUtility.h"

void init_session(struct Session* current_session) {
  current_session->players = NULL;
  current_session->num_players = 0;
  
  current_session->num_themes = quanti_temi();
  printf("numero di temi disponibili current_session: %d\n",current_session->num_themes);//da togliere
  
  current_session->availableThemes = (char**)malloc(current_session->num_themes * sizeof(char*));
  
  for(int i = 0; i < current_session->num_themes; i++) {
    char buf[MAXCHAR_LINE];
    get_theme_name(i+1,buf);
    
    size_t len_themeName = strlen(buf)+1;
    current_session->availableThemes[i] = malloc(len_themeName);
    memcpy(current_session->availableThemes[i],buf,len_themeName);
    
  }
  
  return;
}

bool insert_player(char* nickname,struct Session* current_session) {
  struct Player* p;
  p = current_session->players;
  while(p != NULL) {
    if( strcmp(nickname,p->nickname))
      return false;
    p = p->next;
  }
  p = (struct Player*)malloc(sizeof(struct Player*));
  size_t len_nickname = strlen(nickname)+1;
  p->nickname = (char*)malloc(len_nickname);
  memcpy(nickname,p->nickname,len_nickname);  
  
  //INIZIALIZZAZIONE DEI TEMI CON POINTS = -1
  struct Theme* pTheme = p->theme;
  for(int i = 0; i < current_session->num_themes ; i++) {
    pTheme = (struct Theme*)malloc(sizeof(struct Theme*)); 
    
    size_t len_themeName = strlen(current_session->availableThemes[i]) + 1;
    pTheme->name = (char*)malloc(len_themeName);
    memcpy(pTheme->name,current_session->availableThemes[i],len_themeName);
    
    pTheme->points = -1;//valore di inizializzazione
    pTheme = pTheme->next;
  }
  //giocatore inserito correttamente
  
  
  return true;
}


void get_nickname(int client_fd,struct Session* current_session) {
  char nickname[MAXCHAR_NICKNAME];
  
  ssize_t bytes_received = recv(client_fd, nickname, MAXCHAR_NICKNAME, 0);
  
  if(bytes_received == -1) {
    perror("Errore in recv() per nickname");
    exit(EXIT_FAILURE);
  }
  
  if(insert_player(nickname,current_session)) {   
    //messaggio di ok a client
    if(send(client_fd, MSG_OK, MSG_LEN, 0) == -1) {
      perror("Errore in send() dell'ok al nickname");
      exit(EXIT_FAILURE);
    }    
  }
  else {
    //messaggio non ok al client
    if(send(client_fd, MSG_NO, MSG_LEN, 0) == -1) {
      perror("Errore in send() del no al nickname");
      exit(EXIT_FAILURE);
    } 
  }
}

void send_themes(int client_fd,struct Session* current_session) {
//manda un messaggio al client con il numero di temi e aspetta un feedback sulla ricezione di quest'ultimo
  char msg[MSG_LEN];
  printf("numero di temi: %d\n", current_session->num_themes);

  if(send(client_fd, &current_session->num_themes, sizeof(&current_session->num_themes), 0) == -1) {
      perror("Errore in send() del numero di temi");
      exit(EXIT_FAILURE);
    }

    ssize_t bytes_received = recv(client_fd, msg, MSG_LEN, 0);
    
    if(bytes_received == -1) {
      perror("Errore in recv() di conferma del numero di temi");
      exit(EXIT_FAILURE);
    }
    
    if(strcmp(msg,MSG_OK)) {
      printf("numero temi ok\n");
      //ciclo per mandare il nome di tutti i temi disponibili
      for(int i = 0 ; i < current_session->num_themes ; i++) {
        if(send(client_fd, &current_session->availableThemes[i],MAXCHAR_THEME, 0) == -1) {
        perror("Errore in send() del nome del tema %s", current_session->availableThemes[i]);
        exit(EXIT_FAILURE);
        }
        ssize_t bytes_received = recv(client_fd, msg, MSG_LEN, 0);
    
        if(bytes_received == -1) {
          perror("Errore in recv() di conferma della recezione del nome del tema");
          exit(EXIT_FAILURE);
        }
        if(strcmp(msg,MSG_NO))
          i--;//così viene rimandato lo stesso tema
      }
      
    }
}



