/********************************************************************
	created:	2010/12/06
	filename: 	D:\0day\HelloWorld\helloworld.c
	author:		shineast
	purpose:	Hello world driver demo 
*********************************************************************/
#include <ntddk.h>
#define DEVICE_NAME L"\\Device\\HelloWorld"
#define DEVICE_LINK L"\\DosDevices\\HelloWorld"
//�������豸����ָ��
PDEVICE_OBJECT g_DeviceObject;
/**********************************************************************
 ����ж�غ���
	���룺���������ָ��
	�������
**********************************************************************/
VOID DriverUnload( IN PDRIVER_OBJECT  driverObject )
{
	//ʲô��������ֻ�Ǵ�ӡһ�仰
	KdPrint(("DriverUnload: 88!\n"));
} 
/**********************************************************************
 ������ǲ���̺���
	���룺���������ָ��,Irpָ��
	�����NTSTATUS���͵Ľ��
**********************************************************************/
NTSTATUS DrvDispatch(IN PDEVICE_OBJECT driverObject,IN PIRP pIrp)
{ 
	KdPrint(("Enter DrvDispatch\n"));
	//����IRP״̬
	pIrp->IoStatus.Status=STATUS_SUCCESS;
	//����IRP�����ֽ���
	pIrp->IoStatus.Information=0;
	//���IRP�Ĵ���
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
/*********************************************************************
 ������ں������൱��main������
	���룺���������ָ�룬��������Ӧ��ע���·��
	�����NTSTATUS���͵Ľ��
**********************************************************************/
NTSTATUS DriverEntry( IN PDRIVER_OBJECT  driverObject, IN PUNICODE_STRING  registryPath )
{ 
	NTSTATUS       ntStatus;
	UNICODE_STRING devName;
	UNICODE_STRING symLinkName;
	int i=0; 
	//��ӡһ�������Ϣ
	KdPrint(("DriverEntry: Hello world driver demo!\n"));
	//���ø����������ж�غ���
	//driverObject->DriverUnload = DriverUnload; 
	//�����豸 
	RtlInitUnicodeString(&devName,DEVICE_NAME);
	ntStatus = IoCreateDevice( driverObject,
		0,
		&devName,
		FILE_DEVICE_UNKNOWN,
		0, TRUE,
		&g_DeviceObject );
	if (!NT_SUCCESS(ntStatus))
	{
		return ntStatus;  
	}
	//������������  
	RtlInitUnicodeString(&symLinkName,DEVICE_LINK);
	ntStatus = IoCreateSymbolicLink( &symLinkName,&devName );
	if (!NT_SUCCESS(ntStatus)) 
	{
		IoDeleteDevice( g_DeviceObject );
		return ntStatus;
	}
	//���ø������������ǲ���̺���
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		driverObject->MajorFunction[i] = DrvDispatch;
	}
	//���سɹ����
	return STATUS_SUCCESS;
}