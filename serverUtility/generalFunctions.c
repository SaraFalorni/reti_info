#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "generalFunctions.h"

//inizializzazione mutex per players
pthread_mutex_t lockPlayers = PTHREAD_MUTEX_INITIALIZER;
//inizializzazione della uova sessione di gioco
struct Session current_session; 

//-------------------------------------------------------------------------------------------------------------

//funzione che inizializza i valori di current_session per iniziare una funzione di gioco
void initSession() {
    //inizialmente ci sono 0 giocatori
    current_session.players = NULL;
    current_session.num_players = 0;
    current_session.numThemes = NUM_THEMES;
    
    initQuizThemes();
    
    showOverview();//funzione che mostra i giocatori connessi
    return;
}

//-------------------------------------------------------------------------------------------------------------

//mostra i temi disponibili e i client collegati
void showOverview() {
    printf("Trivia Quiz\n");

    for(int i = 0 ; i < NUM_SEPARATOR; i++)
        printf("+");
        
    printf("\nTemi:\n");
    for(int i = 0 ; i < current_session.numThemes ; i++) {
        printf("%d - %s\n",i+1,current_session.availableThemes[i].name);
    }
    
    for(int i = 0 ; i < NUM_SEPARATOR; i++)
        printf("+");
        
    pthread_mutex_lock(&lockPlayers); 
    printf("\nPartecipanti (%d)\n",current_session.num_players);
    for(int i = 0 ; i < current_session.num_players ; i++) {
        printf("- %s\n", getPlayerIndex(current_session.players,i)->nickname );
    }  
    pthread_mutex_unlock(&lockPlayers);
}

//-------------------------------------------------------------------------------------------------------------

//funzione che inizializza un nuovo ClientSet
void initClientSet(struct ClientSet* set) {
    set = malloc(sizeof(struct ClientSet));
    //errore nel malloc
    if(set == NULL) {
        perror("errore nel malloc");
        exit(EXIT_FAILURE);
    }

    //inizializzazione dei campi
    set->numClients = 0;
    
    //per indicare che clientSocket è vuoto c'è -1
    memset(set->clientSockets, -1, sizeof(set->clientSockets));

    return set;

}

//-------------------------------------------------------------------------------------------------------------
//funzione che controlla se c'è un posto disponibile in un set già creato altrimenti lo crea
bool assignClientToSet(int client_fd, struct ClientSet* sets, int* numSets) {
    for(int i = 0; i < MAX_CLIENTSETS ; i++) {
        if(sets[i].numClients < MAX_CLIENT_IN_THREAD) {
            //c'è posto quindi inserisce il client e ritorna true
            sets[i].clientSockets[sets[i].numClients-1] = client_fd;
            sets[i].numClients++;
            return true;
        }
        //se non c'è posto controlla il set successivo
    }

    //se esce dal for vuol dire che non c'è un set con posti liberi

    //controlla se è possibile crearlo
    if((*numSets) < MAX_CLIENTSETS) {
        //crea nuovo set
        initClientSet(&sets[(*numSets)]);
        if(pthread_create(sets[(*numSets)].thread, NULL, clientHandler, &sets[(*numSets)]) != 0) {
            perror("Errore nella creazione del thread");
            close(client_fd);
        }
        (*numSets)++;
        
        sets[(*numSets)-1].clientSockets[0] = client_fd;
        sets[(*numSets)-1].numClients++;
        return true;
    }

    return false;
}


//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce il collegamento di un nuovo client
void* clientHandler(void* arg) {
    struct ClientSet *set = (struct ClientSet*)arg;

    //il primo msg che riceve è il nickname
    char nickname[MAXCHAR_NICKNAME];
    
    while(1) {

    }
    
    
    
    /*int client_fd = *(int*)arg;
    free(arg);
      
    //il primo msg che riceve è il nickname
    char nickname[MAXCHAR_NICKNAME];
    
    while(1) {
        getNickname(client_fd,nickname);
      
        //quando un client si collega stampa le classifiche e chi ha completato i quiz 
        struct Player** rankings = getThemeRankings(client_fd);
        printRankings(rankings);
        printCompletedQuiz(rankings);
    
        while(getPlayer(current_session.players,nickname) != NULL) {
          //una volta registrato il nuovo giocatore invia i temi disponibili
          sendThemes(client_fd, nickname);
        }
    }
    
    close(client_fd);
    return NULL;*/
}

