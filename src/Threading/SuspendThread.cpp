




#ifdef __cplusplus   
extern "C" {   
#endif   
#include <ntddk.h>   
#include <string.h>   
#include <wdm.h>   
#ifdef __cplusplus   
}; // extern "C"   
#endif   

#include "SuspendThread.h"   

#ifdef __cplusplus   
namespace { // anonymous namespace to limit the scope of this global variable!   
#endif   
	PDRIVER_OBJECT pdoGlobalDrvObj = 0;   
#ifdef __cplusplus   
}; // anonymous namespace   
#endif   
PSRVTABLE ServiceTable;   

NTSTATUS SUSPENDTHREAD_DispatchCreateClose(   
	IN PDEVICE_OBJECT       DeviceObject,   
	IN PIRP                 Irp   
	)   
{   
	NTSTATUS status = STATUS_SUCCESS;   
	Irp->IoStatus.Status = status;   
	Irp->IoStatus.Information = 0;   
	IoCompleteRequest(Irp, IO_NO_INCREMENT);   
	return status;   
}   

NTSTATUS SUSPENDTHREAD_DispatchDeviceControl(   
	IN PDEVICE_OBJECT       DeviceObject,   
	IN PIRP                 Irp   
	)   
{   
	NTSTATUS status = STATUS_SUCCESS;   
	PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);   

	switch(irpSp->Parameters.DeviceIoControl.IoControlCode)   
	{   
	case IOCTL_SUSPENDTHREAD_OPERATION:   
		// status = SomeHandlerFunction(irpSp);   
		break;   
	default:   
		Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;   
		Irp->IoStatus.Information = 0;   
		break;   
	}   

	status = Irp->IoStatus.Status;   
	IoCompleteRequest(Irp, IO_NO_INCREMENT);   
	return status;   
}   

VOID SUSPENDTHREAD_DriverUnload(   
	IN PDRIVER_OBJECT       DriverObject   
	)   
{   
	PDEVICE_OBJECT pdoNextDeviceObj = pdoGlobalDrvObj->DeviceObject;   
	IoDeleteSymbolicLink(&usSymlinkName);   

	// Delete all the device objects   
	while(pdoNextDeviceObj)   
	{   
		PDEVICE_OBJECT pdoThisDeviceObj = pdoNextDeviceObj;   
		pdoNextDeviceObj = pdoThisDeviceObj->NextDevice;   
		IoDeleteDevice(pdoThisDeviceObj);   
	}   
}   

#ifdef __cplusplus   
extern "C" {   
#endif   
	NTSTATUS DriverEntry(   
		IN OUT PDRIVER_OBJECT   DriverObject,   
		IN PUNICODE_STRING      RegistryPath   
		)   
	{   
		PDEVICE_OBJECT pdoDeviceObj = 0;   
		NTSTATUS status = STATUS_UNSUCCESSFUL;   
		pdoGlobalDrvObj = DriverObject;   

		// Create the device object.   
		if(!NT_SUCCESS(status = IoCreateDevice(   
			DriverObject,   
			0,   
			&usDeviceName,   
			FILE_DEVICE_UNKNOWN,   
			FILE_DEVICE_SECURE_OPEN,   
			FALSE,   
			&pdoDeviceObj   
			)))   
		{   
			// Bail out (implicitly forces the driver to unload).   
			return status;   
		};   

		// Now create the respective symbolic link object   
		if(!NT_SUCCESS(status = IoCreateSymbolicLink(   
			&usSymlinkName,   
			&usDeviceName   
			)))   
		{   
			IoDeleteDevice(pdoDeviceObj);   
			return status;   
		}   

		// NOTE: You need not provide your own implementation for any major function that   
		//       you do not want to handle. I have seen code using DDKWizard that left the   
		//       *empty* dispatch routines intact. This is not necessary at all!   
		DriverObject->MajorFunction[IRP_MJ_CREATE] =   
			DriverObject->MajorFunction[IRP_MJ_CLOSE] = SUSPENDTHREAD_DispatchCreateClose;   
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SUSPENDTHREAD_DispatchDeviceControl;   
		DriverObject->DriverUnload = SUSPENDTHREAD_DriverUnload;   

		//===================================================================   
		PETHREAD hEThread = 0;   
		ULONG PsSuspnedThread = GetFncAddr(367);//NtSuspendThread Index(WIN7)?????NtSuspendThread ??SSDT????��?HOOK   
		//PsSuspnedThread = GetNt_OldAddr(L"NtSuspnedThread");//??????????????????????   
		KdPrint(("pNtSuspnedThread addr:0x%X",PsSuspnedThread));   
		if (PsSuspnedThread == 0)   
		{   
			KdPrint(("?NtSuspnedThread??????????"));   
			return STATUS_SUCCESS;   
		}   
		PsSuspnedThread = SearchPsSuspendThreadAddr(PsSuspnedThread + 70);//SearchPsSuspendThreadAddr???????????????WIN7?��? ??????XP?????+ 70 ???????????????????????   
		if (PsSuspnedThread == 0)   
		{   
			KdPrint(("?PsSuspnedThread??????????"));   
			return STATUS_SUCCESS;   
		}   
		KdPrint(("PsSuspnedThread ?????????0x%X",PsSuspnedThread));   
		PSSUSPENDTHREAD FncPsSuspendThread = (PSSUSPENDTHREAD)PsSuspnedThread;   
		PsLookupThreadByThreadId((HANDLE)212,&hEThread);//212?????ID????????XT??ARK???????????????????System?��???????QQProtect.sys   
		if (hEThread != 0)   
		{   
			KdPrint(("QQ.sys EThread:%x",hEThread));   
			if (FncPsSuspendThread(hEThread,NULL) != 0)   
			{   
				KdPrint(("PsSuspendThread ???"));   
			}   
		}   
		else{   
			KdPrint(("PsLookupThreadByThreadId ???"));   
		}   
		//===================================================================   
		return STATUS_SUCCESS;   
	}   
#ifdef __cplusplus   
}; // extern "C"   
#endif   

