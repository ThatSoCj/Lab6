/*.........................................................................*/
/*                  TSHTEST.H ------> TSH test program                     */
/*                  February '13, Oct '18 updated by Justin Y. Shi         */
/*.........................................................................*/

#include "synergy.h"

extern char login[NAME_LEN];

void OpPut(/*void*/) ;
void OpGet(/*void*/) ;
void OpExit(/*void*/) ;
void OpShell(/*void*/) ;

extern int tshsock ;

int connectTsh(u_short) ;
u_short drawMenu(/*void*/) ;

