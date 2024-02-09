#pragma warning(disable:4189)  
#pragma warning(disable:4100) 
#pragma warning(disable:4102) 
#pragma warning(disable:4101) 
#pragma warning(disable:4702) 
#pragma warning(disable:4201) 

/* Headers included on all files */

#include<fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include<intrin.h> /* MSR support */

/* The following configs apply to whole driver */
#define _MYDEBUG_
//#define DEBUG /* define if debug info will be printed */
#define DRIVER_NAME "[LoopHPCs]" /* define the driver name printed when debugging */
#define DRIVERNAME L"\\Device\\LoopHPCs" /* driver name for windows subsystem */
#define DOSDRIVERNAME L"\\DosDevices\\LoopHPCs" /* driver name for ~DOS~ subsystem */

// bin address
#define BIN_BASE 0x14000000
#define BIN_SIZE 0xA00

#pragma warning(disable : 4995)


/* device extension */
#pragma once

/* additional driver data. This struct should be included on DriverObject */
typedef struct _de
{
	/* ISR handler, used to load/unload it */
	PVOID nmicallbackhandler;
	/* DS and BTS O.S. pages, used to alloc/free it */
	PVOID DS_BASE;
	PVOID BTS_BUFFER;
	//[Q]
	PVOID64 PEB_BUFFER;
	/* APIC pointer, used to set/unset it */
	UINT32* APIC;
} device_ext,*pdevice_ext;

/* generic thread controller */
 void control_thread(int action, int core);
 void unhook_handler();
void hook_handler();

extern UINT64 PID;
extern UINT64 TID;
extern UINT64 CurrentTID;
extern UINT64 process_address;
extern UINT64 process_size;
extern UINT64 lib_MaxAddress;
extern PVOID64  g_pSemaphore; 


#define _PMI_

 /* BTS Core -- while muticore is not supported yet */
#define CORE3 3
#define MAX_PROC 4