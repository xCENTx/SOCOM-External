#pragma once
#include <windows.h>
#include <memory>
#include <string>

//	https://github.com/F0bes/pcsx2_offsetreader
FARPROC GetProcAddressEx(HANDLE _Process, HMODULE _Module, LPCSTR _Name);
FARPROC GetProcAddressEx(HANDLE _Process, LPCSTR _Module, LPCSTR _Name);

/*
	
*/
struct PROCESSINFO
{
	HANDLE			hProc{ INVALID_HANDLE_VALUE };			//	handle to process
	DWORD			dwPID{ 0 };								//	process id
	__int64			dwModuleBase{ 0 };						//	base address of module "magic number"
	__int64			dwEEBase{ 0 };							//	base address of EEMemory
	std::string		procName{ "" };							//	name of attached process
}; typedef PROCESSINFO procInfo_t; inline procInfo_t vmProcess;

/*
	Static Class
*/
class Memory
{
public:
	static bool Attach(const char* name, procInfo_t* out);
	static void Detach();
	static HANDLE GetHandle();
	static DWORD GetProcessID();
	static __int64 GetModuleBase();
	static __int64 GetEEMemory();
	static __int64 GetAddress(unsigned int offset);
	static procInfo_t GetProcessInfo();

public:
	static bool ReadVirtualMemory(__int64 addr, LPVOID buffer, DWORD szRead);
	static bool WriteVirtualMemory(__int64 addr, LPVOID buffer, DWORD szWrite);
	static bool ReadVirtualString(__int64 pAddr, std::string& string, DWORD szString = MAX_PATH);
	static __int64 ReadVirtualPointerChain(__int64 addr, unsigned int offsets[], DWORD count);

public:
	template<typename T>
	static T Read(__int64 addr, DWORD szRead)
	{
		T result{};
		ReadVirtualMemory(addr, &result, szRead);
		return result;
	}

	template<typename T>
	static T Read(__int64 addr)
	{
		T result{};
		ReadVirtualMemory(addr, &result, sizeof(T));
		return result;
	}

	template<typename T>
	static T Write(__int64 addr, T patch, DWORD szPatch) { WriteVirtualMemory(addr, &patch, szPatch); }

	template<typename T>
	static T Write(__int64 addr, T patch) { WriteVirtualMemory(addr, &patch, sizeof(T)); }

public:
	Memory();	//	 default constructor | does nothing
	Memory(const char* name);
	~Memory();
};  inline std::unique_ptr<Memory> g_Memory;