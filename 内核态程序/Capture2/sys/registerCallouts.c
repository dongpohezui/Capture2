
#include "Capture2.h"

NTSTATUS
Capture2RegisterCalloutForLayer1(
    IN const GUID* layerKey,
    IN const GUID* calloutKey,
//    IN FWPS_CALLOUT_CLASSIFY_FN classifyFn,
//    IN FWPS_CALLOUT_NOTIFY_FN notifyFn,
//    IN FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteNotifyFn,
    OUT UINT32* calloutId,
    OUT UINT64* filterId
)
{
	DbgPrint("Capture2RegisterCalloutForLayer1\n");

	NTSTATUS        status = STATUS_SUCCESS;
	FWPS_CALLOUT    sCallout = { 0 };
	FWPM_FILTER     mFilter = { 0 };
	FWPM_FILTER_CONDITION mFilter_condition[1] = { 0 };
	FWPM_CALLOUT    mCallout = { 0 };
	FWPM_DISPLAY_DATA mDispData = { 0 };
	BOOLEAN         bCalloutRegistered = FALSE;

	sCallout.calloutKey = *calloutKey;
	sCallout.classifyFn = Capture2Classify1;
	sCallout.flowDeleteFn = Capture2FlowDelete1;
	sCallout.notifyFn = Capture2Notify1;

	//要使用哪个设备对象注册
	status = FwpsCalloutRegister(Globals.WdmDevice, &sCallout, calloutId);

	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

	bCalloutRegistered = TRUE;
	mDispData.name = L"WFP Capture2 Layer1";
	mDispData.description = L"WFP Capture2 Layer1 description";

	//感兴趣的内容
	mCallout.applicableLayer = *layerKey;
	//感兴趣的内容的GUID
	mCallout.calloutKey = *calloutKey;
	mCallout.displayData = mDispData;
	//添加回调函数
	status = FwpmCalloutAdd(Globals.EngineHandle, &mCallout, NULL, NULL);

	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

	mFilter.action.calloutKey = *calloutKey;
	//在callout里决定
	mFilter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	mFilter.displayData.name = L"WFP Capture2 Layer1 Filter";
	mFilter.displayData.description = L"WFP Capture2 Layer1 Filter";
	mFilter.layerKey = *layerKey;
	mFilter.numFilterConditions = 0;
	mFilter.filterCondition = mFilter_condition;
	mFilter.subLayerKey = FWPM_SUBLAYER_UNIVERSAL;
	mFilter.weight.type = FWP_EMPTY;
	//添加过滤器
	status = FwpmFilterAdd(Globals.EngineHandle, &mFilter, NULL, filterId);

	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

exit:

	if (!NT_SUCCESS(status))
	{
		
		DbgPrint("Capture2RegisterCalloutForLayer1 failed. status:%X\n",status);
		
		if (bCalloutRegistered)
		{
			FwpsCalloutUnregisterById(*calloutId);
		}
	}

	return status;
}



NTSTATUS
Capture2RegisterCalloutForLayer2(
    IN const GUID* layerKey,
    IN const GUID* calloutKey,
//    IN FWPS_CALLOUT_CLASSIFY_FN classifyFn,
//    IN FWPS_CALLOUT_NOTIFY_FN notifyFn,
//    IN FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteNotifyFn,
    OUT UINT32* calloutId,
    OUT UINT64* filterId
)
{
	DbgPrint("Capture2RegisterCalloutForLayer2");

	NTSTATUS        status = STATUS_SUCCESS;
	FWPS_CALLOUT    sCallout = { 0 };
	FWPM_FILTER     mFilter = { 0 };
	FWPM_FILTER_CONDITION mFilter_condition[1] = { 0 };
	FWPM_CALLOUT    mCallout = { 0 };
	FWPM_DISPLAY_DATA mDispData = { 0 };
	BOOLEAN         bCalloutRegistered = FALSE;

	sCallout.calloutKey = *calloutKey;
	sCallout.classifyFn = Capture2Classify2;
	sCallout.flowDeleteFn = Capture2FlowDelete2;
	sCallout.notifyFn = Capture2Notify2;

	//要使用哪个设备对象注册
	status = FwpsCalloutRegister(Globals.WdmDevice, &sCallout, calloutId);

	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

	bCalloutRegistered = TRUE;
	mDispData.name = L"WFP Capture2 Layer1";
	mDispData.description = L"WFP Capture2 Layer1 description";

	//感兴趣的内容
	mCallout.applicableLayer = *layerKey;
	//感兴趣的内容的GUID
	mCallout.calloutKey = *calloutKey;
	mCallout.displayData = mDispData;
	//添加回调函数
	status = FwpmCalloutAdd(Globals.EngineHandle, &mCallout, NULL, NULL);

	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

	mFilter.action.calloutKey = *calloutKey;
	//在callout里决定
	mFilter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	mFilter.displayData.name = L"WFP Capture2 Layer2 Filter";
	mFilter.displayData.description = L"WFP Capture2 Layer2 Filter";
	mFilter.layerKey = *layerKey;
	mFilter.numFilterConditions = 0;
	mFilter.filterCondition = mFilter_condition;
	mFilter.subLayerKey = FWPM_SUBLAYER_UNIVERSAL;
	mFilter.weight.type = FWP_EMPTY;
	//添加过滤器
	status = FwpmFilterAdd(Globals.EngineHandle, &mFilter, NULL, filterId);

	if (!NT_SUCCESS(status))
	{
		goto exit;
	}

exit:

	if (!NT_SUCCESS(status))
	{
		DbgPrint("Capture2RegisterCalloutForLayer2 failed. status:%X\n",status);
		if (bCalloutRegistered)
		{
			FwpsCalloutUnregisterById(*calloutId);
		}
	}

	return status;

}


