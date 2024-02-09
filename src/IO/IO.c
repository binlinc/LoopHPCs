

#include "IO.h"
#include "dbg\debug.h"
#include "Threading\thread.h"
#include "list\list.h"
#include "simple-pebs.h"
#include "LBR\LBR.h"
#include "ioctl.h"	
#include "WriteFile\WriteFile.h"
#include<stdlib.h>

ANSI_STRING Ansi_String;

char szProcessName[MAX_PATH]={0};
char szLogFileName [MAX_PATH]="tmp.csv";
int iMLPX=1;

NTSTATUS Read(PDEVICE_OBJECT DeviceObject,PIRP Irp)
{
	PLOOPInfo bdata;
	PVOID userbuffer;
	PIO_STACK_LOCATION PIO_STACK_IRP;
	UINT32 datasize,sizerequired;
	NTSTATUS NtStatus=STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(DeviceObject);
	NtStatus=STATUS_SUCCESS;

#ifdef _MYDEBUG_
	DbgPrint("Dispatch Fucntion to Read");
#endif

	/* remove an item from list */
	if(Remove_LoopList(LoopList_Header)==TRUE)
	{

		/* if data is available, copy to userland */
		userbuffer=Irp->AssociatedIrp.SystemBuffer;
		PIO_STACK_IRP=IoGetCurrentIrpStackLocation(Irp);
		if(PIO_STACK_IRP && userbuffer)
		{
			datasize=PIO_STACK_IRP->Parameters.Read.Length;
			sizerequired=sizeof(LOOPInfo);
			if(datasize>=sizerequired){
				memcpy(userbuffer,&bdata,sizerequired);
				Irp->IoStatus.Status = NtStatus;
				Irp->IoStatus.Information = sizerequired;
			}else{
				Irp->IoStatus.Status = NtStatus;
				Irp->IoStatus.Information = 0;
			}
		}else{
			Irp->IoStatus.Status = NtStatus;
			Irp->IoStatus.Information = 0;
		}
	}else{
		/* if not available, no data is written */
		Irp->IoStatus.Status = NtStatus;
		Irp->IoStatus.Information = 0;
	}

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return NtStatus;

}


/* Create File -- start capture mechanism */
NTSTATUS Create(PDEVICE_OBJECT DeviceObject,PIRP Irp)
{
	int i,j;
	NTSTATUS status;
	UNREFERENCED_PARAMETER(DeviceObject);

	DbgPrint("Dispatch Fucntion to IRP_MJ_CREATE");

	/* I don't know if launching threads inside an I/O routine is OK
	* Anyway, an IOCTL would be more suitable
	* The idea here is Create/CLose work as Start/Stop
	*/
	IndexofPMI=0;
	IndexofMyCode=0;
	IndexofLBR=0;
	previous_LBR_TOS=INVALID_TOS_VALUE;

	IndexofOuterLoop=0;
	IndexofInnerLoop=0;
	TotalInnerLoop=0;
	IndexofPeriod=0;

	previous_INST=0;
	previous_CYCLE=0;
	previous_PMC0=0;
	previous_PMC1=0;
	previous_PMC2=0;
	previous_PMC3=0;
	previous_PMC4=0;
	previous_PMC5=0;
	previous_PMC6=0;
	previous_PMC7=0;

	lastLoop=NULL;
	DLAListHeader=NULL;


	Multiplex_Index=0;

	if(DLAListHeader==NULL)
	{
		DLAListHeader=(PLIST_ENTRY)ExAllocatePool(NonPagedPool,sizeof(PLIST_ENTRY));
		InitializeListHead(DLAListHeader);
	}

	iOld_PERFCTR=0;

	bIOCTL_SEND_PNAME=FALSE;
	bIOCTL_SEND_LOGFILENAME=FALSE;
	bPrint_pebs_index=TRUE ;
	IndexofClose=0;

	for( i=0;i<MAX_PMCx;i++)
	{
		for( j=0;j<4;j++)
		{
			MyDbgPrint ("staticMask[%d][%d]=%d",i, j,StaticMask[Multiplex_Index][i][j]); 
		}
	}

	//for( i=3;i<MAX_PROC;i++)
	//{
		control_thread(LOAD_BTS,Target_Core_Number);
//	}

	Irp->IoStatus.Status=STATUS_SUCCESS;
	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	status = STATUS_SUCCESS;
	return status;
}


/* CLoseFile/CloseHandle -- stop routine */
NTSTATUS Close(PDEVICE_OBJECT DeviceObject,PIRP Irp)
{
//	 int i;
	NTSTATUS status;

	PLIST_ENTRY	removed_DLA=NULL;
	PDLA 	removed_DLA_Content=NULL;

	UNREFERENCED_PARAMETER(DeviceObject);

	//Waiting for Creat IO finished,
	delay_exec(DELAY_INTERVAL*5);
	DbgPrint("Dispatch Fucntion to IRP_MJ_CLOSE");

	IndexofClose++;



		control_thread(UNLOAD_BTS,Target_Core_Number);

		if(DLAListHeader!=NULL)
		{
			while(!IsListEmpty(DLAListHeader))
			{
				removed_DLA=RemoveHeadList(DLAListHeader);
				removed_DLA_Content=CONTAINING_RECORD(removed_DLA,DLA,DataLAList);
				ExFreePool(removed_DLA_Content);	
			}			
			ExFreePool(DLAListHeader);
		}


	Irp->IoStatus.Status=STATUS_SUCCESS;
	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	status = STATUS_SUCCESS;
	return status;
}


//DispatchIoctl
UINT64 PID=0;
UINT64 TID=0;
UINT64 CurrentTID=0;
UINT64 process_address=0;
UINT64 process_size=0;
UINT64 lib_MaxAddress=0;
PVOID64  g_pSemaphore=NULL;
char tmp[256];
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)


