#ifndef GENERALFUNCTIONS_H
#define GENERALFUNCTIONS_H

#include "../utility/utility.h"
#include "../utility/serverUtility.h"
#include "QAfunctions.h"
#include "rankingsFunctions.h"
#include "PlayersFunctions.h"
#include "gameFunctions.h"

 void init_session();
 void show_overview();
 void* client_handler(void* arg);
 void manageErrSend();
 void manageErrRecv();

#endif
