#define PS_CROSS_THREAD_FLAGS_SYSTEM 0x00000010UL

typedef enum _KAPC_ENVIRONMENT {
	OriginalApcEnvironment,
	AttachedApcEnvironment,
	CurrentApcEnvironment,
	InsertApcEnvironment
} KAPC_ENVIRONMENT;

void KeInitializeApc (
	PKAPC Apc,
	PETHREAD Thread,
	KAPC_ENVIRONMENT Environment,
	PKKERNEL_ROUTINE KernelRoutine,
	PKRUNDOWN_ROUTINE RundownRoutine,
	PKNORMAL_ROUTINE NormalRoutine,
	KPROCESSOR_MODE ProcessorMode,
	PVOID NormalContext
	);

BOOLEAN KeInsertQueueApc(PKAPC Apc,PVOID SystemArg1,PVOID SystemArg2,KPRIORITY Increment);

VOID KernelKillThreadRoutine(IN PKAPC Apc,
							 IN OUT PKNORMAL_ROUTINE *NormalRoutine,
							 IN OUT PVOID *NormalContext,
							 IN OUT PVOID *SystemArgument1,
							 IN OUT PVOID *SystemArgument2)
{

	PULONG ThreadFlags;
	ExFreePool(Apc); 
	ThreadFlags=(ULONG *)((ULONG)PsGetCurrentThread()+0x248); 
	if(MmIsAddressValid(ThreadFlags)) 
	{
		*ThreadFlags=(*ThreadFlags) | PS_CROSS_THREAD_FLAGS_SYSTEM; 
		PsTerminateSystemThread(STATUS_SUCCESS);
	}
}


VOID KillProcessWithApc(ULONG epro)
{

	BOOLEAN status;
	PKAPC ExitApc=NULL;
	PEPROCESS eprocess;
	PETHREAD ethread;
	ULONG i;
	ULONG num; 
	ULONG Head; 
	ULONG address;
	num=*(ULONG *)(epro+0x1a0);
	KdPrint(("[RecordThreadAddress] num: 0x%x\n",num)); 
	Head=epro+0x190; 
	for(i=0;i<num;i++)
	{
	
		Head=(ULONG)((PLIST_ENTRY)Head)->Flink;
		address=Head-0x22c;
		KdPrint(("[RecordThreadAddress] address: 0x%x\n",address)); 
		ethread=(PETHREAD)address; 
		ExitApc=(PKAPC)ExAllocatePoolWithTag(NonPagedPool,sizeof(KAPC),MEM_TAG);
		if(ExitApc==NULL)
		{
			KdPrint(("[KillProcessWithApc] malloc memory failed \n"));
			return;
		}
		KeInitializeApc(ExitApc,
			ethread,
			OriginalApcEnvironment,
			KernelKillThreadRoutine,
			NULL,
			NULL,
			KernelMode,
			NULL);
		status=KeInsertQueueApc(ExitApc,ExitApc,NULL,2);
		if(status==STATUS_SUCCESS)
			KdPrint(("KeInsertQueueApc success\n"));
		else
			KdPrint(("KeInsertQueueApc failed\n"));
	}
}


typedef NTSTATUS(__fastcall *ZWTERMINATETHREAD)(HANDLE hThread, ULONG uExitCode);
ZWTERMINATETHREAD ZwTerminateThread = 0Xfffff80012345678; 

void ZwKillThread()
{
	HANDLE hThread = NULL;
	CLIENT_ID ClientId;
	OBJECT_ATTRIBUTES oa;

	ClientId.UniqueProcess = 0;
	ClientId.UniqueThread = (HANDLE)1234; 

	oa.Length = sizeof(oa);
	oa.RootDirectory = 0;
	oa.ObjectName = 0;
	oa.Attributes = 0;
	oa.SecurityDescriptor = 0;
	oa.SecurityQualityOfService = 0;

	ZwOpenProcess(&hThread, 1, &oa, &ClientId);
	if (hThread)
	{
		ZwTerminateThread(hThread, 0);
		ZwClose(hThread);
	};
}
