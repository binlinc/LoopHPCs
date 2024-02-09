

#include "config.h"




/* Debug messages prototype */

void debug(char msg[]);
void emit_error(int code);
void GetCurrentTimeString();

//#define _DEBUG_


#ifdef _DEBUG_
	#define MyDbgPrint DbgPrint
#else
#define MyDbgPrint
#endif


#define _PEB_
#define _LBR_

 BOOLEAN  bPrint_pebs_index;
 int IndexofClose;