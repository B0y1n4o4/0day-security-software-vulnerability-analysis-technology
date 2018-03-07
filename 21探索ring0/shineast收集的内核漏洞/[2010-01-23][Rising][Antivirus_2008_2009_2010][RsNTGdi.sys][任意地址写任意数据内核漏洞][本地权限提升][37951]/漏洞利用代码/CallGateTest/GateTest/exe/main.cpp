#include "windows.h"
#include "winioctl.h"
#include "stdio.h"

#define FILE_DEVICE_GATETEST 0x800

const DWORD IOCTL_GATETEST_OPERATION = CTL_CODE(FILE_DEVICE_GATETEST, 0x01, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA);

HANDLE g_hDev;

void WriteDev(LPVOID Address,
              DWORD Value);

void DoInRing0()
{
    WORD farJmp[3];
    DWORD oldCs;

    farJmp[0] = 0;
    farJmp[1] = 0;
    farJmp[2] = 0x3e0;
    
    __asm
    {
        _emit  0xcc;
        call fword ptr [farJmp];    //��ת��������
        //����R0��ִ������д���retn

        mov eax, [esp];     //eax = cs selector
        mov oldCs, eax;
        add esp, 4
    }

    //�Ѿ���R0�ˡ�
    *(DWORD*)(0x8003f000) = 0x11111111;     //��һ���ں˿ռ俴�Ƿ��ܳɹ�

    //����R3
    __asm{
        mov eax, oldCs;
        push eax;
        mov eax, JmpToR3;
        push eax;
        retf;
    }

JmpToR3:
    __asm nop;
}

void AddCallGate()
{
    DWORD addrGdt;
    WORD callGate[4];
    BYTE segment[8];

    __asm 
    {
        push eax
        sgdt [esp - 2]      //���gdt��ַ
        pop eax
        mov addrGdt, eax
    }

    //������GDT��0ƫ�ƴ�, д��retnָ��
    WriteDev((LPVOID)addrGdt,
              0xc3);

    //���һ��������
    //ָ��ƫ��ָ������д���retn
    //�α�־ָ���Լ������Ķ�ѡ����
    callGate[0] = (WORD)(addrGdt & 0x0000FFFF);
    callGate[3] = (WORD)(addrGdt >> 16);            //ָ��ƫ��
    callGate[1] = 0x3e8;      //��ѡ����ƫ��
    callGate[2] = 0xec00;     //dpl = 3

    //���һ����ѡ����
    *(WORD*)(&segment[0]) = 0xffff;     //�ν���4G
    *(WORD*)(&segment[2]) = 0x0000;     //�λ�ַ0x00000000
    segment[4] = 0x00;
    segment[7] = 0x00;
    segment[5] = 0x9a;
    segment[6] = 0xcf;

    //�������źͶ�ѡ��������д��GDT��, 0x3e0λ
    WriteDev((LPVOID)(addrGdt + 0x3e0),
             *(DWORD*)callGate);
    WriteDev((LPVOID)(addrGdt + 0x3e4),
             *((DWORD*)callGate + 1));
    WriteDev((LPVOID)(addrGdt + 0x3e8),
             *(DWORD*)segment);
    WriteDev((LPVOID)(addrGdt + 0x3ec),
             *((DWORD*)segment + 1));
}

void WriteDev(LPVOID Address,
              DWORD Value)
{
    BOOL ret;
    DWORD dwWritten;

    ret = DeviceIoControl(g_hDev,
                          IOCTL_GATETEST_OPERATION,
                          &Value,
                          sizeof(Value),
                          Address,
                          sizeof(Value),
                          &dwWritten,
                          NULL);
}

int main(int argc, char** argv)
{
    g_hDev = CreateFile("\\\\.\\GATETEST_DeviceName",
                      FILE_READ_DATA | FILE_WRITE_DATA,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      0,
                      OPEN_EXISTING,
                      0, 0);
    if(g_hDev == INVALID_HANDLE_VALUE)
    {
        printf("can not open device");
        return 0;
    }

    AddCallGate();
    DoInRing0();
    return 0;
}