
#include "Capture2.h"


VOID EvFileCreate(IN WDFDEVICE device,IN WDFREQUEST request,IN WDFFILEOBJECT fileObject)
{
	WdfRequestComplete(request, STATUS_SUCCESS);

	DbgPrint("EvFileCreate\n");

	UNREFERENCED_PARAMETER(device);

	UNREFERENCED_PARAMETER(fileObject);
}

VOID EvFileClose(IN WDFFILEOBJECT fileObject)
{
	UNREFERENCED_PARAMETER(fileObject);
	DbgPrint("EvFileClose\n");
}

VOID EvFileCleanup(IN WDFFILEOBJECT fileObject)
{
	UNREFERENCED_PARAMETER(fileObject);
	DbgPrint("EvFileCleanup\n");
}


/*

内核事件KEVENT（同步） - 沉疴 - 博客园
https://www.cnblogs.com/lsh123/p/7358702.html

*/
VOID
Capture2EvtIoDeviceControl(
    IN WDFQUEUE queue,
    IN WDFREQUEST request,
    IN size_t outputBufferLength,
    IN size_t inputBufferLength,
    IN ULONG ioControlCode
)
{
	DbgPrint("Capture2EvtIoDeviceControl\n");
	UNREFERENCED_PARAMETER(queue);

	UNREFERENCED_PARAMETER(outputBufferLength);


	PVOID buffer=NULL;
	size_t length=0;
	NTSTATUS status;

	switch (ioControlCode)
	{
	case CAPTURE2_IOCTL_ONE:
	{

		if (inputBufferLength) {

			//清除List_entry

			HANDLE hEvent ;
			
			PHANDLE pEvent;
			
			status=WdfRequestRetrieveInputBuffer(request, 4, &pEvent, &length);
			hEvent = *pEvent;

			if (!NT_SUCCESS(status)) {

				DbgPrint("WdfRequestRetrieveInputBuffer failed. status:%X\n",status);
				WdfRequestCompleteWithInformation(request, STATUS_UNSUCCESSFUL, 4);
				return ;
			}

			//把句柄转化为KEvent结构
			//获得传递进来的事件句柄
			status = ObReferenceObjectByHandle(hEvent,
			                                   GENERIC_ALL, *ExEventObjectType, KernelMode,
			                                   &(Globals.pkEvent), NULL);

			if (!NT_SUCCESS(status)) {

				DbgPrint("ObReferenceObjectByHandle failed. status:%X\n",status);
				WdfRequestCompleteWithInformation(request, STATUS_UNSUCCESSFUL, 0);
				return ;
			}

			Globals.MacDebug = 1;

			WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, 0);
			return ;

		}
		
		WdfRequestCompleteWithInformation(request, STATUS_UNSUCCESSFUL, 0);
		return ;
		break;

	}

	case CAPTURE2_IOCTL_TWO:
	{
		Globals.MacDebug = 0;
		
		
		if(!IsListEmpty( & (Globals.mac_list_head))) {
			//遍历链表获取长度
			length = 0;
			PLIST_ENTRY pListEntry = NULL;
			
			for(pListEntry=Globals.mac_list_head.Flink; pListEntry!=& (Globals.mac_list_head); pListEntry=pListEntry->Flink){
				length++;
			}

			WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, length);
		}else{
			WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, 0);
		}
		return ;

		break;

	}
	case CAPTURE2_IOCTL_THREE:
	{
		length = 0;

		//dump结点指针
		if(!IsListEmpty( & (Globals.mac_list_head))) {
			PLIST_ENTRY plist;
			plist = ExInterlockedRemoveHeadList(&(Globals.mac_list_head), &(Globals.spin_lock));
		
			PCapture2_MACDATA pmacdata = CONTAINING_RECORD(plist,Capture2_MACDATA,ListEntry);

			//输出数据
			//PrintDataString(pmacdata->data,pmacdata->BufLen);

			//输出完毕

			status=WdfRequestRetrieveOutputBuffer(request, 0, &buffer, NULL);
			
			if (!NT_SUCCESS(status)) {

				DbgPrint("WdfRequestRetrieveOutputBuffer failed. status:%X\n",status);
				WdfRequestCompleteWithInformation(request, STATUS_UNSUCCESSFUL, 0);
				return ;
			}

			if (outputBufferLength >= pmacdata->BufLen) {
				RtlCopyMemory(buffer,pmacdata->data,pmacdata->BufLen);
				length = pmacdata->BufLen;
			}

			//释放内存
			ExFreePoolWithTag(pmacdata->data, 'tag1');
			ExFreePoolWithTag(pmacdata, 'MACd');

		}

		//WdfRequestComplete(request,STATUS_SUCCESS);
		WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, length);
		return ;

		break;

	}
	case CAPTURE2_IOCTL_FOUR:
	{
		break;
	}
	case CAPTURE2_IOCTL_FIVE:
	{
		break;
	}
	case CAPTURE2_IOCTL_SIX:
	{
		break;
	}	
	//开启ip拦截
	case CAPTURE2_IOCTL_SEVEN:
	{
		Globals.ipBlock = 1;
		WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, 0);
		return ;
		break;
	}	
	//关闭ip拦截
	case CAPTURE2_IOCTL_EIGHT:
	{
		Globals.ipBlock = 0;
		WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, 0);
		return ;
		break;
	}	
	//获取iP拦截规则
	case CAPTURE2_IOCTL_NINE:
	{
		Capture2cleanIprules();
		
		Capture2getIprules();

		WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, 0);
		return;

		break;
	}	
	//清空ip拦截规则
	case CAPTURE2_IOCTL_TEN:
	{

		Capture2cleanIprules();
		
		WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, 0);
		return;
		
		break;
	}	
	//查看ip拦截规则
	case CAPTURE2_IOCTL_ELEVEN:
	{
		
		WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, 0);
		return;
		
		break;
	}	
	
	case CAPTURE2_IOCTL_TWELVE:
	{
		break;
	}
	default:
	{
		if (inputBufferLength)
		{
			status=WdfRequestRetrieveInputBuffer(request, 4, &buffer, &length);

			if (NT_SUCCESS(status))
			{
				UINT32 data = *(PUINT32)buffer;
				KdPrint(("data",data));
				status=WdfRequestRetrieveOutputBuffer(request, 4, &buffer, &length);

				if (NT_SUCCESS(status))
				{
					*(PUINT32)buffer = data*2;
					WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, 4);
					return;
				}
			}
		}

		break;

	}
	}
	
	WdfRequestCompleteWithInformation(request, STATUS_SUCCESS, 0);
	//WdfRequestCompleteWithInformation(request, STATUS_INVALID_PARAMETER, 0);
}

