#include "Memory.h"

#include <TlHelp32.h>
#include <Psapi.h>


static auto to_lower = [](const std::string& input) -> std::string
{
	std::string str;
	for (auto& c : input)
		str += tolower(c);
	return str;
};


//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------

Memory::Memory() {}

Memory::Memory(const char* name) 
{
	if (!Attach(name, &vmProcess))
	{
		printf("[!][Memory] failed to attach to target process.\n");
		return;
	}
}

Memory::~Memory() 
{
	Detach();
}

bool Memory::Attach(const char* name, procInfo_t* pInfo)
{
	procInfo_t info;
	if (!GetProcessInfoEx(name, &info))
		return false;

	i64_t pEEmem;
	if (!GetProcAddressEx(info.hProc, info.dwModuleBase, "EEmem", pEEmem))
		return false;

	const auto& EE = ReadEx<i64_t>(info.hProc, pEEmem);
	if (!EE)
		return false;

	info.dwEEBase = EE;

	*pInfo = info;

	return true;
}

void Memory::Detach()
{
	vmProcess = procInfo_t();
}


//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------

HANDLE Memory::GetHandle() { return vmProcess.hProc; }

DWORD Memory::GetProcessID() { return vmProcess.dwPID; }

i64_t Memory::GetModuleBase() { return vmProcess.dwModuleBase; }

i64_t Memory::GetEEMemory() { return vmProcess.dwEEBase; }

i64_t Memory::GetAddress(unsigned int offset) { return vmProcess.dwModuleBase + offset; }

procInfo_t Memory::GetProcessInfo() { return vmProcess; }


//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------

bool Memory::ReadMemory(i64_t addr, LPVOID buffer, DWORD szRead)
{
	if (vmProcess.hProc == INVALID_HANDLE_VALUE)
		return false;

	return ReadMemoryEx(vmProcess.hProc, addr, buffer, szRead);
}

bool Memory::ReadString(i64_t addr, std::string& string, DWORD szString)
{
	if (vmProcess.hProc == INVALID_HANDLE_VALUE)
		return false;

	return ReadStringEx(vmProcess.hProc, addr, string, szString);
}

i64_t Memory::ReadPointerChain(i64_t addr, unsigned int offsets[], DWORD count)
{
	if (vmProcess.hProc == INVALID_HANDLE_VALUE)
		return false;

	i64_t base = addr;
	for (int i = 0; i < count; i++)
	{
		base = Read<i64_t>(base);
		base += offsets[i];
	}

	return base;
}

bool Memory::WriteMemory(i64_t addr, LPVOID buffer, DWORD szWrite)
{
	if (vmProcess.hProc == INVALID_HANDLE_VALUE)
		return false;

	SIZE_T size_written{};
	return WriteProcessMemory(vmProcess.hProc, LPVOID(addr), buffer, szWrite, &size_written) && szWrite == size_written;
}

bool Memory::PatchMemory(i64_t addr, const void* buffer, DWORD szWrite, int access)
{
	DWORD oldprotect;																					//	store original protection
	VirtualProtectEx(vmProcess.hProc, LPVOID(addr), szWrite, access, &oldprotect);						//	set memory protection
	bool result = WriteProcessMemory(vmProcess.hProc, LPVOID(addr), buffer, szWrite, nullptr);			//	write bytes to address
	VirtualProtectEx(vmProcess.hProc, LPVOID(addr), szWrite, oldprotect, &oldprotect);					//	restore memory protection
	return result;
}

i64_t Memory::FindPattern(const std::string& sig, i64_t& result, int padding, bool isRelative, opCode_t instruction)
{
	static auto pattern_to_byte = [](const char* pattern)
	{
		const auto start = const_cast<char*>(pattern);
		const auto end = const_cast<char*>(pattern) + strlen(pattern);

		auto bytes = std::vector<int>{};
		for (auto current = start; current < end; ++current)
		{
			if (*current == '?')
			{
				++current;
				bytes.push_back(-1);
			}
			else
			{
				bytes.push_back(strtoul(current, &current, 16));
			}
		}
		return bytes;
	};

	result = 0;

	//	Get .text segment
	i64_t section_base = 0;
	size_t section_size = 0;
	if (!GetSectionHeader(ESECTIONHEADERS::SECTION_TEXT, section_base, section_size))
		return false;

	//	get pattern
	const auto pattern_bytes = pattern_to_byte(sig.c_str());
	const auto cbSize = pattern_bytes.size();
	const auto cbData = pattern_bytes.data();

	//	read section
	std::vector<unsigned __int8> scan_bytes(section_size);
	if (!ReadMemory(section_base, scan_bytes.data(), scan_bytes.size()))
		return false;

	//	iterate through buffer & compare with pattern
	for (auto i = 0ul; i < section_size - cbSize; ++i)
	{
		bool found = true;
		for (auto j = 0ul; j < cbSize; ++j)
		{
			if (scan_bytes[i + j] != cbData[j] && cbData[j] != -1)
			{
				found = false;
				break;
			}
		}

		if (!found)
			continue;

		//	set result address
		auto address = section_base + i;

		//	apply optional padding
		if (padding != NULL)
			address += padding;

		//	pull offset from instruction
		switch (instruction)
		{
			case opCode_t::OP_NULL: { result = address; break; }
			case opCode_t::OP_MOV: { const auto offset = Read<int>(address + 3); return isRelative ? result = address + offset + 7 : result = address; }
			case opCode_t::OP_CALL: { const auto offset = Read<int>(address + 1); return isRelative ? result = address + offset + 5 : result = address; }
			case opCode_t::OP_LEA: { const auto offset = Read<int>(address + 3); return isRelative ? result = address + offset + 7 : result = address; }
			case opCode_t::OP_CMP: { const auto offset = Read<int>(address + 2); return isRelative ? result = address + offset + 6 : result = address; }
		}

		break;
	}

	return result;
}

