//FUNZIONI CHE AGISCONO SU STRUCT PLAYERS SIA DI MODIFICA CHE DI SOLA LETTURA

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "PlayersFunctions.h"
//-------------------------------------------------------------------------------------------------------------

//funzione che ritorna il puntatore dove inserire un nuovo giocatore (in fondo alla lista p)
//se usata su current_session.players deve essere usata in un blocco critico
struct Player* find_last_player(struct Player* p) {
    if(p == NULL || p->next == NULL)
        return p;
    return find_last_player(p->next);
}

//-------------------------------------------------------------------------------------------------------------

//funzione che inserisce un nuovo giocatore con il nickname passato come parametro
//ritorna true se l'inserimento va a buon fine, false altrimenti
bool insert_player(char* nickname, int client_fd) {
    //inserimento in players con mutex per evitare errori
    pthread_mutex_lock(&lockPlayers);
    
    struct Player* new_player;
    new_player = current_session.players;

    //se esiste un giocatore con lo stesso nickname torna false
    if(get_player(current_session.players,nickname) != NULL) {
        pthread_mutex_unlock(&lockPlayers);
        return false;
    }
    
    //altrimenti crea un nuovo giocatore
    new_player = (struct Player*)malloc(sizeof(struct Player));
    if(new_player == NULL) {
        //errore nel malloc chiude il thread
        perror("errore nel malloc");
        close(client_fd);
        pthread_exit(NULL);
    }
    size_t len_nickname = strlen(nickname)+1;
    new_player->nickname = (char*)malloc(len_nickname);
    if(new_player->nickname == NULL) {
        //errore nel malloc chiude il thread
        perror("errore nel malloc");
        free(new_player);//libera la memoria
        close(client_fd);
        pthread_exit(NULL);
    }
    memcpy(new_player->nickname,nickname,len_nickname);  
    
    //INIZIALIZZAZIONE DEI TEMI CON POINTS = -1 e completed con false
    
    new_player->themePoints = malloc(current_session.num_themes * sizeof(int));
    if(new_player->themePoints == NULL) {
        //errore nel malloc chiude il thread
        perror("errore nel malloc");
        free(new_player->nickname);//libera la memoria
        free(new_player);
        close(client_fd);
        pthread_exit(NULL);
    }
    
    new_player->themeCompleted = malloc(current_session.num_themes * sizeof(bool));
    if(new_player->themeCompleted == NULL) {
        //errore nel malloc chiude il thread
        perror("errore nel malloc");
        free(new_player->themePoints);//libera la memoria
        free(new_player->nickname);
        free(new_player);
        close(client_fd);
        pthread_exit(NULL);
    }
    for(int i = 0; i < current_session.num_themes ; i++){
        new_player->themePoints[i] = -1;
        new_player->themeCompleted[i] = false;
    }
    new_player->next = NULL;
    current_session.num_players++;
    
    //se è il primo giocatore a registrarsi
    if(current_session.players == NULL)
        current_session.players = new_player;
    else { //altrimenti viene aggiunto in fondo alla coda
        struct Player* last_player = find_last_player(current_session.players);
        last_player->next = new_player;
    }
      
    //sblocco il mutex
    pthread_mutex_unlock(&lockPlayers);

    return true; //inserimento avvenuto con successo
}

//-------------------------------------------------------------------------------------------------------------

//dato il nickname ritorna un puntatore al giocatore corrispondente nella lista current_player , NULL se il nickname non è presente
//l'uso di questa funzione deve essere fatto all'interno di un blocco critico
struct Player* get_player(struct Player* current_player,char* nickname) {
    if(current_player == NULL)
        return NULL;
      
    if(strcmp(current_player->nickname, nickname) == 0)
        return current_player;
      
    return get_player(current_player->next, nickname);
}

//-------------------------------------------------------------------------------------------------------------

//dato un intero ritorna un puntatore al giocatore in quella posizione nella lista current_player, NULL se non esiste
//l'uso di questa funzione deve essere fatto all'interno di un blocco critico
struct Player* get_player_index(struct Player* current_player,int index) {
    if(current_player == NULL || index < 0)
        return NULL;
      
    if(index == 0)
        return current_player;
      
    return get_player_index(current_player->next, index-1);
}

//-------------------------------------------------------------------------------------------------------------

//funzione ausiliaria di get_theme_rankings() per creare le classifiche 
//copia un giocatore (current_player) con solo il punteggio relativo a un tema ( identificato da index_theme)
//ritorna un puntatore alla copia
struct Player* copy_player(struct Player* current_player,int index_theme,int client_fd) {
    // Crea una copia del giocatore
    struct Player* player_copy = (struct Player*)malloc(sizeof(struct Player));
    if(player_copy == NULL) {
        //errore nel malloc chiude il thread
        perror("errore nel malloc");
        close(client_fd);
        pthread_exit(NULL);
    }
   
    player_copy->nickname = malloc(strlen(current_player->nickname));
    if(player_copy->nickname == NULL) {
        //errore nel malloc chiude il thread
        perror("errore nel malloc");
        close(client_fd);
        free(player_copy);
        pthread_exit(NULL);
    }
    strcpy(player_copy->nickname,current_player->nickname);

    //basta memorizzare il punteggio relativo a solo quel tema
    player_copy->themePoints = malloc(sizeof(int));
    if(player_copy->themePoints == NULL) {
        //errore nel malloc chiude il thread
        perror("errore nel malloc");
        close(client_fd);
        free(player_copy->nickname);
        free(player_copy);
        pthread_exit(NULL);
    }
    
    player_copy->themePoints[0] = current_player->themePoints[index_theme];
    
    //memorizza anche se il tema è stato completato o meno (serve per la funzione print_completed_quiz
    //basta memorizzare il punteggio relativo a solo quel tema
    player_copy->themeCompleted = malloc(sizeof(bool));
    if(player_copy->themeCompleted == NULL) {
        //errore nel malloc chiude il thread
        perror("errore nel malloc");
        close(client_fd);
        free(player_copy->nickname);
        free(player_copy->themePoints);
        free(player_copy);
        pthread_exit(NULL);
    }
    player_copy->themeCompleted[0] = current_player->themeCompleted[index_theme];
    
    player_copy->next = NULL;
    return player_copy;
}

//-------------------------------------------------------------------------------------------------------------

//elimina il giocatore individuato da nickname dalla struttura dati current_session.players
void delete_player(char* nickname) {
    //elimina il player corrispondente da current_session
    
    pthread_mutex_lock(&lockPlayers);
    struct Player* current_player = current_session.players;
    struct Player* prec_player = NULL;

    while(current_player != NULL) {
        if(strcmp(nickname, current_player->nickname) == 0) {
            if(prec_player == NULL) {
                //se è il primo della lista
                current_session.players = current_player->next;
            }
            else {
                prec_player->next = current_player->next;
            }
            
            current_session.num_players--;
            
            //libera la memoria allocata
            free(current_player->nickname);
            free(current_player->themePoints);
            free(current_player->themeCompleted);
            free(current_player);
            
            break;
        }
        prec_player = current_player;
        current_player = current_player->next;
    }
    pthread_mutex_unlock(&lockPlayers);
}
