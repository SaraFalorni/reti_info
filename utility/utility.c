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

//funzione che dato un nickname verifica se è disponibile,
//torna true se nickname è utilizzabile, false altrimenti
bool check_nickname(char* nickname) {

    FILE *file = fopen("./txt/used_nicknames.txt","r");

    if(!file) {
        perror("Errore nell'apertura del file");
        return false;
    }

    char buf[MAXCHAR_NICKNAME];

    while(fgets(buf,MAXCHAR_NICKNAME,file) != NULL) {
        size_t lenght = strlen(buf);
        buf[lenght-1] = '\0'; //per eliminare '\n' presente nella riga del file
        if(strcmp(buf,nickname) == 0) {
            fclose(file);    
            return false;
        }
        memset(buf,'\0', MAXCHAR_NICKNAME);
    }
        
    fclose(file); 
    return true;  
}

//-------------------------------------------------------------------------------

void insert_nickname(char* nickname) {
  FILE *file = fopen("./txt/used_nicknames.txt","r");

  if(!file) {
      perror("Errore nell'apertura del file");
      return;
  }
  
  fprintf(file,"%s",nickname);
  
  fclose(file);
  
  return;
}

//-------------------------------------------------------------------------------
//funzione che modifica buf (che contiene l'intera riga)
//fa si che ci siano sono le possibili risposte
void get_answ_from_line(char *buf) {

    char *answer = strchr(buf,'=');

    size_t lenght = strlen(answer);
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
    strcpy(answer_up,answer);
    for(int i = 0; i < strlen(answer); i++)
        answer_up[i] = toupper(answer[i]);

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

//-------------------------------------------------------------------------------

//restituisce la classifica per un tema
void get_theme_leaderboard(char* theme) {

    FILE *file_leaderboard = fopen(theme,"r");

    if(!file_leaderboard) {
        perror("Errore nell'apertura del file");
        return;
    }

    FILE *file_themes_index = fopen("../txt/","r");

    if(!file_themes_index) {
        perror("Errore nell'apertura del file");
        return;
    }


}
//------------------------------------------------------------------
//dato un numero restituisce il nome del corrispondente tema
void get_theme_name(int num, char* buf) {
  read_line("./txt/indiceTemi.txt",buf,num);
  
  int len = strlen(buf);
  
  if(len > 0 && buf[len-1] == '\n') {
    buf[len-1] == '\0';
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
  
  printf("in quanti temi: numero %d, stringa %s \n", num, buf);
  return num;
}
