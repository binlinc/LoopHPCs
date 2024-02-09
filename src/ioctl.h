

/* avoid multiple includes */
#pragma once

#define FILE_DEVICE_INVERTED 0xCF54

/* Operations - control codes */
#define IOCTL_SEND_LOGFILENAME CTL_CODE(FILE_DEVICE_UNKNOWN, 2048, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SEND_MLPX CTL_CODE(FILE_DEVICE_UNKNOWN, 2049, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SEND_PNAME CTL_CODE(FILE_DEVICE_UNKNOWN, 2050, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SEND_PID CTL_CODE(FILE_DEVICE_UNKNOWN, 2051, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SEND_TID CTL_CODE(FILE_DEVICE_UNKNOWN, 2052, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SEND_CurrentTID CTL_CODE(FILE_DEVICE_UNKNOWN, 2053, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SEND_ADDR CTL_CODE(FILE_DEVICE_UNKNOWN, 2061, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SEND_SIZE CTL_CODE(FILE_DEVICE_UNKNOWN, 2062, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SEND_LibsAddrMax CTL_CODE(FILE_DEVICE_UNKNOWN, 2063, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_Semp CTL_CODE(FILE_DEVICE_UNKNOWN, 2071, METHOD_BUFFERED, FILE_ANY_ACCESS)