#include "utility.c"
 
int main() {

    char *answ = "soPRAno";
    bool giusta = check_answer("../txt/serieTV.txt",answ,5);
    if(giusta == true) {
        printf("risposta %s è giusta\n", answ);
    }
    else
        printf("risposta %s è sbagliata\n", answ);

    return 0;
}