//-------------------------------------------------------------------------------------------------------------

void initQuizThemes() {
    //inizializzazione dei temi

    //per ogni tema legge da file il nome del tema, domande e risposte corrette
    for(int i = 0; i < NUM_THEMES; i++) {
        char buf[MAXCHAR_LINE];
        readLine("./txt/indiceTemi.txt",buf,i);
        buf[strcspn(buf,"\n")] = '\0';
        
        //copia il nome dei temi nella corrispondente struttura dati 
        int len = strlen(buf)+1;
        current_session.availableThemes[i].name = malloc(len);
        if(current_session.availableThemes[i].name == NULL) {
            //errore nel malloc
            perror("errore nel malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(current_session.availableThemes[i].name,buf);
        
        initThemePrompt(i); //inizializza domande e risposte per l'i-esimo tema
    }

}

//-------------------------------------------------------------------------------------------------------------

void initThemePrompt(int numTheme) {

    char fileName[MAXCHAR_LINE];
    getFilenameFromIndex(fileName,numTheme,&current_session);//dato l'indice del tema scrive in fileNAme il nome del file da aprire per leggere le domande
    
    for(int numPrompt = 0; numPrompt < NUM_Q ; numPrompt++) {
        
        char lineBuf[MAXCHAR_LINE];
        readLine(fileName,lineBuf,numPrompt); 
        //il file relativo ad ogni tema è strutturato in modo che ogni riga sia così fatta:
        //domanda?=rispostaGiusta1|rispostaGiusta2\n 
        //il numero di risposte giuste presenti è variabile da domanda a domanda

        
        char Qbuf[MAXCHAR_LINE];
        strcpy(Qbuf,lineBuf);
        getQuestionFromLine(Qbuf);
        
        //mette la domanda nell'apposita struttura dati della current_session
        current_session.availableThemes[numTheme].quiz[numPrompt].question = malloc(strlen(Qbuf)+1);
        if(current_session.availableThemes[numTheme].quiz[numPrompt].question == NULL) {
            //errore nel malloc
            perror("errore nel malloc");
            exit(EXIT_FAILURE);
        }
        
        strcpy(current_session.availableThemes[numTheme].quiz[numPrompt].question, Qbuf);
        
        //mette le risposte nelle apposite strutture dati
        char Abuf[MAXCHAR_LINE];
        strcpy(Abuf,lineBuf); 
        getAnswerFromLine(Abuf);//risposte giuste separate da "|"
        int numA = getNumAnswers(Abuf);
        current_session.availableThemes[numTheme].quiz[numPrompt].numAnswers = numA; //ritorna il numero di risposte giuste presenti
        current_session.availableThemes[numTheme].quiz[numPrompt].answer = malloc(numA * sizeof(char*));
        if(current_session.availableThemes[numTheme].quiz[numPrompt].answer == NULL) {
            //errore nel malloc
            perror("errore nel malloc");
            exit(EXIT_FAILURE);
        }    
        //divide la stringa con le risposte e mette ogni risposta nell'apposita struttura dati
        char *token = strtok(Abuf,"|");
        for(int i = 0; i < numA ; i++) {
            current_session.availableThemes[numTheme].quiz[numPrompt].answer[i] = malloc(strlen(token)+1);
            if(current_session.availableThemes[numTheme].quiz[numPrompt].answer[i] == NULL) {
                //errore nel malloc
                perror("errore nel malloc");
                exit(EXIT_FAILURE);
            } 
            token[strcspn(token,"\n")] = '\0';
            strcpy(current_session.availableThemes[numTheme].quiz[numPrompt].answer[i], token);
            token = strtok(NULL, "|");
        }

    }//fine for
}

