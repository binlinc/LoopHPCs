

#include "config.h"

/* I/O routines */
NTSTATUS NotSupported(PDEVICE_OBJECT DeviceObject,PIRP Irp);
NTSTATUS Read(PDEVICE_OBJECT DeviceObject,PIRP Irp);
NTSTATUS Create(PDEVICE_OBJECT DeviceObject,PIRP Irp);
NTSTATUS Close(PDEVICE_OBJECT DeviceObject,PIRP Irp);
NTSTATUS DispatchIoctl(PDEVICE_OBJECT DeviceObject,PIRP Irp);

BOOLEAN bIOCTL_SEND_PNAME;
BOOLEAN bIOCTL_SEND_LOGFILENAME;

