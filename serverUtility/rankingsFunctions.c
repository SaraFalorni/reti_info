//FUNZIONI CHE CREANO LE CLASSIFICHE DEI GIOCATORI

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "rankingsFunctions.h"

//-------------------------------------------------------------------------------------------------------------

//funzione che restituisce la classifica per tutti i temi come array di liste (classifiche) per tema
//il tema è individuato dall'indice (corrispondente al numero di riga che ha nel documento ./txt/indiceTemi.txt)
//l'uso di questa funzione deve essere fatto all'interno di un blocco critico
struct Player** get_theme_rankings(int client_fd) {
   
    // array di liste (classifiche) per tema, l'indice dell'array identifica il tema
    struct Player** rankings = (struct Player**)malloc(current_session.numThemes * sizeof(struct Player*));
    if(rankings == NULL) {
        //errore nel malloc chiude il thread
        perror("errore nel malloc");
        close(client_fd);
        pthread_exit(NULL);
    }
       
    // per ordinare le classifiche utilizza Counting sort
    //crea un array di NUM_Q+1 elementi i cui indici identificano il punteggio ottenuto nel quiz per quel tema
    //ogni elemento è una lista ai giocatori che hanno ottenuto quel punteggio
    //utilizzo puntatori all'inizio della lista (score_bins) e alla fine della lista (score_bins_tails) per rendere costante il tempo di inserimento in lista
    struct Player*** score_bins = (struct Player***)malloc(current_session.numThemes * sizeof(struct Player**));
    if(score_bins == NULL) {
        //errore nel malloc chiude il thread
        perror("errore nel malloc");
        close(client_fd);
        free(rankings);//libera memoria
        pthread_exit(NULL);
    }
    
    struct Player*** score_bins_tails = (struct Player***)malloc(current_session.numThemes * sizeof(struct Player**));//puntatore all'ultimo elemento, per inserimento ordinato
    if(score_bins_tails == NULL) {
      //errore nel malloc chiude il thread
      perror("errore nel malloc");
      close(client_fd);
      free(rankings);//libera memoria
      free(score_bins);
      pthread_exit(NULL);
    }
    // Inizializzazione degli array di supporto
    for (int i = 0; i < current_session.numThemes; i++) {
        //i è l'indice del tema
        //score_bins[i] è  un array di Player* di NUM_Q+1 elementi 
        score_bins[i] = (struct Player**)malloc((NUM_Q+1) * sizeof(struct Player*));
        if(score_bins[i] == NULL) {
            //errore nel malloc chiude il thread
            perror("errore nel malloc");
            close(client_fd);
            free(rankings);//libera memoria
            free(score_bins);
            free(score_bins_tails);
            pthread_exit(NULL);
        }
        score_bins_tails[i] = (struct Player**)malloc((NUM_Q+1) * sizeof(struct Player*));
        if(score_bins[i] == NULL) {
            //errore nel malloc chiude il thread
            perror("errore nel malloc");
            close(client_fd);
            free(rankings);//libera memoria
            free(score_bins);
            free(score_bins_tails);
            pthread_exit(NULL);
        }

        for (int k = 0; k < (NUM_Q+1); k++) {
            //k è l'indice del punteggio
            //score_bins[i][k] è la lista dei giocatori che hanno totalizzato k punti all'i-esimo quiz
            score_bins[i][k] = NULL;//i-esimo tema k-esimo punteggio
            score_bins_tails[i][k] = NULL;
        }
    }
    
    pthread_mutex_lock(&lockPlayers);
    // Scansione dei giocatori
    struct Player* current_player = current_session.players;
    
