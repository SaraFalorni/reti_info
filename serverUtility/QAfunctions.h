#ifndef QAFUNCTIONS_H
#define QAFUNCTIONS_H

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "generalFunctions.h"
#include "PlayersFunctions.h"
#include "rankingsFunctions.h"
#include "gameFunctions.h"

void readLine(const char* filename,char* buf, int line);
void getQuestionFromLine(char* buf);
void getAnswerFromLine(char *buf);
int getNumAnswers(char *bufA);
bool checkAnswer(int theme, char* answer, int numQ);
void getFilenameFromIndex(char* bufFile, int themeChosen,struct Session* current_session);

#endif
