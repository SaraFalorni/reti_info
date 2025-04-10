#ifndef QAFUNCTIONS_H
#define QAFUNCTIONS_H

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "generalFunctions.h"
#include "PlayersFunctions.h"
#include "rankingsFunctions.h"
#include "gameFunctions.h"

void read_line(const char* filename,char* buf, int line);
void read_q(const char* filename,char* buf, int numQ);
void get_answ_from_line(char *buf);
bool check_answer(const char *theme, char* answer, int numQ);
void get_theme_name(int num, char* buf);
int quanti_temi();
void get_filename_from_index(char* bufFile, int themeChosen,struct Session* current_session); 

#endif