    while (current_player != NULL) { //per ogni giocatore
        // Per ogni tema, crea copie del giocatore solo se il punteggio è valido
        //un punteggio è valido se è diverso da -1, perchè indica che il giocatore ha partecipato a quel quiz
        for (int i = 0; i < current_session.numThemes; i++) {
            //i è l'indice del tema    

            // Consideriamo solo i giocatori che hanno giocato a quell quiz, cioè quelli con punteggio diverso da -1
            if (current_player->themePoints[i] != -1) {
              
                //copia il giocatore solo con il punteggio/completamento del relativo tema (i-esimo tema)
                struct Player* player_copy = copy_player(current_player,i,client_fd);
                     
                // Inserisce nel bucket in base al punteggio
                if (score_bins[i][current_player->themePoints[i]] == NULL) {
                    //se è il primo giocatore inserito
                    score_bins[i][current_player->themePoints[i]] = player_copy;
                    score_bins_tails[i][current_player->themePoints[i]] = player_copy;
                } 
                else {
                    //se è già presente un giocatore nella lista del bucket
                    score_bins_tails[i][current_player->themePoints[i]]->next = player_copy;
                    score_bins_tails[i][current_player->themePoints[i]] = player_copy;
                }
            }
      }
      current_player = current_player->next;//giocatore successivo
  }
  
  //ordinamento delle classifiche dagli score_bins a rankings
  //partendo dagli score_bins restituisce un array i cui elementi sono le liste ordinate per punteggio dei giocatori
  get_final_rankings(rankings,score_bins,score_bins_tails );
  
  pthread_mutex_unlock(&lockPlayers);
  
  // Pulizia memoria delle strutture temporanee
  for (int theme = 0; theme < current_session.numThemes; theme++) {
      free(score_bins[theme]);
      free(score_bins_tails[theme]);
  }
  free(score_bins);
  free(score_bins_tails);
 
  return rankings;
}

//-------------------------------------------------------------------------------------------------------------

//funzione ausiliaria di get_theme_rankings() per creare le classifiche
//ordina in una sola lista per tema tutti i giocatori con relativo punteggio in ordine decrescente utilizzando gli score_bins già creati
//ritorna un array di liste ordinate, l'elemento i-esimo dell'array è la classifica dell'i-esimo tema
struct Player** get_final_rankings(struct Player** rankings,struct Player*** score_bins,struct Player*** score_bins_tails ) {
    // Costruisce le liste risultanti per ogni tema
    for (int i = 0; i < current_session.numThemes; i++) { //per ogni tema i 
        rankings[i] = NULL;
        struct Player* ranking_tail = NULL;
       
        // Combina gli score_bins in ordine decrescente 
        for (int points = NUM_Q; points >= 0; points--) {
            if (score_bins[i][points] != NULL) {
                if (rankings[i] == NULL) {//se non c'è ancora niente in classifica 
                    rankings[i] = score_bins[i][points];
                    ranking_tail = score_bins_tails[i][points];
                } 
                else {//se c'è già qualcosa in classifica aggiungo in fondo alla coda
                    ranking_tail->next = score_bins[i][points];
                    ranking_tail = score_bins_tails[i][points];
                }
            }
        }
    }
    return rankings;
}

//-------------------------------------------------------------------------------------------------------------

//funzione che stampa le classifiche di ogni tema a partire dai rankings creati in precedenza
void print_rankings(struct Player** rankings) {
  
    for(int i = 0; i < current_session.numThemes ; i++) {
        struct Player* current_player = rankings[i];
        
        if(current_player != NULL) 
            printf("\nPunteggio tema %d\n",i+1);
        
        while(current_player != NULL) {
            printf("- %s %d\n",current_player->nickname, *current_player->themePoints);
            current_player = current_player->next;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------

//funzione che stampa la lista dei giocatori che hanno completato il quiz per ogni tema a partire dai rankings creati
void print_completed_quiz(struct Player** rankings) {
    bool first;
    for(int i = 0; i < current_session.numThemes ; i++) {
        struct Player* current_player = rankings[i];
        first = true;
            
        while(current_player != NULL && *current_player->themeCompleted == true) {
            if(first) {//stampa il titolo del quiz se è il primo giocatore ad averlo completato 
                printf("\nQuiz Tema %d completato\n",i+1);
                first = false;
            }
            
            printf("- %s\n",current_player->nickname);
            current_player = current_player->next;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------

//conta i giocatori nella lista rankings[theme_index]
int count_ranked(struct Player** rankings,int theme_index) {
    if(rankings == NULL || theme_index < 0) 
       return -1;
      
    if(rankings[theme_index] == NULL)
        return 0;
      
    int num_players = 0;
    struct Player* current_player = rankings[theme_index];
    
    while(current_player != NULL) {
        num_players++;
        current_player = current_player->next;
    }
    return num_players;
}

