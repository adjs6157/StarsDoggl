#include <windows.h>
#include <winternl.h>
//
//
//NTSTATUS  SearchMouServiceCallBack(IN PDRIVER_OBJECT DriverObject)
//{
//	//定义用到的一组全局变量，这些变量大多数是顾名思义的  
//	NTSTATUS status = STATUS_SUCCESS;
//	UNICODE_STRING uniNtNameString;
//	PDEVICE_OBJECT pTargetDeviceObject = NULL;
//	PDRIVER_OBJECT KbdDriverObject = NULL;
//	PDRIVER_OBJECT KbdhidDriverObject = NULL;
//	PDRIVER_OBJECT Kbd8042DriverObject = NULL;
//	PDRIVER_OBJECT UsingDriverObject = NULL;
//	PDEVICE_OBJECT UsingDeviceObject = NULL;
//
//	PVOID UsingDeviceExt = NULL;
//
//	//这里的代码用来打开USB键盘端口驱动的驱动对象  
//	RtlInitUnicodeString(&uniNtNameString, USBMOU_DRIVER_NAME);
//	status = ObReferenceObjectByName(&uniNtNameString,
//		OBJ_CASE_INSENSITIVE, NULL, 0,
//		*IoDriverObjectType,
//		KernelMode,
//		NULL,
//		(PVOID *)&KbdhidDriverObject);
//	if (!NT_SUCCESS(status))
//	{
//		KdPrint(("Couldn't get the USB Mouse Object\n"));
//	}
//	else
//	{
//		ObDereferenceObject(KbdhidDriverObject);
//		KdPrint(("get the USB Mouse Object\n"));
//	}
//	//打开PS/2键盘的驱动对象  
//	RtlInitUnicodeString(&uniNtNameString, PS2MOU_DRIVER_NAME);
//	status = ObReferenceObjectByName(&uniNtNameString,
//		OBJ_CASE_INSENSITIVE,
//		NULL, 0,
//		*IoDriverObjectType,
//		KernelMode,
//		NULL,
//		(PVOID *)&Kbd8042DriverObject);
//	if (!NT_SUCCESS(status))
//	{
//		KdPrint(("Couldn't get the PS/2 Mouse Object %08x\n", status));
//	}
//	else
//	{
//		ObDereferenceObject(Kbd8042DriverObject);
//		KdPrint(("get the PS/2 Mouse Object\n"));
//	}
//	//如果两个设备都没有找到  
//	if (!Kbd8042DriverObject && !KbdhidDriverObject)
//	{
//		return STATUS_SUCCESS;
//	}
//	//如果USB键盘和PS/2键盘同时存在，使用USB鼠标
//	if (KbdhidDriverObject)
//	{
//		UsingDriverObject = KbdhidDriverObject;
//	}
//	else
//	{
//		UsingDriverObject = Kbd8042DriverObject;
//	}
//	RtlInitUnicodeString(&uniNtNameString, MOU_DRVIER_NAME);
//	status = ObReferenceObjectByName(&uniNtNameString,
//		OBJ_CASE_INSENSITIVE, NULL,
//		0,
//		*IoDriverObjectType,
//		KernelMode,
//		NULL,
//		(PVOID *)&KbdDriverObject);
//	if (!NT_SUCCESS(status))
//	{
//		//如果没有成功，直接返回即可  
//		KdPrint(("MyAttach: Coundn't get the Mouse driver Object\n"));
//		return STATUS_UNSUCCESSFUL;
//	}
//	else
//	{
//		ObDereferenceObject(KbdDriverObject);
//	}
//	//遍历KbdDriverObject下的设备对象 
//	UsingDeviceObject = UsingDriverObject->DeviceObject;
//	while (UsingDeviceObject)
//	{
//		status = SearchServiceFromMouExt(KbdDriverObject, UsingDeviceObject);
//		if (status == STATUS_SUCCESS)
//		{
//			break;
//		}
//		UsingDeviceObject = UsingDeviceObject->NextDevice;
//	}
//	if (g_KoMCallBack.MouDeviceObject && g_KoMCallBack.MouseClassServiceCallback)
//	{
//		KdPrint(("Find MouseClassServiceCallback\n"));
//	}
//	return status;
//}