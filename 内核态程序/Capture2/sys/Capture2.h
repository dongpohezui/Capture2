
#ifndef _Capture2_H
#define _Capture2_H

// Legal values include:
//    6.0  Available starting with Windows Vista RTM
//    6.1  Available starting with Windows Vista SP1 / Windows Server 2008
//    6.20 Available starting with Windows 7 / Windows Server 2008 R2
//    6.30 Available starting with Windows 8 / Windows Server "8"
// Legal values include:
//    6.0  Available starting with Windows Vista RTM
//    6.1  Available starting with Windows Vista SP1 / Windows Server 2008
//    6.20 Available starting with Windows 7 / Windows Server 2008 R2
//    6.30 Available starting with Windows 8 / Windows Server "8"
#define FILTER_MAJOR_NDIS_VERSION   6

#if defined(NDIS60)
#define FILTER_MINOR_NDIS_VERSION 0
#elif defined(NDIS620)
#define FILTER_MINOR_NDIS_VERSION   20
#elif defined(NDIS630)
#define FILTER_MINOR_NDIS_VERSION   30
#endif

#pragma warning(disable:4100) //UNREFERENCED_PARAMETER();
#pragma warning(disable:4101)
#pragma warning(disable:4189)

#include <ntifs.h>

#include <initguid.h>

#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union
#pragma warning(disable:4995)
#include <ndis.h>
#include <fwpsk.h>
#include <wdm.h>
#include <ntddk.h>
#include <wdf.h>
#pragma warning(pop)
#include <fwpmk.h>


#include <strsafe.h>


/*
导入相应的库

$(DDK_LIB_PATH)NTOSKrnl.lib
$(DDK_LIB_PATH)FwpKClnt.lib
$(DDK_LIB_PATH)NetIO.lib
$(DDK_LIB_PATH)NDIS.lib
$(DDK_LIB_PATH)WDMSec.lib
$(SDK_LIB_PATH)UUID.lib
$(DDK_LIB_PATH)WDM.lib

#pragma comment ( lib,"NTOSKrnl.lib" )
#pragma comment ( lib,"FwpKClnt.lib" )
#pragma comment ( lib,"NetIO.lib" )
#pragma comment ( lib,"NDIS.lib" )
#pragma comment ( lib,"WDMSec.lib" )
#pragma comment ( lib,"UUID.lib" )
#pragma comment ( lib,"WDM.lib" )

*/





#include "ioctl.h"

NTSTATUS
Capture2RegisterCallouts(
    _In_  PVOID DeviceObject
);

VOID
Capture2UnregisterCallout(VOID);

// {ED877AC0 - 0114 - 7C74 - 379C - D6C3766C3CD4}
__declspec (selectany) const GUID WFP_DRIVER_CLASS_GUID =
{ 0xED877AC0, 0x0114,0x7C74,{ 0x37, 0x9c,0xd6, 0xc3, 0x76, 0x6c, 0x3c, 0xd4 } };

//{026C008D-1EF0-4044-A81D-80DC94996CA5}
__declspec (selectany) const GUID Capture2_V4_CALLOUT1 =
{ 0x26c008d, 0x1ef0, 0x4044, { 0xa8, 0x1d, 0x80, 0xdc, 0x94, 0x99, 0x6c, 0xa5 } };

// {147C0788-EA96-449F-8FCC-295FA59E815D}
__declspec (selectany) const GUID Capture2_V4_CALLOUT2 =
{ 0x147c0788, 0xea96, 0x449f, { 0x8f, 0xcc, 0x29, 0x5f, 0xa5, 0x9e, 0x81, 0x5d } };

// {375529A1-65C1-4AF7-BB11-C09B620850B2}
__declspec (selectany) const GUID Capture2_V4_CALLOUT3 =
{ 0x375529a1, 0x65c1, 0x4af7, { 0xbb, 0x11, 0xc0, 0x9b, 0x62, 0x8, 0x50, 0xb2 } };

// {C409A75E-5A52-42B7-A4F8-2C3F4CB73042}
__declspec (selectany) const GUID Capture2_V4_CALLOUT4 =
{ 0xc409a75e, 0x5a52, 0x42b7, { 0xa4, 0xf8, 0x2c, 0x3f, 0x4c, 0xb7, 0x30, 0x42 } };