bool Memory::GetProcAddress(const std::string& fnName, i64_t& result)
{
	if (vmProcess.hProc == INVALID_HANDLE_VALUE)
		return false;

	return GetProcAddressEx(vmProcess.hProc, vmProcess.dwModuleBase, fnName, result);
}

bool Memory::GetSectionHeader(const ESECTIONHEADERS& section, i64_t& result, size_t& szImage)
{
	if (vmProcess.hProc == INVALID_HANDLE_VALUE)
		return false;

	//	get segment title
	std::string segment;
	switch (section)
	{
		case ESECTIONHEADERS::SECTION_TEXT: { segment = ".text"; break; }
		case ESECTIONHEADERS::SECTION_DATA: { segment = ".data"; break; }
		case ESECTIONHEADERS::SECTION_RDATA: { segment = ".rdata"; break; }
		case ESECTIONHEADERS::SECTION_IMPORT: { segment = ".idata"; break; }
		case ESECTIONHEADERS::SECTION_EXPORT: { segment = ".edata"; break; }
		default: return false;
	}
	if (segment.empty())	//	segment title not captured ?? 
		return false;

	//	get module base
	const auto& dwModuleBase = vmProcess.dwModuleBase;

	//	get dos header
	const auto& image_dos_header = Read<IMAGE_DOS_HEADER>(dwModuleBase);
	if (image_dos_header.e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	//	get dos header
	const auto& e_lfanew = dwModuleBase + image_dos_header.e_lfanew;
	const auto& image_nt_headers = Read<IMAGE_NT_HEADERS>(e_lfanew);
	if (image_nt_headers.Signature != IMAGE_NT_SIGNATURE)
		return false;

	//	Get section
	size_t section_size = 0;
	i64_t section_base = 0;
	const auto& image_section_header = e_lfanew + sizeof(IMAGE_NT_HEADERS);
	IMAGE_SECTION_HEADER section_headers_base = Read<IMAGE_SECTION_HEADER>(image_section_header);
	for (int i = 0; i < image_nt_headers.FileHeader.NumberOfSections; ++i)
	{
		if (strncmp(reinterpret_cast<const char*>(section_headers_base.Name), segment.c_str(), segment.size()) != 0)
		{
			section_headers_base = Read<IMAGE_SECTION_HEADER>(image_section_header + (sizeof(IMAGE_SECTION_HEADER) * i));
			continue;
		}

		section_base = dwModuleBase + section_headers_base.VirtualAddress;
		section_size = section_headers_base.SizeOfRawData;
		break;
	}
	if (!section_base)
		return false;

	//	pass result
	result = section_base;
	szImage = section_size;

	return true;
}


//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------

bool Memory::GetProcessInfoEx(const std::string& name, procInfo_t* lpInfo)
{
	bool bFound{ false };
	procInfo_t process{};
	process.procName = to_lower(name);
	std::wstring procNameW = std::wstring(process.procName.begin(), process.procName.end());

	//	GET PROCESSES
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32 procEntry{};
	procEntry.dwSize = sizeof(procEntry);
	if (!Process32First(hSnap, &procEntry))
	{
		CloseHandle(hSnap);
		return false;
	}

	//	get process id
	do
	{
		// compare string
		if (std::wstring(procEntry.szExeFile) != procNameW)
			continue;

		bFound = true;
		process.dwPID = procEntry.th32ProcessID;
		break;

	} while (Process32Next(hSnap, &procEntry));
	CloseHandle(hSnap);	//	free snapshot resource
	if (!bFound)
		return false;

	//	GET MODULES
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPPROCESS, process.dwPID);
	if (hSnap == INVALID_HANDLE_VALUE)
		return false;

	MODULEENTRY32 modEntry;
	modEntry.dwSize = sizeof(modEntry);
	if (!Module32First(hSnap, &modEntry))
	{
		CloseHandle(hSnap);
		return false;
	}

	//	get module base address
	do
	{
		if (_wcsicmp(modEntry.szModule, procEntry.szExeFile))
			continue;

		process.dwModuleBase = i64_t(modEntry.modBaseAddr);
		break;

	} while (Module32Next(hSnap, &modEntry));
	CloseHandle(hSnap);

	//	Get Main Process Window
	EnumWindowData eDat;
	eDat.procId = process.dwPID;
	if (EnumWindows(GetProcWindowsEx, reinterpret_cast<LPARAM>(&eDat)))
		process.hWnd = eDat.hwnd;

	//	 open handle to process
	process.hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, process.dwPID);
	if (process.hProc == INVALID_HANDLE_VALUE)
		return false;

	*lpInfo = process;

	return true;
}

