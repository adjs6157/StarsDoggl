#include "StarsControl.h"
#include <windows.h>
#include "winio.h"

#define key_cmd 0x64//��������˿�

#define key_dat 0x60//�������ݶ˿�

void KBCwait4IBE() {

	DWORD ch = 0; //ע�⣬����ѭ�����Ҳ�û�иı��������˿ڵ����ݣ���Ҳ���������ѭ����/ ��Ϊ�����жϻ����м�����ݶ�ȡ�����㣬�ǾͲ��ᵼ����ѭ����

	do {
		GetPortVal(key_cmd, &ch, 1);//��ȡ���̵�����˿ڣ��ó�ch

	} while (ch & 0x2);//bit1��1�Ļ���˵�����뻺����������������⣡ֱ����

}

void MakeKeyDown(DWORD VirtualKey)
{
	DWORD K_Make_Code = MapVirtualKey(VirtualKey, 0);//winuser. h���涨��õĺ�����
	KBCwait4IBE();
	SetPortVal(key_cmd, 0xD2, 1);//d2��˵��׼��д���ݵ�Output Register�С����ͨ��60hд�뵽Input Register���ֽڻᱻ���뵽Output Register�У��˹��ܱ�����ģ��������Keyboard���͵����ݡ�����жϱ�������ᴥ��һ���жϴ��������忴ԭ���ĵ��� 
	KBCwait4IBE();
	SetPortVal(key_dat, K_Make_Code, 1);
} //�� �ſ� 

void MakeKeyUp(DWORD VirtualKey)
{
	DWORD K_Make_Code = MapVirtualKey(VirtualKey, 0);//����ɨ���� 
	DWORD K_Break_Code = K_Make_Code + 0x80;//���Ķ��� 
	KBCwait4IBE(); 
	SetPortVal(key_cmd, 0xD2, 1);//d2��˵��׼��д���ݵ�Output Register�С����ͨ��60hд�뵽Input Register���ֽڻᱻ���뵽Output Register�У��˹��ܱ�����ģ��������Keyboard���͵����ݡ�����жϱ�������ᴥ��һ���жϴ��������忴ԭ���ĵ��� 
	KBCwait4IBE();
	SetPortVal(key_dat, K_Break_Code, 1);
}

// ��չ��
void MakeKeyDownEx(DWORD VirtualKey)
{
	DWORD K_Make_Code = MapVirtualKey(VirtualKey, 0);//winuser. h���涨��õĺ�����

	KBCwait4IBE();
	SetPortVal(key_cmd, 0xD2, 1);
	KBCwait4IBE();
	SetPortVal(key_dat, 0xE0, 1);	//д����չ����־��Ϣ

	KBCwait4IBE();
	SetPortVal(key_cmd, 0xD2, 1);//d2��˵��׼��д���ݵ�Output Register�С����ͨ��60hд�뵽Input Register���ֽڻᱻ���뵽Output Register�У��˹��ܱ�����ģ��������Keyboard���͵����ݡ�����жϱ�������ᴥ��һ���жϴ��������忴ԭ���ĵ��� 
	KBCwait4IBE();
	SetPortVal(key_dat, K_Make_Code, 1);
} //�� �ſ� 

void MakeKeyUpEx(DWORD VirtualKey)
{
	DWORD K_Make_Code = MapVirtualKey(VirtualKey, 0);//����ɨ���� 
	DWORD K_Break_Code = K_Make_Code + 0x80;//���Ķ��� 

	KBCwait4IBE();
	SetPortVal(key_cmd, 0xD2, 1);
	KBCwait4IBE();
	SetPortVal(key_dat, 0xE0, 1);	//д����չ����־��Ϣ

	KBCwait4IBE();
	SetPortVal(key_cmd, 0xD2, 1);//d2��˵��׼��д���ݵ�Output Register�С����ͨ��60hд�뵽Input Register���ֽڻᱻ���뵽Output Register�У��˹��ܱ�����ģ��������Keyboard���͵����ݡ�����жϱ�������ᴥ��һ���жϴ��������忴ԭ���ĵ��� 
	KBCwait4IBE();
	SetPortVal(key_dat, K_Break_Code, 1);
}

StarsControl::StarsControl()
{

}

StarsControl::~StarsControl()
{

}

bool StarsControl::Initalize()
{
	if (!InitializeWinIo())
	{
		MessageBoxA(NULL, "WinIo��ʼ��ʧ��", "Warning", MB_OK);
		return false;
	}
	return true;
}

bool StarsControl::Finitalize()
{
	ShutdownWinIo();
	return true;
}

void StarsControl::Update()
{
	//MakeKeyDown('A');
	//MakeKeyDownEx(VK_RIGHT);
}

void StarsControl::OnKeyDown(DWORD dwKey)
{
	if (dwKey >= VK_LEFT && dwKey <= VK_DOWN)
	{
		MakeKeyDownEx(dwKey);
	}
	else
	{
		MakeKeyDown(dwKey);
	}
}

void StarsControl::OnKeyUp(DWORD dwKey)
{
	if (dwKey >= VK_LEFT && dwKey <= VK_DOWN)
	{
		MakeKeyUpEx(dwKey);
	}
	else
	{
		MakeKeyUp(dwKey);
	}
}


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