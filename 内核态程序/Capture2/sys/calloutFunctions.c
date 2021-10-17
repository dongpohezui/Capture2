#include "Capture2.h"

VOID
PSQueryPacketLength(
    IN PNET_BUFFER_LIST Packet,
    OUT OPTIONAL PUINT32 Length
)
{
	PNET_BUFFER pNetBuffer = NET_BUFFER_LIST_FIRST_NB(Packet);
	BOOLEAN FLAG = FALSE;

	do
	{
		if (NULL == Length)
		{
			break;
		}

		*Length = 0;

		while (pNetBuffer)
		{
			*Length += NET_BUFFER_DATA_LENGTH(pNetBuffer);
			pNetBuffer = NET_BUFFER_NEXT_NB(pNetBuffer);
		}
	} while (FLAG);

	return;
}


void 
PrintDataString(PCHAR pBuf,int BufLen){
	PCHAR pStringBuf;
	NTSTATUS Status;
	pStringBuf=ExAllocatePoolWithTag(NonPagedPool,BufLen *2+10,'tag2');

	if (NULL == pStringBuf)
	{
		DbgPrint("%s: Allocate memory failed! \n",__FUNCTION__);
		return ;
	}

	NdisZeroMemory(pStringBuf,BufLen *2+10);

	int i = 0;

	for (i=0; i<BufLen; i++) {
		Status = StringCchPrintfA(pStringBuf +i*2,3,"%02X", (PCHAR)*(pBuf+i));
	}

	DbgPrint("pStringBuf:%s\n",pStringBuf);

	ExFreePoolWithTag(pStringBuf, 'tag2');
	
}


/*
数据偏移位置-Windows驱动程序| 微软文档
https://docs.microsoft.com/en-us/windows-hardware/drivers/network/data-offset-positions

FwpsAllocateCloneNetBufferList0()

*/
BOOLEAN
GetOneNetBufferListData(
    __in PNET_BUFFER_LIST Packet,
    __out PCHAR pBuf,
    __out int *BufLen
)
{
	DbgPrint("GetOneNetBufferListData\n");

	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	PNET_BUFFER_LIST pCurrentNBL = NULL;
	PNET_BUFFER pCurrentNB = NULL;
	int lTotalLength = 0;
	PMDL pMdl = NULL;
	PUCHAR pData = NULL;
	int lOffset = 0;
	int lDataBufferLen = 0;
	int lBytesToCopy = 0;
	int lMdlOffset = 0;

	pCurrentNBL = Packet;
	PSQueryPacketLength(pCurrentNBL,(PUINT32)&lTotalLength);

	PCHAR test = ExAllocatePoolWithTag(NonPagedPool, 60, 'test');

	if (lTotalLength > 0)
	{
		pBuf = ExAllocatePoolWithTag(NonPagedPool,lTotalLength,'tag1');

		if (NULL == pBuf)
		{
			DbgPrint("%s: Allocate memory failed! Status=0x%08x\n",__FUNCTION__,Status);
			return FALSE;
		}
	}
	else
	{
		DbgPrint("%s: The packet is an invalid packet!\n",__FUNCTION__);
		return FALSE;
	}

	NdisZeroMemory(pBuf,lTotalLength);
	*BufLen = (UINT32)lTotalLength;

	pCurrentNB = NET_BUFFER_LIST_FIRST_NB(pCurrentNBL);

	while (pCurrentNB)
	{
		pMdl = NET_BUFFER_CURRENT_MDL(pCurrentNB);
		lMdlOffset = NET_BUFFER_CURRENT_MDL_OFFSET(pCurrentNB);

		while(pMdl)
		{

			NdisQueryMdl( pMdl,(PVOID *)&pData,&lDataBufferLen,NormalPagePriority );

			//NBL->DataLength <= MDL1->ByteCount+ MDL2->ByteCount+........+ MDLx->ByteCount  -  NBL->CurrentMDLOffset.
			//这里 Data length 不等于 Buffer Length
			lBytesToCopy =min( lDataBufferLen - lMdlOffset, lTotalLength);

			NdisMoveMemory(
			    (pBuf+ lOffset),
			    pData + lMdlOffset,
			    lBytesToCopy
			);
			lTotalLength -= lBytesToCopy;
			lOffset += lBytesToCopy;

			//一般来说，只有第一个MDL的buffer才有偏移，其余的都没有。
			//如果offset大于了buflen，那么第一个MDL就没有存在的意义了。

			lMdlOffset = 0;

			NdisGetNextMdl(pMdl, &pMdl); //pMdl = pMdl->Next;
		}

		pCurrentNB = NET_BUFFER_NEXT_NB(pCurrentNB);

	}//end while(pCurrentNB)

	//当while循环结束以后，pBuf保存的是：一整个NET_BUFFER_LIST里面的数据内容。

	//处理数据

	//将数据插入链表

	//分配结构体的内存，在数据传送给用户层后注意释放内存
	PCapture2_MACDATA pmacdata;
	pmacdata = ExAllocatePoolWithTag(NonPagedPool,sizeof(Capture2_MACDATA),'MACd');

	RtlZeroMemory(pmacdata, sizeof(Capture2_MACDATA));
	pmacdata->BufLen = (*BufLen);
	pmacdata->data  = pBuf;

	PrintDataString(pmacdata->data,pmacdata->BufLen);

	//ExInterlockedInsertTailList从尾部插入节点
	ExInterlockedInsertTailList( &(Globals.mac_list_head),&(pmacdata->ListEntry),&(Globals.spin_lock) );
	DbgPrint("ExInterlockedInsertHeadList end\n");
	
	//使事件激发
	//KeSetEvent第三个参数一定为FALSE
	KeSetEvent(Globals.pkEvent, 0, FALSE);

	//处理数据完毕

	DbgPrint("GetOneNetBufferListData end\n");

	return TRUE;
}