bool Memory::GetModuleAddressEx(const HANDLE& hProc, const std::string& name, i64_t& lresult)
{
	return false;
}

bool Memory::ReadMemoryEx(const HANDLE& hProc, const i64_t& addr, LPVOID lresult, DWORD szRead)
{
	SIZE_T size_read{};
	return ReadProcessMemory(hProc, LPCVOID(addr), lresult, szRead, &size_read) && szRead == size_read;
}

bool Memory::ReadStringEx(const HANDLE& hProc, const i64_t& addr, std::string& lresult, const DWORD& szString)
{
	char buf[MAX_PATH]{};
	SIZE_T bytes_read{};
	if (!ReadProcessMemory(hProc, LPCVOID(addr), buf, szString, &bytes_read) || bytes_read != szString)
		return false;

	lresult = std::string(buf);

	return true;
}

bool Memory::GetProcAddressEx(const HANDLE& hProc, const i64_t& dwModuleBase, const std::string& fnName, i64_t& lpout)
{
	const auto& fnNameLower = to_lower(fnName);

	//	get image doe header
	auto image_dos_header = ReadEx<IMAGE_DOS_HEADER>(hProc, dwModuleBase);
	if (image_dos_header.e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	//	get nt headers
	const auto& image_nt_headers = ReadEx<IMAGE_NT_HEADERS>(hProc, dwModuleBase + image_dos_header.e_lfanew);
	if (image_nt_headers.Signature != IMAGE_NT_SIGNATURE
		|| image_nt_headers.OptionalHeader.NumberOfRvaAndSizes <= 0)
		return false;

	//	get export directory
	const auto& export_directory_va = image_nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + dwModuleBase;
	const auto& export_directory = ReadEx<IMAGE_EXPORT_DIRECTORY>(hProc, export_directory_va);
	if (!export_directory.AddressOfNames || !export_directory.AddressOfFunctions || !export_directory.AddressOfNameOrdinals)
		return false;

	//	get address of *
	const auto& names_va = dwModuleBase + export_directory.AddressOfNames;
	const auto& functions_va = dwModuleBase + export_directory.AddressOfFunctions;
	const auto& ordinals_va = dwModuleBase + export_directory.AddressOfNameOrdinals;
	for (int i = 0; i < export_directory.NumberOfNames; i++)
	{
		//	get address of name
		const auto& name_rva = ReadEx<DWORD>(hProc, names_va + (i * 0x4));
		const auto& name_va = name_rva + dwModuleBase;

		//	read & compare name with input string
		std::string cmp;
		if (!ReadStringEx(hProc, name_va, cmp))
			continue;

		//	compare strings
		if (fnNameLower != to_lower(cmp))
			continue;

		//	get function address
		const auto& name_ordinal = ReadEx<short>(hProc, ordinals_va + (i * 0x2));				//	get ordinal at the current index
		const auto& function_rva = ReadEx<DWORD>(hProc, functions_va + (name_ordinal * 0x4));	//	get function rva

		//	
		lpout = i64_t(function_rva + dwModuleBase);

		return true;
	}

	return false;
}

BOOL CALLBACK Memory::GetProcWindowsEx(HWND window, LPARAM lParam)
{
	auto data = reinterpret_cast<EnumWindowData*>(lParam);

	DWORD windowPID;
	GetWindowThreadProcessId(window, &windowPID);

	bool isMainWindow = GetWindow(window, GW_OWNER) == (HWND)0 && IsWindowVisible(window);
	if (windowPID != data->procId || !isMainWindow)
		return true;

	data->hwnd = window;

	return true;
}