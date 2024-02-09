
#include "config.h"
#include "dbg\debug.h"
#include "checks\checks.h"
#include "device\device.h"
#include "simple-pebs.h"
#include "list\list.h"
#include "WriteFile\WriteFile.h"
#include "strsafe.h"


/*************************************************************************
    Prototypes
*************************************************************************/

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

VOID LoopHPCsUnload(PDRIVER_OBJECT  DriverObject);
void test_attach_core1(UINT64 id);
void test_attach_core2(UINT64 id);
extern void thread_attach_to_core(unsigned int id);

NTKERNELAPI NTSTATUS PsSuspendProcess(PEPROCESS pEProcess);



/*************************************************************************
    Driver initialization and unload routines.
*************************************************************************/
PDRIVER_OBJECT drv;
extern 	DWORD64 GetAddress(WCHAR APIName[]);
extern BOOLEAN SuspendThread();

 LONG64 beta=0;
 PCHAR szBuffer_BASE=NULL;
 

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )

{
	int ret_code;
    NTSTATUS status=STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(RegistryPath);



	DriverObject->DriverUnload=LoopHPCsUnload;
	NumberofPEBS=PMCx_check();
	thread_attach_to_core(0);

	ret_code=perform_checks();
	if(ret_code!=NO_ERROR)
	{
		status=STATUS_NOT_SUPPORTED;
		emit_error(ret_code);
	}

	if((status=CreateDevice(DriverObject))!=STATUS_SUCCESS)
	{
		DbgPrint("Error Creating Device");
	}


	debug("Setting driver extension");
	DriverObject->DeviceObject->DeviceExtension=ExAllocatePool(NonPagedPool,sizeof(device_ext));


#ifdef _PMI_
	hook_handler();
#else

	DbgPrint("Registering NMI callback");
	ext=(pdevice_ext)DriverObject->DeviceObject->DeviceExtension;
	ext->nmicallbackhandler=KeRegisterNmiCallback(&NmiCallback,NULL);
#endif

	drv=DriverObject;


	LoopList_Header =NULL;
	/* create list */
	if(create_list(&LoopList_Header)==FALSE)
	{
		DbgPrint("Create LoopList_Header error");
	}

	if(szBuffer_BASE==NULL)
	{
		szBuffer_BASE=(PCHAR) ExAllocatePool(NonPagedPool,szBuffer_SIZE);
		DbgPrint("[DriverEntry] szBuffer_BASE  is %X",szBuffer_BASE);
		memset(szBuffer_BASE, 0,	szBuffer_SIZE);
	}

	debug("Entry Point: Out");
    return status;
}


extern void control_thread(int action, int core);
extern int index;
extern ULONG interval_Time;

VOID LoopHPCsUnload(PDRIVER_OBJECT  DriverObject)
{
	UNICODE_STRING path;
	DbgPrint("LoopHPCs Unload");

	PID=0;


#ifdef _TIME_
	DbgPrint(" interval_Time: %ds,%dms",  interval_Time/1000,  interval_Time%1000);
#endif

#ifdef _PMI_
	DbgPrint( "Unhook PMI handler");
	unhook_handler();
#else
	ext=(pdevice_ext)DriverObject->DeviceObject->DeviceExtension;

		DbgPrint("Unhook NMI callback");
		if(ext->nmicallbackhandler!=NULL)
	{
		KeDeregisterNmiCallback(ext->nmicallbackhandler);
	}
	ExFreePool(ext);

#endif


			/* clear list */
	debug("clear list entries");
	destroy_list();

	debug("Removing Device");

	if(Unicode_ProcessName.Length!=0)
	{	
		RtlFreeUnicodeString(&Unicode_ProcessName);
	}

	RtlInitUnicodeString(&path,DOSDRIVERNAME);
	IoDeleteSymbolicLink(&path);


	if(DriverObject->DeviceObject!=NULL)
	{
		DbgPrint("Delete Device");
		IoDeleteDevice(DriverObject->DeviceObject);
	}

	//clean g_pSemaphore
	if(g_pSemaphore!=NULL)
	{
		ObDereferenceObject(g_pSemaphore);
		g_pSemaphore=NULL;
	}

		ExFreePool(szBuffer_BASE);
		szBuffer_BASE=NULL;

	debug("LoopHPCs Unload");
	return;
}


void test_attach_core1(UINT64 id)
{
	KAFFINITY mask;
	ULONG OldCore;
	ULONG NewCore;


	DbgPrint("\nAttaching to core%d",id);
	mask=(UINT64)1<<id;

	OldCore=KeGetCurrentProcessorNumber ();
	KeSetSystemAffinityThread(mask);
	NewCore=KeGetCurrentProcessorNumber ();

	DbgPrint("Result: \tcore%d-->core%d",OldCore,NewCore);


	OldCore=KeGetCurrentProcessorNumber ();
	KeRevertToUserAffinityThread();
	NewCore=KeGetCurrentProcessorNumber ();
	DbgPrint("Revert: \tcore%d-->core%d",OldCore,NewCore);
}

typedef NTSTATUS (__stdcall * pFnKeSetAffinityThread)(PKTHREAD Thread, KAFFINITY Affinity); 
pFnKeSetAffinityThread KeSetAffinityThread = NULL;
    
void test_attach_core2(UINT64 id)
{
	CCHAR OldCurrentProcessor;
	CCHAR NewCurrentProcessor;
	KAFFINITY OrigAffinity;
	KAFFINITY	mask;
	UNICODE_STRING NameString;
	DbgPrint("\nAttaching to core%d",id);

	mask=(UINT64)1<<id;

	if(KeSetAffinityThread==NULL)
	{
	   RtlInitUnicodeString(&NameString, L"KeSetAffinityThread");
		KeSetAffinityThread = (pFnKeSetAffinityThread)MmGetSystemRoutineAddress(&NameString);
	}

	 OldCurrentProcessor = (CCHAR)KeGetCurrentProcessorNumber();
	OrigAffinity = KeSetAffinityThread(KeGetCurrentThread(), mask); 
	 NewCurrentProcessor = (CCHAR)KeGetCurrentProcessorNumber();
	 DbgPrint("Result: \tcore%d-->core%d",OldCurrentProcessor,NewCurrentProcessor);

	  OldCurrentProcessor = (CCHAR)KeGetCurrentProcessorNumber();
	 KeRevertToUserAffinityThread();
	  NewCurrentProcessor = (CCHAR)KeGetCurrentProcessorNumber();
	   DbgPrint("Revert: \tcore%d-->core%d", id,OldCurrentProcessor,NewCurrentProcessor);
}