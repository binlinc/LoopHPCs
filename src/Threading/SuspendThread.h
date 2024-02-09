#ifndef __SUSPENDTHREAD_H_VERSION__ 
#define __SUSPENDTHREAD_H_VERSION__ 100 

#if defined(_MSC_VER) && (_MSC_VER = 1020) 
#pragma once 
#endif 


#include msg_SuspendThread.h 
#include drvcommon.h 
#include drvversion.h 

#define DEVICE_NAME			DeviceSUSPENDTHREAD_DeviceName 
#define SYMLINK_NAME		DosDevicesSUSPENDTHREAD_DeviceName 
PRESET_UNICODE_STRING(usDeviceName, DEVICE_NAME); 
PRESET_UNICODE_STRING(usSymlinkName, SYMLINK_NAME); 

#ifndef FILE_DEVICE_SUSPENDTHREAD 
#define FILE_DEVICE_SUSPENDTHREAD 0x8000 
#endif 

Values defined for Method 
	METHOD_BUFFERED 
	METHOD_IN_DIRECT 
	METHOD_OUT_DIRECT 
	METHOD_NEITHER 

	Values defined for Access 
	FILE_ANY_ACCESS 
	FILE_READ_ACCESS 
	FILE_WRITE_ACCESS 

	const ULONG IOCTL_SUSPENDTHREAD_OPERATION = CTL_CODE(FILE_DEVICE_SUSPENDTHREAD, 0x01, METHOD_BUFFERED, FILE_READ_DATA  FILE_WRITE_DATA); 

#define GetSSDTIndex(_function) (PULONG)((PUCHAR)_function+1) 
#define GetFncAddr(_index) (PULONG)((ULONG)KeServiceDescriptorTable-ServiceTableBase + (_index)  4) 

#define IOCTL_TRANSFER_TYPE( _iocontrol)   (_iocontrol & 0x3) 
typedef struct _ServiceDescriptorTable_ { 
	PVOID ServiceTableBase; System Service Dispatch Table ??????   
		PVOID ServiceCounterTable; ?????? SSDT ???????????????????????????????????????sysenter ????. 
		ULONG NumberOfServices;?? ServiceTableBase ???????????????   
		PVOID ParamTableBase; ?????????????????????????????-???????????  
} SRVTABLE, PSRVTABLE; 


typedef NTSTATUS (__stdcall PSSUSPENDTHREAD)( 
	IN PETHREAD Thread, 
	OUT PULONG PreviousSuspendCount); 

extern C NTSYSAPI NTSTATUS NTAPI PsLookupThreadByThreadId( 
	__in HANDLE ThreadId, 
	__deref_out PETHREAD Thread 
	); 
extern C NTSYSAPI NTSTATUS ZwSuspendThread ( 
	__in HANDLE ThreadHandle, 
	__out_opt PULONG PreviousSuspendCount 
	); 

extern C PSRVTABLE KeServiceDescriptorTable; 
#endif  __SUSPENDTHREAD_H_VERSION__ 

ULONG GetNt_OldAddr(IN PCWSTR SourceString)??????????? 
{ 
	UNICODE_STRING Old_NtOpenprocess; 
	ULONG Old_Addr; 
	RtlInitUnicodeString(&Old_NtOpenprocess,SourceString); 
	Old_Addr = (ULONG)MmGetSystemRoutineAddress(&Old_NtOpenprocess);??��??????? 
		KdPrint((????????Old_NtOpenProcess???? %x,Old_Addr)); 
	return Old_Addr; 
} 
ULONG SearchPsSuspendThreadAddr(ULONG StartAddress) 
{ 
	ULONG	u_index; 

	UCHAR	p = (UCHAR)StartAddress; 
	83eb126b 6a18 
		83eb126d 683851c783  
		for (u_index = 0;u_index  200;u_index++) 
		{ 
			if (p==0x8B&& 
				(p+1)==0x6A&& 
				(p+2)==0x18&& 
				(p+3)==0x68&& 
				(p+4)==0x38&& 
				(p+5)==0x51&& 
				(p+6)==0xC7) 

			{ 
				return (ULONG)p; 
			} 

			p++; 
		} 

		return 0; 
}