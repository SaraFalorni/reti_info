#include "utility.h"


//-------------------------------------------------------------------------------



//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------------
//ELIMINAZIONE DI SPAZI IN PIÙ NON IMPLEMENTATA, DA FARE!


void remove_spaces(char* str) {
  //rimozione spazi iniziali
  while(isspace((unsigned char)* str)) 
    str++;
    
  char* fine = str + strlen(str) -1;
  
  //rimozione spazi finali
  while(fine > str && isspace((unsigned char)* str)) 
    fine--;
    
  *(fine + 1) = '\0';
}

//-------------------------------------------------------------------------------

//------------------------------------------------------------------


//------------------------------------------------------------------


int recv_all_bytes(int socket, void *buf, int len) {
  int tot_rec = 0;
  int bytes_rec = 0;
  
  while(tot_rec < len) {
    bytes_rec = recv(socket,buf+tot_rec,len - tot_rec,0);
    if(bytes_rec <= 0)
      return -1;
    tot_rec += bytes_rec;
  }
  
  return tot_rec;
}




//print session
void print_session(struct Session* current_session) {
  printf("numero temi: %d\n", current_session->num_themes);
  for(int i = 0 ; i < current_session->num_themes ; i++) 
      printf(" %s\n", current_session->availableThemes[i]);
  struct Player* ptr= current_session->players;
  while(ptr != NULL)
  {
    printf("giocatore: %s\n",ptr->nickname);
    for(int i = 0 ; i < current_session->num_themes ; i++) 
      printf(" punteggio %s: %d\n", current_session->availableThemes[i], ptr->themePoints[i]);
    printf("\n");
    ptr= ptr->next;
  }
}
