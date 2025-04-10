#ifndef RANKINGSFUNCTIONS_H
#define RANKINGSFUNCTIONS_H

struct Player** get_theme_rankings();
struct Player** get_final_rankings(struct Player** rankings,struct Player*** score_bins,struct Player*** score_bins_tails ) ;
void print_rankings(struct Player** rankings) ;
void print_completed_quiz(struct Player** rankings) ;
int count_ranked(struct Player** rankings,int theme_index);

#endif
