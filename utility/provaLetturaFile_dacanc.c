#include <stdio.h>
 
int main() {
    FILE *f = fopen("../txt/indiceTemi.txt", "r");

    if(!f) {
        perror("Errore nell'apertura del file");
        return 1;
    }

int current_line = 0;

    char buffer[25];
while(fgets(buffer,sizeof(buffer),f)) {
    if(++current_line == 6) {
        fclose(f);    
        break;
    }
    printf("tema: %s",buffer);
    current_line++;
}
    
    fclose(f);
    return 0;
}