NTSTATUS
Capture2RegisterCallouts(
    _In_  PVOID DeviceObject
)
/**
    TODO
*/
{
	DbgPrint("Capture2RegisterCallouts");
	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS Status = STATUS_SUCCESS;

	BOOLEAN EngineOpened = FALSE;
	BOOLEAN InTransaction = FALSE;

	FWPM_SESSION session = { 0 };

	session.flags = FWPM_SESSION_FLAG_DYNAMIC;

	//打开引擎
	Status = FwpmEngineOpen(
	             NULL,
	             RPC_C_AUTHN_WINNT,
	             NULL,
	             &session,
	             &Globals.EngineHandle
	         );

	if (NT_SUCCESS(Status))
	{
		EngineOpened = TRUE;

		//开始事务
		Status = FwpmTransactionBegin(Globals.EngineHandle, 0);

		if (NT_SUCCESS(Status))
		{
			InTransaction = TRUE;

			// Add Callout Set 1

			//注册callout和filter
			Capture2RegisterCalloutForLayer1(
			    &FWPM_LAYER_OUTBOUND_TRANSPORT_V4,
			    &Capture2_V4_CALLOUT1,
			    //Capture2Classify,
			    //Capture2Notify,
			    //Capture2FlowDelete,
			    &Globals.Layer1V4Callout1,
			    &Globals.Layer1V4Filter1);

			//注册callout和filter
			Capture2RegisterCalloutForLayer1(
			    &FWPM_LAYER_INBOUND_TRANSPORT_V4,
			    &Capture2_V4_CALLOUT2,
			    //Capture2Classify,
			    //Capture2Notify,
			    //Capture2FlowDelete,
			    &Globals.Layer1V4Callout2,
			    &Globals.Layer1V4Filter2);

			// Add Callout Set 2
			// 注册callout和filter
			Capture2RegisterCalloutForLayer2(
			    &FWPM_LAYER_INBOUND_MAC_FRAME_NATIVE,
			    &Capture2_V4_CALLOUT3,
			    //Capture2Classify,
			    //Capture2Notify,
			    //Capture2FlowDelete,
			    &Globals.Layer2V4Callout1,
			    &Globals.Layer2V4Filter1);

			Capture2RegisterCalloutForLayer2(
			    &FWPM_LAYER_OUTBOUND_MAC_FRAME_NATIVE,
			    &Capture2_V4_CALLOUT4,
			    //Capture2Classify,
			    //Capture2Notify,
			    //Capture2FlowDelete,
			    &Globals.Layer2V4Callout2,
			    &Globals.Layer2V4Filter2);

		}

		//提交事务
		Status = FwpmTransactionCommit(Globals.EngineHandle);

		if (NT_SUCCESS(Status))
		{
			InTransaction = FALSE;
		}

	}

	if (!NT_SUCCESS(Status))
	{
		DbgPrint("Capture2RegisterCallouts failed. Status:%X\n", Status);
		
		if (InTransaction)
		{
			NTSTATUS AbortStatus;
			AbortStatus = FwpmTransactionAbort(Globals.EngineHandle);
			_Analysis_assume_(NT_SUCCESS(AbortStatus));
		}

		if (EngineOpened)
		{
			FwpmEngineClose(Globals.EngineHandle);
			Globals.EngineHandle = NULL;
		}
	}
	return Status;
}

VOID
Capture2UnregisterCallout(VOID)
/**
    UnregisterCallout.
*/
{
	DbgPrint("Capture2UnregisterCallout\n");

	//FwpmCalloutDeleteByKey

	if(Globals.Layer1V4Callout1) {
		FwpsCalloutUnregisterById(Globals.Layer1V4Callout1);
	}
	
	if(Globals.Layer1V4Callout2) {
		FwpsCalloutUnregisterById(Globals.Layer1V4Callout2);
	}

	if(Globals.Layer2V4Callout1) {
		FwpsCalloutUnregisterById(Globals.Layer2V4Callout1);
	}

	if(Globals.Layer2V4Callout2) {
		FwpsCalloutUnregisterById(Globals.Layer2V4Callout2);
	}


}