//
// Capture2 Globals block
//
typedef struct _Capture2_GLOBALS
{

	// ... For Callout #1
	//
	// IPV4 callout for  layer1
	UINT32 Layer1V4Callout1;
	UINT64 Layer1V4Filter1;
	
	UINT32 Layer1V4Callout2;
	UINT64 Layer1V4Filter2;
	//
	// ...For Callout #2
	//
	// IPV4 callout for FWPM_LAYER_INBOUND_MAC_FRAME_ETHERNET layer
	UINT32 Layer2V4Callout1;
	UINT64 Layer2V4Filter1;

	// IPV4 callout for FWPM_LAYER_OUTBOUND_MAC_FRAME_ETHERNET
	UINT32 Layer2V4Callout2;
	UINT64 Layer2V4Filter2;

	// WFP injection handles
	HANDLE InjectionHandle;

	// WFP Engine handle
	HANDLE EngineHandle;

	// Device object
	DEVICE_OBJECT* WdmDevice;


	// True if the driver is unloading/shutting down
	volatile char DriverUnloading;


	//输出Mac层数据包
	int MacDebug;
	LIST_ENTRY mac_list_head;
	PKEVENT pkEvent;
	KSPIN_LOCK spin_lock;
	
	//输出日志
	int record;
	LIST_ENTRY record_list_head;
	PKEVENT record_pkEvent;
	KSPIN_LOCK record_spin_lock;
	
	//ip过滤
	int ipBlock;
	LIST_ENTRY rule_list_head;
	KSPIN_LOCK rule_spin_lock;

} Capture2_GLOBALS;


//Mac数据包的结构体
typedef struct _Capture2_MACDATA {
	LIST_ENTRY ListEntry;
	size_t BufLen;
	PCHAR data;
} Capture2_MACDATA,*PCapture2_MACDATA;


//ip拦截规则
typedef struct _Capture2_IPBLOCK {
	LIST_ENTRY ListEntry;
	int id;
	int direction;   			//0入站，1出站，2双向，3其他
	UINT8 protocol;
	UINT32 srcIp;
	UINT32 srcIpMask;
	UINT16 srcPort;
	UINT16 srcPortMask;
	UINT32 dstIp;
	UINT32 dstIpMask;
	UINT16 dstPort;
	UINT16 dstPortMask;
} Capture2_IPBLOCK,*PCapture2_IPBLOCK;



extern Capture2_GLOBALS      Globals;

VOID EvFileCreate(IN WDFDEVICE device,IN WDFREQUEST request,IN WDFFILEOBJECT fileObject);

VOID EvFileClose(IN WDFFILEOBJECT fileObject);
VOID EvFileCleanup(IN WDFFILEOBJECT fileObject);
VOID Capture2EvtIoDeviceControl(
    IN WDFQUEUE queue,
    IN WDFREQUEST request,
    IN size_t outputBufferLength,
    IN size_t inputBufferLength,
    IN ULONG ioControlCode
);


VOID
Capture2Classify1(
    _In_ const FWPS_INCOMING_VALUES* inFixedValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
    _In_ PVOID LayerData,
    _In_ const VOID* ClassifyContext,
    _In_ const FWPS_FILTER* Filter,
    _In_ UINT64 InFlowContext,
    _Inout_ FWPS_CLASSIFY_OUT* ClassifyOut
);


VOID
Capture2Classify2(
    _In_ const FWPS_INCOMING_VALUES* inFixedValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
    _In_ PVOID LayerData,
    _In_ const VOID* ClassifyContext,
    _In_ const FWPS_FILTER* Filter,
    _In_ UINT64 InFlowContext,
    _Inout_ FWPS_CLASSIFY_OUT* ClassifyOut
);


NTSTATUS
Capture2Notify1(
    _In_ FWPS_CALLOUT_NOTIFY_TYPE NotifyType,
    _In_ const GUID* FilterKey,
    _In_ const FWPS_FILTER* Filter
);


NTSTATUS
Capture2Notify2(
    _In_ FWPS_CALLOUT_NOTIFY_TYPE NotifyType,
    _In_ const GUID* FilterKey,
    _In_ const FWPS_FILTER* Filter
);


VOID
Capture2FlowDelete1(
    _In_ UINT16 LayerId,
    _In_ UINT32 CalloutId,
    _In_ UINT64 Context
);


VOID
Capture2FlowDelete2(
    _In_ UINT16 LayerId,
    _In_ UINT32 CalloutId,
    _In_ UINT64 Context
);



BOOLEAN
Capture2BlockPacket(
	IN PCapture2_IPBLOCK ppacket
);

NTSTATUS 
Capture2getIprules();

NTSTATUS 
Capture2cleanIprules();


void 
PrintDataString(PCHAR pBuf,int BufLen);


#endif 


