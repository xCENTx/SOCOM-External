#pragma once
#include <windows.h>
#include <memory>
#include <string>
#include <vector>


#ifdef _WIN64
typedef unsigned long long	i64_t;
#else
typedef unsigned int		i64_t;
#endif

/*
	
*/
struct PROCESSINFO
{
	HWND			hWnd{ 0 };								//	handle to process window
	HANDLE			hProc{ INVALID_HANDLE_VALUE };			//	handle to process
	DWORD			dwPID{ 0 };								//	process id
	i64_t			dwModuleBase{ 0 };						//	base address of module "magic number"
	i64_t			dwEEBase{ 0 };							//	base address of EEMemory
	std::string		procName{ "" };							//	name of attached process
}; typedef PROCESSINFO procInfo_t; 

enum class EASM : int
{
	OP_NULL = 0,
	OP_MOV,
	OP_LEA,
	OP_CMP,
	OP_CALL
}; typedef EASM opCode_t;

enum class ESECTIONHEADERS : int
{
	SECTION_TEXT = 0,
	SECTION_DATA,
	SECTION_RDATA,
	SECTION_IMPORT,
	SECTION_EXPORT,
}; typedef ESECTIONHEADERS sectionHeaders_t;

/*
	Static Class
*/
class Memory
{
public:
	Memory();	//	 default constructor | does nothing
	Memory(const char* name);
	~Memory();

public:
	static bool Attach(const char* name, procInfo_t* out);
	static void Detach();

public://	returns reference to vmProcess
	static HANDLE GetHandle();
	static DWORD GetProcessID();
	static i64_t GetModuleBase();
	static i64_t GetEEMemory();
	static procInfo_t GetProcessInfo();

public://	core memory operations
	static i64_t GetAddress(unsigned int offset);
	static bool ReadMemory(i64_t addr, LPVOID buffer, DWORD szRead);
	static bool ReadString(i64_t pAddr, std::string& string, DWORD szString = MAX_PATH);
	static i64_t ReadPointerChain(i64_t addr, unsigned int offsets[], DWORD count);
	static bool WriteMemory(i64_t addr, LPVOID buffer, DWORD szWrite);
	static bool PatchMemory(i64_t addr, const void* buffer, DWORD szWrite, int access = PAGE_EXECUTE_READWRITE);
	static i64_t FindPattern(const std::string& signature, i64_t& result, int padding = 0, bool isRelative = false, opCode_t instruction = EASM::OP_NULL);
	static bool GetSectionHeader(const ESECTIONHEADERS& section, i64_t& address, size_t& szImage);
	static bool GetProcAddress(const std::string& fnName, i64_t& result);

public:
	template<typename T>
	static T Read(i64_t addr, DWORD szRead)
	{
		T result{};
		ReadMemory(addr, &result, szRead);
		return result;
	}

	template<typename T>
	static T Read(i64_t addr)
	{
		T result{};
		ReadMemory(addr, &result, sizeof(T));
		return result;
	}

	template<typename T>
	static T Write(i64_t addr, T patch, DWORD szPatch) { WriteMemory(addr, &patch, szPatch); }

	template<typename T>
	static T Write(i64_t addr, T patch) { WriteMemory(addr, &patch, sizeof(T)); }

	//	~

private:
	static bool GetProcessInfoEx(const std::string& name, procInfo_t* lpInfo);
	static bool GetModuleAddressEx(const HANDLE& hProc, const std::string& name, i64_t& lresult);
	static bool ReadMemoryEx(const HANDLE& hProc, const i64_t& addr, LPVOID lresult, DWORD szRead);
	static bool ReadStringEx(const HANDLE& hProc, const i64_t& addr, std::string& lresult, const DWORD& szString = MAX_PATH);

private:
	static bool GetProcAddressEx(const HANDLE& hProc, const i64_t& dwModule, const std::string& fnName, i64_t& lresult);

private:
	template<typename T>
	static auto ReadEx(const HANDLE& hProc, const i64_t& addr) -> T
	{
		T result{};
		ReadMemoryEx(hProc, i64_t(addr), &result, sizeof(T));
		return result;
	}

private://	tools
	struct EnumWindowData
	{
		unsigned int procId;
		HWND hwnd;
	};
	static BOOL CALLBACK GetProcWindowsEx(HWND handle, LPARAM lParam);
};  
inline std::unique_ptr<Memory> g_Memory;
inline procInfo_t vmProcess;