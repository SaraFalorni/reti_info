#ifndef RANKINGSFUNCTIONS_H
#define RANKINGSFUNCTIONS_H

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "QAfunctions.h"
#include "generalFunctions.h"
#include "PlayersFunctions.h"
#include "gameFunctions.h"

struct Player** getThemeRankings(int client_fd);
struct Player** getFinalRankings(struct Player** rankings,struct Player*** score_bins,struct Player*** score_bins_tails ) ;
void printRankings(struct Player** rankings) ;
void printCompletedQuiz(struct Player** rankings) ;
int countRanked(struct Player** rankings,int theme_index);

#endif