NTSTATUS DispatchIoctl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION pIrpStack;
	ULONG uIoControlCode;
	PVOID pIoBuffer;

	ULONG uOutSize=0;
	HANDLE SemaphoreHandle=INVALID_HANDLE_VALUE;


	memset(tmp, 0, sizeof(tmp));

	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;


	pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;


	switch(uIoControlCode)
	{
		case IOCTL_SEND_PNAME:
		{
			strcpy(tmp,pIoBuffer);
#ifdef _MYDEBUG_ 
			//DbgPrint("case IOCTL_SEND_PNAME");
			DbgPrint("[IO.c] case IOCTL_SEND_PNAME, process name= %s",tmp);
#endif
			strcpy(szProcessName,tmp);
			DbgPrint("[IO.c] case IOCTL_SEND_PNAME, szProcessName = %s",szProcessName);

			RtlInitAnsiString(&Ansi_String,tmp);
			MyDbgPrint("[IO.c] Ansi_String=%Z",Ansi_String);

			status =RtlAnsiStringToUnicodeString(&Unicode_ProcessName,&Ansi_String,TRUE);
			MyDbgPrint("[IO.c] Unicode_ProcessName=%wZ",Unicode_ProcessName);
			status = STATUS_SUCCESS;
			bIOCTL_SEND_PNAME=TRUE;
			DbgPrint("[IO.c] case IOCTL_SEND_PNAME, end");
			break;
		}

		case IOCTL_SEND_LOGFILENAME:
		{
			strcpy(tmp,pIoBuffer);
#ifdef _MYDEBUG_ 
			DbgPrint("[IO.c] case IOCTL_SEND_LOGFILENAME, logFileName= %s",tmp);
#endif
			strcpy(szLogFileName,tmp);
			DbgPrint("[IO.c] case IOCTL_SEND_LOGFILENAME, szLogFileName= %s",szLogFileName);
			bIOCTL_SEND_LOGFILENAME=TRUE;
			break;
		}
		
		case IOCTL_SEND_MLPX:
		{
			strcpy(tmp,pIoBuffer);
			iMLPX=atoi(tmp);
#ifdef _MYDEBUG_ 
			DbgPrint("[case IOCTL_SEND_MLPX:] MLPX is %d\n",iMLPX);
#endif
			break;
		}
		//IOCTL_SEND_PID
	case IOCTL_SEND_PID:
		{
			strcpy(tmp,pIoBuffer);
			PID=atoi(tmp);
#ifdef _MYDEBUG_ 
			DbgPrint("case IOCTL_SEND_PID, process id is %X\n",PID);
#endif
			status = STATUS_SUCCESS;
			break;
		}
	case IOCTL_SEND_TID:
		{
			strcpy(tmp,pIoBuffer);
			TID=atoi(tmp);
#ifdef _MYDEBUG_ 
			DbgPrint("case IOCTL_SEND_TID, thread id is %X\n",TID);
#endif
			status = STATUS_SUCCESS;
			break;
		}
			case IOCTL_SEND_CurrentTID:
		{
			strcpy(tmp,pIoBuffer);
			CurrentTID=atoi(tmp);
#ifdef _MYDEBUG_ 
			DbgPrint("[TIME] case IOCTL_SEND_CurrentTID, thread id is %X\n",CurrentTID);
#endif
			status = STATUS_SUCCESS;
			break;
		}

	case IOCTL_SEND_ADDR:
		{
			strcpy(tmp,pIoBuffer);
			process_address=atoi(tmp);
#ifdef _MYDEBUG_ 
			DbgPrint("case IOCTL_SEND_ADDR, process_address is %X\n",process_address);
#endif
			status = STATUS_SUCCESS;
			break;
		}

	case IOCTL_SEND_SIZE:
		{
			strcpy(tmp,pIoBuffer);
			process_size=atoi(tmp);
#ifdef _MYDEBUG_ 
			DbgPrint("case IOCTL_SEND_SIZE, process_size is %X\n",process_size);
#endif
			status = STATUS_SUCCESS;
			break;
		}

		case IOCTL_SEND_LibsAddrMax:
		{
			strcpy(tmp,pIoBuffer);
			lib_MaxAddress=atoi(tmp);
#ifdef _MYDEBUG_ 
			DbgPrint("case IOCTL_SEND_LibsAddrMax, lib_MaxAddress is %X\n",lib_MaxAddress);
#endif
			status = STATUS_SUCCESS;
			break;
		}
		
	case IOCTL_SET_Semp:
		{
			SemaphoreHandle = *(HANDLE *)pIoBuffer;
#ifdef _MYDEBUG_ 
			DbgPrint("case SET_Semp, SemaphoreHandle is %d\n",SemaphoreHandle);
#endif

			/* Get the Object */
			status = ObReferenceObjectByHandle(SemaphoreHandle,SEMAPHORE_MODIFY_STATE, *ExSemaphoreObjectType, KernelMode,(PVOID*)&g_pSemaphore, NULL);
			//status = STATUS_SUCCESS;
			break;
		}

	default:
		break;

	}

	if(status == STATUS_SUCCESS)
		pIrp->IoStatus.Information = uOutSize;
	else
		pIrp->IoStatus.Information = 0;

	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}



/* generic routine to support non-implemented I/O */
NTSTATUS NotSupported(PDEVICE_OBJECT DeviceObject,PIRP Irp)
{
	NTSTATUS status;
	UNREFERENCED_PARAMETER(DeviceObject);
	debug("Not Supported I/O operation");
	Irp->IoStatus.Status=STATUS_NOT_SUPPORTED;
	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	status = STATUS_NOT_SUPPORTED;
	return status;
}