#include "config.h"

#define DELAY_ONE_MICROSECOND (-10)
#define DELAY_ONE_MILLISECOND (DELAY_ONE_MICROSECOND*1000)
#define DELAY_ONE_SECOND (DELAY_ONE_MILLISECOND*1000)

#define DELAY_INTERVAL DELAY_ONE_MILLISECOND*100
 void delay_exec(INT64 interval);



enum thread_action
{
	LOAD_BTS,
	UNLOAD_BTS
};

void control_thread(int action, int core);
VOID ThreadStart(_In_ PVOID StartContext);
VOID ThreadStop(_In_ PVOID StartContext);

void thread_attach_to_core(unsigned int id);