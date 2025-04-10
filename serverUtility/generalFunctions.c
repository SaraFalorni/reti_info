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
    
    //recupera il numero di temi dal file ./txt/indiceTemi.txt
    current_session.num_themes = quanti_temi();
    
    current_session.availableThemes = (char**)malloc(current_session.num_themes * sizeof(char*));
    
    for(int i = 0; i < current_session.num_themes; i++) {
        char buf[MAXCHAR_LINE];
        get_theme_name(i+1,buf);
        //copia il nome dei temi nella corrispondente struttura dati 
        size_t len_themeName = strlen(buf)+1;
        current_session.availableThemes[i] = malloc(len_themeName);
        memcpy(current_session.availableThemes[i],buf,len_themeName);
    }
    
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
    get_nickname(client_fd,nickname);
    
    //quando un client si collega stampa le classifiche e chi ha completato i quiz 
    struct Player** rankings = get_theme_rankings();
    print_rankings(rankings);
    print_completed_quiz(rankings);
    
    while(1) {
        //una volta registrato il nuovo giocatore invia i temi disponibili
        send_themes(client_fd, nickname);
    }
    close(client_fd);
}
