#include <windows.h>
#include <winternl.h>
//
//
//NTSTATUS  SearchMouServiceCallBack(IN PDRIVER_OBJECT DriverObject)
//{
//	//�����õ���һ��ȫ�ֱ�������Щ����������ǹ���˼���  
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
//	//����Ĵ���������USB���̶˿���������������  
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
//	//��PS/2���̵���������  
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
//	//��������豸��û���ҵ�  
//	if (!Kbd8042DriverObject && !KbdhidDriverObject)
//	{
//		return STATUS_SUCCESS;
//	}
//	//���USB���̺�PS/2����ͬʱ���ڣ�ʹ��USB���
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
//		//���û�гɹ���ֱ�ӷ��ؼ���  
//		KdPrint(("MyAttach: Coundn't get the Mouse driver Object\n"));
//		return STATUS_UNSUCCESSFUL;
//	}
//	else
//	{
//		ObDereferenceObject(KbdDriverObject);
//	}
//	//����KbdDriverObject�µ��豸���� 
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