#include "utility.h"

//funzione per leggere una riga specifica in un file
void read_line(const char* filename,char* buf, int line) {

    FILE *file = fopen(filename,"r");

    if(!file) {
        perror("Errore nell'apertura del file");
        printf("%s \n", filename); 
        return;
    }

    int current_line = 0;

    while(fgets(buf,MAXCHAR_LINE,file) != NULL) {
        if(current_line == line) {
            fclose(file);    
            return;
        }
        current_line++;
    }
        
    fclose(file); 
    return;   
}

//-------------------------------------------------------------------------------

//funzione per leggere la domanda di un tema identificata da un numero numQ
void read_q(const char* filename,char* buf, int numQ) {

    char buf2[MAXCHAR_LINE];
    read_line(filename,buf2,numQ);

    const char delimitator[] = "=";

    char *question = strtok(buf2,delimitator);

    if(question != NULL)
        strcpy(buf,question);

    return;
}

//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
//funzione che modifica buf (che contiene l'intera riga)
//fa si che ci siano sono le possibili risposte
void get_answ_from_line(char *buf) {

    char *answer = strchr(buf,'=');

    size_t lenght = strlen(answer);
    if(lenght > 0 && answer[lenght-1] == '\n')
      answer[lenght-1] = '\0'; //per eliminare '\n' presente nella riga del file

    if(answer != NULL)
        strcpy(buf,answer+1);

    return;
}

//-------------------------------------------------------------------------------------------------------------
//ELIMINAZIONE DI SPAZI IN PIÙ NON IMPLEMENTATA, DA FARE!
bool check_answer(const char *theme, char* answer, int numQ) {

    char buf[MAXCHAR_LINE];
    read_line(theme,buf,numQ);

    get_answ_from_line(buf);
    
    //mette la risposta data in maiuscolo
    char *answer_up = malloc(strlen(answer)+1);
    for(int i = 0; i < strlen(answer); i++)
        answer_up[i] = toupper(answer[i]);
    answer_up[strlen(answer)] = '\0';
        
    //remove_spaces(answer_up); //rimuove spazi all'inizio o la fine

    char delimitator[] = "|";

    char *right_answer = strtok(buf,delimitator);

    while(right_answer != NULL) {
        if(strcmp(answer_up,right_answer) == 0) {
            //confronto le stringhe tutte in upper case per valutarne solo il contenuto
            return true;
        }
        right_answer = strtok(NULL,delimitator);
    }

    return false;

}

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
//dato un numero restituisce il nome del corrispondente tema
void get_theme_name(int num, char* buf) {
  read_line("./txt/indiceTemi.txt",buf,num);
  
  int len = strlen(buf);
  
  if(len > 0 && buf[len-1] == '\n') {
    buf[len-1] = '\0';
  }
  if(len > 0 && buf[len-2] == '\r') {
    buf[len-2] = '\0';
  }

}

//------------------------------------------------------------------
int quanti_temi() {
  char buf[MAXCHAR_LINE];
  read_line("./txt/indiceTemi.txt",buf,0);
  
  int len = strlen(buf);
  
  if(len > 0 && buf[len-1] == '\n') {
    buf[len-1] = '\0';
  }
  if(len > 0 && buf[len-2] == '\r') {
    buf[len-2] = '\0';
  }
  
  char* endptr;
  int num = (int)strtol(buf,&endptr,10);
  
  if(endptr == buf || *endptr != '\0') {
    return 0; //conversione fallita
  }
  return num;
}

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

void get_filename_from_index(char* bufFile, int themeChosen,struct Session* current_session) {
  int lenTheme = strlen(current_session->availableThemes[themeChosen]);
  int lenPre = strlen("./txt/");
  int lenPost = strlen(".txt");
  memcpy(bufFile,"./txt/",lenPre);
  memcpy(bufFile+lenPre, current_session->availableThemes[themeChosen],lenTheme);
  memcpy(bufFile+lenPre+lenTheme, ".txt", lenPost+1);
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
