//FUNZIONI CHE AGISCONO SUI FILE:PRENDONO TEMI E DOMANDE, CONTROLLANO LA CORRETTEZZA DELLE RISPOSTE
#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "QAfunctions.h"

//-------------------------------------------------------------------------------------------------------------

//funzione per leggere una riga specifica in un file
//filename è il percorso del file da aprire, buf è inizialmente vuoto e line è il numero di riga (partendo da 0)
//la riga viene messa nella variabile buf
void readLine(const char* filename,char* buf, int line) {

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

//-------------------------------------------------------------------------------------------------------------

//funzione per leggere una domanda
//filename è il percorso del file da aprire, buf è inizialmente vuoto e numQ è il numero della domanda (partendo da 0)
//la domanda viene messa nella variabile buf
void getQuestionFromLine(char* buf) {
    
    //nel file le domande sono separate dalle relative risposte giuste dal delimitatore "="
    const char delimitator[] = "=";

    char *question = strtok(buf,delimitator);

    if(question != NULL)
        strcpy(buf,question);

    return;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che modifica buf (che contiene l'intera riga), fa si che ci siano sono le possibili risposte
//le possibili risposte sono separate da |
void getAnswerFromLine(char *buf) {

    //"=" è il separatore tra domanda e risposta in una riga del file
    char *answer = strchr(buf,'=');

    int lenght = strlen(answer);
    if(lenght > 0 && answer[lenght-1] == '\n')
      answer[lenght-1] = '\0'; //per eliminare '\n' presente nella riga del file

    if(answer != NULL)
        strcpy(buf,answer+1);

    return;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che controlla la correttezza di una risposta data
//in theme c'è una stringa con il percorso del file da aprire, answer è la risposta da controllare,
//numQ è il numero della domanda (a partire da 0)
//restituisce true se answer è giusta false altrimenti
bool check_answer(const char *theme, char* answer, int numQ) {

    //ricava la risposta dal file corrispondente
    char buf[MAXCHAR_LINE];
    readLine(theme,buf,numQ);

    get_answ_from_line(buf);
    
    //mette la risposta data in maiuscolo
    char *answer_up = malloc(strlen(answer)+1);
    
    for(int i = 0; i < strlen(answer); i++)
        answer_up[i] = toupper(answer[i]);
        
    answer_up[strlen(answer)] = '\0';

    //le risposte giuste sono separate da "|" neel file
    char delimitator[] = "|";

    char *right_answer = strtok(buf,delimitator);
  
    //confronta le parti della stringa right_answer delimitate da | con la risposta data
    while(right_answer != NULL) {
        if(strcmp(answer_up,right_answer) == 0) {
            //confronta le stringhe tutte in upper case per valutarne solo il contenuto
          
            //libera la memoria
            free(answer_up);
            
            return true;
        }
        right_answer = strtok(NULL,delimitator);
    }

    return false;
    //libera la memoria
    free(answer_up);
}

//-------------------------------------------------------------------------------------------------------------

//per semplicità i temi sono identificati da un indice 
//corrispondente al numero di riga nel file "./txt/indiceTemi.txt" (a partire da 1)
//dato un indice (num) di tema restituisce il nome del corrispondente tema in buf
void getThemeName(int num, char* buf) {
    readLine("./txt/indiceTemi.txt",buf,num);
  
    int len = strlen(buf);
    
    if(len > 0 && buf[len-1] == '\n') {
        buf[len-1] = '\0';
    }
    if(len > 0 && buf[len-2] == '\r') {
        buf[len-2] = '\0';
    }
}

//-------------------------------------------------------------------------------------------------------------

//per semplicità i temi sono identificati da un indice 
//corrispondente al numero di riga nel file "./txt/indiceTemi.txt" (a partire da 1)
//dato un indice (themeChosen) di tema restituisce il nome del corrispondente file da aprire in bufFile
void getFilenameFromIndex(char* bufFile, int themeChosen,struct Session* current_session) {
    int lenTheme = strlen(current_session->availableThemes[themeChosen].name);
    int lenPre = strlen("./txt/");
    int lenPost = strlen(".txt");
    
    memcpy(bufFile,"./txt/",lenPre);//prefisso
    memcpy(bufFile+lenPre, current_session->availableThemes[themeChosen].name,lenTheme);//nome del tema
    memcpy(bufFile+lenPre+lenTheme, ".txt", lenPost+1);//parte finale
}
