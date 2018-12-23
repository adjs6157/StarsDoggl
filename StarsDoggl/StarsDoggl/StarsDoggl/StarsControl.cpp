#include "StarsControl.h"
#include <windows.h>
#include "winio.h"

#define key_cmd 0x64//键盘命令端口

#define key_dat 0x60//键盘数据端口

void KBCwait4IBE() {

	DWORD ch = 0; //注意，在这循环，我并没有改变键盘命令端口的数据，但也不会出现死循环，/ 因为键盘中断会在中间把数据读取并置零，那就不会导致死循环了

	do {
		GetPortVal(key_cmd, &ch, 1);//读取键盘的命令端口，得出ch

	} while (ch & 0x2);//bit1是1的话，说明输入缓冲器已满，反复检测！直到空

}

void MakeKeyDown(DWORD VirtualKey)
{
	DWORD K_Make_Code = MapVirtualKey(VirtualKey, 0);//winuser. h里面定义好的函数！
	KBCwait4IBE();
	SetPortVal(key_cmd, 0xD2, 1);//d2是说，准备写数据到Output Register中。随后通过60h写入到Input Register的字节会被放入到Output Register中，此功能被用来模拟来自于Keyboard发送的数据。如果中断被允许，则会触发一个中断处理。（具体看原理文档） 
	KBCwait4IBE();
	SetPortVal(key_dat, K_Make_Code, 1);
} //键 放开 

void MakeKeyUp(DWORD VirtualKey)
{
	DWORD K_Make_Code = MapVirtualKey(VirtualKey, 0);//键的扫描码 
	DWORD K_Break_Code = K_Make_Code + 0x80;//键的断码 
	KBCwait4IBE(); 
	SetPortVal(key_cmd, 0xD2, 1);//d2是说，准备写数据到Output Register中。随后通过60h写入到Input Register的字节会被放入到Output Register中，此功能被用来模拟来自于Keyboard发送的数据。如果中断被允许，则会触发一个中断处理。（具体看原理文档） 
	KBCwait4IBE();
	SetPortVal(key_dat, K_Break_Code, 1);
}

// 扩展键
void MakeKeyDownEx(DWORD VirtualKey)
{
	DWORD K_Make_Code = MapVirtualKey(VirtualKey, 0);//winuser. h里面定义好的函数！

	KBCwait4IBE();
	SetPortVal(key_cmd, 0xD2, 1);
	KBCwait4IBE();
	SetPortVal(key_dat, 0xE0, 1);	//写入扩展键标志信息

	KBCwait4IBE();
	SetPortVal(key_cmd, 0xD2, 1);//d2是说，准备写数据到Output Register中。随后通过60h写入到Input Register的字节会被放入到Output Register中，此功能被用来模拟来自于Keyboard发送的数据。如果中断被允许，则会触发一个中断处理。（具体看原理文档） 
	KBCwait4IBE();
	SetPortVal(key_dat, K_Make_Code, 1);
} //键 放开 

void MakeKeyUpEx(DWORD VirtualKey)
{
	DWORD K_Make_Code = MapVirtualKey(VirtualKey, 0);//键的扫描码 
	DWORD K_Break_Code = K_Make_Code + 0x80;//键的断码 

	KBCwait4IBE();
	SetPortVal(key_cmd, 0xD2, 1);
	KBCwait4IBE();
	SetPortVal(key_dat, 0xE0, 1);	//写入扩展键标志信息

	KBCwait4IBE();
	SetPortVal(key_cmd, 0xD2, 1);//d2是说，准备写数据到Output Register中。随后通过60h写入到Input Register的字节会被放入到Output Register中，此功能被用来模拟来自于Keyboard发送的数据。如果中断被允许，则会触发一个中断处理。（具体看原理文档） 
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
		MessageBoxA(NULL, "WinIo初始化失败", "Warning", MB_OK);
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