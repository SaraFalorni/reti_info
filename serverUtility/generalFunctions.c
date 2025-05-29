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
//funzione che controlla se c'è un posto disponibile in un set già creato altrimenti lo crea
bool assignClientToSet(int client_fd, struct ClientSet* sets, int* numSets) {
    for(int i = 0; i < (*numSets) ; i++) {
        //se c'è un thread già creato con spazio libero il client viene inserito li
        //nel caso che sia il primo set a essere inizializzato devo comunque entrare nell'if successivo
        if(sets[i].numClients < MAX_CLIENT_IN_THREAD) {
            //c'è posto quindi inserisce il client e ritorna true
            sets[i].clients[sets[i].numClients].client_fd = client_fd;
            sets[i].clients[sets[i].numClients].nickname = NULL;
            sets[i].clients[sets[i].numClients].currentTheme = -1;
            sets[i].clients[sets[i].numClients].currentQ = -1;
            sets[i].clients[sets[i].numClients].state = WaitingForNickname;
            sets[i].clients[sets[i].numClients].isResponding = true;//il primo messaggio è il nickname, inviato dal client
            sets[i].FDUpdateNeeded = true; //perchè si è aggiunto un client
            sets[i].numClients++;
            return true;
        }
        //se non c'è posto controlla il set successivo
    }

    //se esce dal for vuol dire che non c'è un set con posti liberi

    //controlla se è possibile crearlo
    if((*numSets) < MAX_CLIENTSETS) {
        //crea nuovo set
        //inizializzazione dei campi
        sets[(*numSets)].numClients = 1;
        
        //inserisce il client come primo client del nuovo set
        sets[*numSets].clients[0].client_fd = client_fd;
        sets[*numSets].clients[0].nickname = NULL;
        sets[*numSets].clients[0].currentTheme = -1;
        sets[*numSets].clients[0].currentQ = -1;
        sets[*numSets].clients[0].state = WaitingForNickname;
        sets[*numSets].clients[0].isResponding = true;//il primo messaggio è il nickname, inviato dal client
        sets[*numSets].FDUpdateNeeded = true; //perchè si è aggiunto un client

        if(pthread_create(&sets[(*numSets)].thread, NULL, clientHandler, &sets[(*numSets)]) != 0) {
            perror("Errore nella creazione del thread");
            close(client_fd);
            return false;
        }


        (*numSets)++;

        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------------------

//elimina n-esimo client dal set
void removeClientFromSet(int n,struct ClientSet* set) {
    
    //libera la memoria del nickname del client da eliminare
    /*if(set->clients[n].nickname != NULL) { cancellare
        free(set->clients[n].nickname);
        set->clients[n].nickname = NULL;
    }*/

    //rimuove il client dal ClientSet e compatta i client restanti
    for(int j = n; j < set->numClients-1; j++) {
        set->clients[j].client_fd = set->clients[j+1].client_fd;
        set->clients[j].currentTheme = set->clients[j+1].currentTheme;
        set->clients[j].currentQ = set->clients[j+1].currentQ;
        set->clients[j].state = set->clients[j+1].state;
        set->clients[j].isResponding = set->clients[j+1].isResponding;
        free(set->clients[j].nickname);

        //copia del nickname
        if(set->clients[j+1].nickname != NULL) {
            set->clients[j].nickname = malloc(strlen(set->clients[j+1].nickname)+1);
            if(set->clients[j].nickname == NULL) {
                //errore nel malloc
                perror("errore nel malloc");
                close(set->clients[j].client_fd);
    
                set->clients[j].client_fd = -1;
                set->numClients--;
                continue;
    
            }
            strcpy(set->clients[j].nickname,set->clients[j+1].nickname);
        }
        else
            set->clients[j].nickname = NULL;
        
        
    }
    set->numClients--;
    set->FDUpdateNeeded = true; //client è stato eliminato, deve aggiornare il set per select
}


//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce il collegamento di un nuovo client
void* clientHandler(void* arg) {
    struct ClientSet *set = (struct ClientSet*)arg;

    fd_set master; //set principale
    fd_set read_fds; //set di lettura

    int fdmax = -1; //numero massimo di descrittori

    FD_ZERO(&master);

    //inizializzazione master con i client presenti nel set
    /*for(int i = 0; i < set->numClients; i++) { cancellare
        int fd = set->clients[i].client_fd;
        FD_SET(fd,&master);
        if(fd > fdmax)
            fdmax = fd;
    }*/

    while(1) {
        printf("altro ciclo di while\n");
        if(set->FDUpdateNeeded == true) {
            //inizializzazione master con i client presenti nel set
            FD_ZERO(&master);
            fdmax = -1;
            for(int i = 0 ; i < set->numClients; i++) {
                int fd = set->clients[i].client_fd;
                FD_SET(fd,&master);
                if(fd > fdmax)
                    fdmax = fd;
            }
            set->FDUpdateNeeded = false; //aggiornamento set fatto
        }
        read_fds = master;
        //timeout
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;// 0.5 secondi

        if(select(fdmax+1,&read_fds, NULL,NULL,&timeout) == -1) {
            perror("errore nel select");
            continue;
        }
        printf("num clients: %d\n", set->numClients); //cancellare
        for(int i = 0 ; i < set->numClients; i++) {
            printf("client[%d] : %d nickname : %s\n", i, set->clients[i].client_fd, set->clients[i].nickname ? set->clients[i].nickname : "NULL"); //cancellare

            int fd = set->clients[i].client_fd;
            //se il client deve inviare qualcosa entra in manageClientGame
            //se il client è in attesa di un messaggio dal server entra comunque in manageClientGame
            //caso 1: aspetta numero e nomi dei temi
            //caso 2: aspetta una domanda del quiz

            if(FD_ISSET(fd, &read_fds) || set->clients[i].isResponding == false ) {

                //gestione del client
                if(manageClientGame(&set->clients[i]) < 0) {
                    //in caso di errore
                    close(fd);
                    FD_CLR(fd,&master); //elimina il client disocnesso dal set di client

                    removeClientFromSet(i,set);//elimina i esimo client dal set
                    i--;

                    //aggiorna fdmax
                    fdmax = -1;
                    for(int j = 0; j < set->numClients; j++) {
                        if(set->clients[j].client_fd > fdmax)
                            fdmax = set->clients[j].client_fd;
                    }
                }
            }

            //se il client è in attesa di un messaggio dal server entra comunque in manageClientGame
            //caso 1: aspetta numero e nomi dei temi cancellare
            //caso 2: aspetta una domanda del quiz
            /*else if ( (set->clients[i].state == WaitingForTheme && set->clients[i].isResponding == false) ||
                       (set->clients[i].state == PlayingQuiz && set->clients[i].isResponding == false ) ) {
                //invio gestito in manageClientGame
                if(manageClientGame(&set->clients[i]) <= 0) {
                    //in caso di errore
                    close(fd);
                    FD_CLR(fd,&master); //elimina il client disocnesso dal set di client

                    removeClientFromSet(i,set);//elimina i esimo client dal set
                    i--;

                    //aggiorna fdmax
                    fdmax = -1;
                    for(int j = 0; j < set->numClients; j++) {
                        if(set->clients[j].client_fd > fdmax)
                            fdmax = set->clients[j].client_fd;
                    }
                }
            }*/
        }
        
    }
    
    return NULL;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce lo stato WaitingForNickname, 
int handleWaitingForNickname(struct ClientInfo* client) {
    //il primo msg che riceve è il nickname
    int res = getNickname(client);
    if( res <= 0)//gestisce la recezione del nickname e registra il nuovo player
        return res; 
    if(client->nickname == NULL) {
        //nel caso in cui getNickanme non sia andato a buon fine client->nickname non è inizializzato e quindi non può procedere con il flusso di gioco
        client->isResponding = true;//il client deve inviare un nuovo nickname
        return 0;
    }
        

    //nickname acquisito, passa allo stato WaitingForTheme
    client->state = WaitingForTheme;
    client->isResponding = false;

    //a questo punto il nuovo player è stato registrato, vengono mostrate al server le classifiche e chi ha completato i quiz
    struct Player** rankings = getThemeRankings(client->client_fd);
    if(rankings == NULL) {
        //gestione errore
        perror("Errore nel calcolo della classifica");
        deletePlayer(client->nickname);
        return -1;//gestito in manageClientGame
    }
    printRankings(rankings);
    printCompletedQuiz(rankings);

    return 1;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce lo stato WaitingForTheme
int handleWaitingForTheme(struct ClientInfo* client) {
    printf("entra in handleWaitingForThemes\n");//cancellare
    if(client->isResponding == false) {
        //se isResponding è false nello stato WaitingForTheme vuol dire che deve ancora ricevere i temi
        int res = sendThemes(client, client->nickname);
        if( res <= 0)
            return res;
        client->isResponding = true;
    }
    else {
        //se isResponding è true nello stato WaitingForTheme vuol dire che ha già ricevuto i temi
        int theme = recvThemes(client, client->nickname);
        //controllo che il tema scelto sia accettabile
        if(theme <= 0) {
            return theme;//errore gestito a livello di clientHandler
        }
        client->currentTheme = theme-1;
        client->currentQ = 0;
        client->isResponding = false;
        client->state = PlayingQuiz;
    }
    return 1;
}
//-------------------------------------------------------------------------------------------------------------
int handlePlayingQuiz(struct ClientInfo* client) {
    if(client->isResponding == false) {
        int res = sendQuestion(client);
        if(res <= 0 )
            return res;
        client->isResponding = true; 
    }
    else {
    //se isResponding è true nello stato PlayingQuiz 
    //deve controllare se è una risposta o un altro comando (show score, endquiz)
        int res = recvCommand(client);
        if( res <= 0)
            return res; 
        client->isResponding = false;

        //controllo se è concluso il quiz o meno
        if(client->currentQ == NUM_Q) {
            //se arriva a questo punto il quiz è stato completato 
            //aggiorna la struttura dati corispondente in current_session
            pthread_mutex_lock(&lockPlayers);

            getPlayer(current_session.players,client->nickname)->themeCompleted[client->currentTheme] = true;

            pthread_mutex_unlock(&lockPlayers);

            client->currentTheme = -1;
            client->currentQ = -1;
            client->state = WaitingForTheme; //il client ha concluso un tema, deve sceglierne un altro tra quelli rimasti
            client->isResponding = false;
        }
    }

    return 1;
}


//-------------------------------------------------------------------------------------------------------------

//funzione che gestisce le comunicazioni con il client
int manageClientGame(struct ClientInfo* client) {
    switch(client->state) {
        case WaitingForNickname:
            int res1 = handleWaitingForNickname(client);
            if( res1 == -1)
                return -1;//gestito in ClientHandler 
            else if(res1 == 0)
                return 0;   
        break;

        case WaitingForTheme:
            int res2 = handleWaitingForTheme(client);
            if(res2 == -1)
                return -1;
            else if(res2 == 0)
                return 0; 
        break;

        case PlayingQuiz:
            int res3 = handlePlayingQuiz(client);
            if(res3 == -1)
                return -1;
            else if(res3 == 0)
                return 0; 
        break;
    }
    return 1;
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
        if(numA == -1) {
            //errore nel malloc all'interno della funzione getNumAnswers
            perror("errore nel malloc");
            exit(EXIT_FAILURE);
        }

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