VOID
ProtocolIdToName(UINT16 id,char* ProtocolName)
/**
协议代码转为名称
*/
{
	switch (id)
	{
	case 1:
		strcpy_s(ProtocolName, 4 + 1, "ICMP");
		break;

	case 2:
		strcpy_s(ProtocolName, 4 + 1, "IGMP");
		break;

	case 6:
		strcpy_s(ProtocolName, 3 + 1, "TCP");
		break;

	case 17:
		strcpy_s(ProtocolName, 3 + 1, "UDP");
		break;

	case 27:
		strcpy_s(ProtocolName, 3 + 1, "RDP");
		break;

	default:
		strcpy_s(ProtocolName, 7 + 1, "UNKNOWN");
		break;
	}

	return ;
}









VOID
Capture2Classify1(
    _In_ const FWPS_INCOMING_VALUES* inFixedValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
    _In_ PVOID LayerData,
    _In_ const VOID* ClassifyContext,
    _In_ const FWPS_FILTER* Filter,
    _In_ UINT64 InFlowContext,
    _Inout_ FWPS_CLASSIFY_OUT* ClassifyOut
)
/**
    处理流量
*/
{
	
	if(Globals.ipBlock == 0){
		
		ClassifyOut->actionType = FWP_ACTION_PERMIT;//允许连接
		return;
	}
	
	
	Capture2_IPBLOCK packet = {0};
	
	
	if(inFixedValues->layerId == FWPS_LAYER_INBOUND_TRANSPORT_V4){
		packet.direction = 0;
		packet.protocol= inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_PROTOCOL].value.uint16;
		packet.dstIp   = inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS].value.uint32;
		packet.dstPort = inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_PORT].value.uint16;
		packet.srcIp   = inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS].value.uint32;
		packet.srcPort = inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_PORT].value.uint16;
		
	}else if(inFixedValues->layerId == FWPS_LAYER_OUTBOUND_TRANSPORT_V4){
		packet.direction = 1;
		packet.protocol= inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_PROTOCOL].value.uint16;
		packet.srcIp   = inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS].value.uint32;
		packet.srcPort = inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_PORT].value.uint16;
		packet.dstIp   = inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS].value.uint32;
		packet.dstPort = inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_PORT].value.uint16;
	}
	
	
	if( Capture2BlockPacket(&packet) ) {
		ClassifyOut->actionType = FWP_ACTION_BLOCK;//禁止连接
	}else{
		ClassifyOut->actionType = FWP_ACTION_PERMIT;//允许连接
	}


}


VOID
Capture2Classify2(
    _In_ const FWPS_INCOMING_VALUES* inFixedValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
    _In_ PVOID LayerData,
    _In_ const VOID* ClassifyContext,
    _In_ const FWPS_FILTER* Filter,
    _In_ UINT64 InFlowContext,
    _Inout_ FWPS_CLASSIFY_OUT* ClassifyOut
)
{
	if (Globals.MacDebug) {
		DbgPrint("Capture2:[WFP]IRQL=%d;Local=%X;Remote=%X\n",
		         (USHORT)KeGetCurrentIrql(),
		         inFixedValues->incomingValue[FWPS_FIELD_INBOUND_MAC_FRAME_ETHERNET_MAC_LOCAL_ADDRESS].value.uint32,
		         inFixedValues->incomingValue[FWPS_FIELD_INBOUND_MAC_FRAME_ETHERNET_MAC_REMOTE_ADDRESS].value.uint32
		        );

		PCHAR pBuf = 0;
		int BufLen = 0;

		GetOneNetBufferListData(
		    __in LayerData,
		    __out pBuf,
		    __out &BufLen
		);

	}

	ClassifyOut->actionType = FWP_ACTION_PERMIT;//允许连接
}

NTSTATUS
Capture2Notify1(
    _In_ FWPS_CALLOUT_NOTIFY_TYPE NotifyType,
    _In_ const GUID* FilterKey,
    _In_ const FWPS_FILTER* Filter
)
/**
Capture2Notify

*/
{

	UNREFERENCED_PARAMETER(FilterKey);

	NT_ASSERT(Filter != NULL);
	return STATUS_SUCCESS;

}

NTSTATUS
Capture2Notify2(
    _In_ FWPS_CALLOUT_NOTIFY_TYPE NotifyType,
    _In_ const GUID* FilterKey,
    _In_ const FWPS_FILTER* Filter
)
{
	UNREFERENCED_PARAMETER(FilterKey);

	NT_ASSERT(Filter != NULL);

	return STATUS_SUCCESS;
}


VOID
Capture2FlowDelete1(
    _In_ UINT16 LayerId,
    _In_ UINT32 CalloutId,
    _In_ UINT64 Context
)
{
	UNREFERENCED_PARAMETER(LayerId);
	UNREFERENCED_PARAMETER(CalloutId);
	UNREFERENCED_PARAMETER(Context);
}


VOID
Capture2FlowDelete2(
    _In_ UINT16 LayerId,
    _In_ UINT32 CalloutId,
    _In_ UINT64 Context
)
{
	UNREFERENCED_PARAMETER(LayerId);
	UNREFERENCED_PARAMETER(CalloutId);
	UNREFERENCED_PARAMETER(Context);
}
