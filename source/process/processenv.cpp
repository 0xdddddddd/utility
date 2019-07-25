#include <processenv.hpp>
namespace Cry
{
	bool ProcessHelper::CommandLineArgs(const std::xstring& CommandLine, std::vector<std::xstring>&& v)
	{
		String Buffer[256];
		bool JumpSpace = false;
		for (uint32 i = 0, n = 0, j = 0; i < CommandLine.length(); ++i)
		{
			if (j == 2)
			{
				Buffer[n] = '\0';
				v.push_back(Buffer);
				n = 0;
				JumpSpace = true;
			}
			if ('"' == CommandLine[i])
			{
				++j;
				continue;
			}
			if (false == JumpSpace)
			{
				Buffer[n++] = CommandLine[i];
				continue;
			}
			switch (CommandLine[i])
			{
			case ' ':
			case '-':
			case '"':
			case '\t':
			case '\n':
			case '\r':
			{
				break;
			}
			default:
			{
				JumpSpace = false;
				j = 0;
				break;
			}
			}
		}
		return v.empty() == false;
	}

	Handle ProcessHelper::GetProcessHandle(uint32 Index, uint32 dwDesiredAccess, uint32 bInheritHandle)
	{
		return OpenProcess(dwDesiredAccess, bInheritHandle, Index);
	}

	std::xstring ProcessHelper::GetDirectory(const std::xstring& lpWindowName, const std::xstring& lpClassName, const std::xstring& lpszProcessName)
	{
		return this->GetDirectory(this->GetProcessIndex(lpWindowName, lpClassName, lpszProcessName));
	}
	std::xstring ProcessHelper::GetDirectory(uint32 Index)
	{
		if (0 == Index)
		{
			return std::move(std::xstring());
		}
		if (Handle hHandle = GetProcessHandle(Index, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ))
		{
			CloseHandleEx CloseHandle(hHandle);
			{
				ulong uResult;
				Hinstance hModule;
				String lpszFullDirectory[256];
				if (EnumProcessModules(hHandle, &hModule, sizeof(hModule), &uResult))
				{
					if (GetModuleFileNameEx(hHandle, hModule, lpszFullDirectory, 256) > 0)
					{
						return std::move(std::xstring(lpszFullDirectory));
					}
				}
				if (QueryFullProcessImageName(hHandle, 0, lpszFullDirectory, &uResult))
				{
					return std::move(std::xstring(lpszFullDirectory));
				}
			}
		}
		return std::move(std::xstring());
	}

	std::xstring ProcessHelper::GetDirectory(const std::xstring& lpszProcessName, uint32 th32ProcessID)
	{
		Handle hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, th32ProcessID);
		if (INVALID_HANDLE_VALUE == hSnapshot) {
			return std::move(std::xstring());
		}
		return std::move([&](Handle hHandle, const std::xstring& lpszProcessName, uint32 ProcessIndex) -> std::xstring {
			CloseHandleEx CloseHandle(hHandle);
			{
				PROCESSENTRY32 ProcessEntry = { sizeof(ProcessEntry) };
				for (int32 Result = Process32First(hHandle, &ProcessEntry); Result; Result = Process32Next(hHandle, &ProcessEntry))
				{
					if (0 == _tcsicmp(ProcessEntry.szExeFile, lpszProcessName.c_str()))
					{
						return std::move(std::xstring(ProcessEntry.szExeFile));
					}
				}
				return std::move(this->GetDirectory(0 == ProcessIndex ? (ProcessEntry.th32ParentProcessID == ProcessIndex ? ProcessIndex : ProcessEntry.th32ParentProcessID) : ProcessIndex));
			}
		}(hSnapshot, lpszProcessName, th32ProcessID));
	}

	uint32 ProcessHelper::GetProcessIndex(const std::xstring& lpWindowName, const std::xstring& lpClassName, const std::xstring& lpszProcessName)
	{
		ulong ProcIndex;
		if (0 == GetWindowThreadProcessId(FindWindow(lpClassName.c_str(), lpWindowName.c_str()), &ProcIndex)) {

			return true == lpszProcessName.empty() ? 0 : this->GetProcessIndex(lpszProcessName, TH32CS_SNAPPROCESS);
		}
		return ProcIndex;
	}

	uint32 ProcessHelper::GetProcessIndex(const std::xstring& lpszProcessName, uint32 dwFlags, uint32 th32ProcessID)
	{
		Handle hSnapshot = CreateToolhelp32Snapshot(dwFlags, th32ProcessID);
		if (INVALID_HANDLE_VALUE == hSnapshot) {
			return 0;
		}
		return [](Handle hHandle, const std::xstring& lpszProcessName) -> uint32 {
			CloseHandleEx CloseHandle(hHandle);
			{
				PROCESSENTRY32 ProcessEntry = { sizeof(ProcessEntry) };
				for (int32 Result = Process32First(hHandle, &ProcessEntry); Result; Result = Process32Next(hHandle, &ProcessEntry))
				{
					if (0 == _tcsicmp(ProcessEntry.szExeFile, lpszProcessName.c_str()))
					{
						return ProcessEntry.th32ProcessID;
					}
				}
				return 0;
			}
		}(hSnapshot, lpszProcessName);
	}
}