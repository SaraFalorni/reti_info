#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "generalFunctions.h"

//inizializzazione mutex per players
pthread_mutex_t lockPlayers = PTHREAD_MUTEX_INITIALIZER;
//inizializzazione della uova sessione di gioco
struct Session current_session; 

//-------------------------------------------------------------------------------------------------------------

//funzione che inizializza i valori di current_session per iniziare una funzione di gioco
void init_session() {
    //inizialmente ci sono 0 giocatori
    current_session.players = NULL;
    current_session.num_players = 0;
    
    initQuizThemes();
    
    show_overview();//funzione che mostra i giocatori connessi
    return;
}

//-------------------------------------------------------------------------------------------------------------

//mostra i temi disponibili e i client collegati
void show_overview() {
    printf("Trivia Quiz\n");

    for(int i = 0 ; i < NUM_SEPARATOR; i++)
        printf("+");
        
    printf("\nTemi:\n");
    for(int i = 0 ; i < current_session.num_themes ; i++) {
        printf("%d - %s\n",i+1,current_session.availableThemes[i]);
    }
    
    for(int i = 0 ; i < NUM_SEPARATOR; i++)
        printf("+");
        
    pthread_mutex_lock(&lockPlayers); 
    printf("\nPartecipanti (%d)\n",current_session.num_players);
    for(int i = 0 ; i < current_session.num_players ; i++) {
        printf("- %s\n", get_player_index(current_session.players,i)->nickname );
    }  
    pthread_mutex_unlock(&lockPlayers);
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce il collegamento di un nuovo client
void* client_handler(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);
      
    //il primo msg che riceve è il nickname
    char nickname[MAXCHAR_NICKNAME];
    
    while(1) {
        get_nickname(client_fd,nickname);
      
        //quando un client si collega stampa le classifiche e chi ha completato i quiz 
        struct Player** rankings = get_theme_rankings(client_fd);
        print_rankings(rankings);
        print_completed_quiz(rankings);
    
        while(get_player(current_session.players,nickname) != NULL) {
          //una volta registrato il nuovo giocatore invia i temi disponibili
          send_themes(client_fd, nickname);
        }
    }
    
    close(client_fd);
    return NULL;
}

//-------------------------------------------------------------------------------------------------------------

void initQuizThemes() {
    //inizializzazione dei temi
    current_session.availableThemes = (char**)malloc(NUM_THEMES * sizeof(Theme));
    if(current_session.availableThemes == NULL) {
        //errore nel malloc 
        perror("errore nel malloc");
        exit(EXIT_FAILURE);
    }
    
    //per ogni tema legge da file il nome del tema, domande e risposte corrette
    for(int i = 0; i < NUM_THEMES; i++) {
        char buf[MAXCHAR_LINE];
        get_theme_name(i,buf);
        
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
    getFilenameFromIndex(fileName,numTheme,current_session);
    
    for(int numPrompt = 0; numPrompt < NUM_Q ; numPrompt++) {
        
        char lineBuf[MAXCHAR_LINE];
        readLine(fileName,lineBuf,numPrompt);
        
        char Qbuf[MAXCHAR_LINE];
        strcpy(Qbuf,lineBuf);
        getQuestionFromLine(Qbuf);
        
        //mette la domanda nell'apposita struttura dati della current_session
        current_session.availableThemes[numTheme].quiz[numPrompt].question = malloc(strlen(Qbuf));
        if(current_session.availableThemes[numTheme].quiz[numPrompt].question == NULL) {
            //errore nel malloc
            perror("errore nel malloc");
            exit(EXIT_FAILURE);
        }
        
        strcpy(availableThemes[numTheme].quiz[numPrompt].question, bufQ);
        
        //mette le risposte nelle apposite strutture dati
        char Abuf[MAXCHAR_LINE];
        strcpy(Abuf,lineBuf); 
        getAnswerFromLine(Abuf);//risposte giuste separate da "|"
        int numA = getNumAnswers(Abuf); //ritorna il numero di risposte giuste presenti
        *current_session.availableThemes[numTheme].quiz[numPrompt].answer = malloc(numA * sizeof(char*));
        if(*current_session.availableThemes[numTheme].quiz[numPrompt].answer == NULL) {
            //errore nel malloc
            perror("errore nel malloc");
            exit(EXIT_FAILURE);
        }
        
        
        
        
        
        
    }//fine for
}

