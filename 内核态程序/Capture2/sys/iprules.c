


#include "Capture2.h"



NTSTATUS 
Capture2getIprules(){
	
	NTSTATUS  status = STATUS_SUCCESS;
	HANDLE	hRegister = 0;
	
	
	OBJECT_ATTRIBUTES       KeyObjAttr;

	UNICODE_STRING          RegDirectory;
	
	RtlInitUnicodeString( &RegDirectory,L"\\Registry\\Machine\\SYSTEM\\ControlSet001\\Services\\Capture2\\iprules" );
	//初始化OBJECT_ATTRIBUTES结构
	InitializeObjectAttributes( &KeyObjAttr,
                                &RegDirectory,
                                OBJ_KERNEL_HANDLE,
                                0,
                                NULL);

	
	status = ZwOpenKey(&hRegister,
		KEY_ALL_ACCESS,
		&KeyObjAttr);
	
	if (NT_SUCCESS(status))
	{
		
	}else{
		
		return status;
		
	}


	ULONG size;
	
	//第一次调用获取size,返回的status为0xc000023不成功
	status = ZwQueryKey( hRegister,
                         KeyFullInformation,
                         NULL,
                         0,
                         &size );


	PKEY_FULL_INFORMATION pfi =
		(PKEY_FULL_INFORMATION)ExAllocatePool(PagedPool, size);
	status = ZwQueryKey(hRegister,
		KeyFullInformation,
		pfi,
		size,
		&size);
	
	if (NT_SUCCESS(status))
	{
		
	}else{
		return status;
	}
	
	for (ULONG i = 0; i < pfi->Values; i++)
	{
		ZwEnumerateValueKey(hRegister,
			i,
			KeyValuePartialInformation,
			NULL,
			0,
			&size);
		
		PKEY_VALUE_PARTIAL_INFORMATION pvbi =
			(PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, size);

		ZwEnumerateValueKey(hRegister,
			i,
			KeyValuePartialInformation,
			pvbi,
			size,
			&size);

		if(pvbi->DataLength != sizeof(Capture2_IPBLOCK)){
			continue;
		}

		//只需要REG_BINARY类型的数据
		if(pvbi->Type != REG_BINARY){
			continue;
		}

		PrintDataString(pvbi->Data,(int) pvbi->DataLength);
		
		PCapture2_IPBLOCK pIpblock;
		pIpblock = ExAllocatePoolWithTag(NonPagedPool,sizeof(Capture2_IPBLOCK),'rule');
		RtlZeroMemory(pIpblock, sizeof(Capture2_IPBLOCK));

		//读取注册表
		RtlCopyMemory(pIpblock,pvbi->Data,sizeof(Capture2_IPBLOCK));

		ExInterlockedInsertTailList( &(Globals.rule_list_head),&(pIpblock->ListEntry),&(Globals.rule_spin_lock) );

		ExFreePool(pvbi);
	}

	ZwClose(hRegister);

	ExFreePool(pfi);
}

NTSTATUS 
Capture2cleanIprules(){
	NTSTATUS        status = STATUS_SUCCESS;
	
	while( !IsListEmpty( & (Globals.rule_list_head)) ){
			PLIST_ENTRY plist;
			plist = ExInterlockedRemoveHeadList(&(Globals.rule_list_head), &(Globals.rule_spin_lock));
			PCapture2_IPBLOCK pIpblock = CONTAINING_RECORD(plist,Capture2_IPBLOCK,ListEntry);
			//释放内存
			ExFreePoolWithTag(pIpblock, 'rule');
		}

	return status;

}

BOOLEAN
Capture2BlockPacket(
	IN PCapture2_IPBLOCK ppacket
){
	
	if(ppacket == NULL){
		return FALSE;
	}

	//遍历ip规则
	if(IsListEmpty( & (Globals.rule_list_head))) {
		//允许连接
		return FALSE;
	}
	//遍历链表
	PLIST_ENTRY pList = NULL;
	
	for(pList=Globals.rule_list_head.Flink; pList!=& (Globals.rule_list_head); pList= pList->Flink){
		
		PCapture2_IPBLOCK pIpBlock = CONTAINING_RECORD(pList,Capture2_IPBLOCK,ListEntry);
		
		if(pIpBlock ->direction !=2 &&  pIpBlock ->direction != ppacket->direction){
			continue;
		}
		if(pIpBlock ->protocol != ppacket->protocol){
			continue;
		}
		if((pIpBlock ->srcIpMask & pIpBlock ->srcIp) != (pIpBlock ->srcIpMask & ppacket->srcIp) ){	
			continue;
		}				
					
		if((pIpBlock ->srcPortMask & pIpBlock ->srcPort) != (pIpBlock ->srcPortMask & ppacket->srcPort)  ){
			continue;
		}
		
		if((pIpBlock ->dstIpMask & pIpBlock ->dstIp) != (pIpBlock ->dstIpMask & ppacket->dstIp) ){
			continue;
		}
		if((pIpBlock ->dstPortMask & pIpBlock ->dstPort) != (pIpBlock ->dstPortMask & ppacket->dstPort)  ){
			continue;
		}
		
		return TRUE;

	}//for循环
	
	
	return FALSE;

}




		













