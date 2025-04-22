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

//funzione che ritorna il numero di risposte presenti data la stringa che le contiene tutte
int getNumAnswers(char *bufA) {
    char* buf = malloc(strlen(bufA)); // buffer modificabile
    
    int num = 0;
    char* token = strtok(buf,"|");
    
    while(token != NULL) {
        num++;
        token = strtok(NULL,"|");
    }
    
    return num;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che controlla la correttezza di una risposta data
//in theme c'è una stringa con il percorso del file da aprire, answer è la risposta da controllare,
//numQ è il numero della domanda (a partire da 0)
//restituisce true se answer è giusta false altrimenti
bool checkAnswer(int theme, char* answer, int numQ) {

    //mette la risposta data in maiuscolo
    char *answer_up = malloc(strlen(answer)+1);
    for(int i = 0; i < strlen(answer); i++)
        answer_up[i] = toupper(answer[i]);

    //confronta le stringhe tutte in upper case per valutarne solo il contenuto
    //le stringhe delle risposte giuste sono già in maiuscolo
    for(int i = 0; i < current_session.availableThemes[theme].quiz[numQ].numAnswers; i++) {
        
        if(strcmp(answer_up,current_session.availableThemes[theme].quiz[numQ].answer[i]) == 0) {
            //libera la memoria
            free(answer_up);
            
            return true;
        }
    }
    //libera la memoria
    free(answer_up);

    return false;

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
