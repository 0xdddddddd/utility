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

	Handle ProcessHelper::GetProcessHandle(uint32 Index, uint32 dwDesiredAccess, uint32 bInheritHandle) const
	{
		return OpenProcess(dwDesiredAccess, bInheritHandle, Index);
	}

	bool ProcessHelper::GetDirectory(String ResultString[256], const std::xstring& lpWindowName, const std::xstring& lpClassName, const std::xstring& lpszProcessName) const
	{
		return this->GetDirectory(this->GetProcessIndex(lpWindowName, lpClassName, lpszProcessName), ResultString);
	}
	bool ProcessHelper::GetDirectory(uint32 Index, PString ResultString, ulong StrSize) const
	{
		if (0 == Index)
		{
			return false;
		}
		if (Handle hHandle = GetProcessHandle(Index, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ); 0 != hHandle)
		{
			CloseHandleEx CloseHandle(hHandle);
			{
				String DosDirectoryName[8], DosDirectoryFull[256];

				if (0 == GetProcessImageFileName(hHandle, DosDirectoryFull, StrSize))
				{
					return false;
				}

				if (0 == GetLogicalDriveStrings(_countof(DosDirectoryName), DosDirectoryName))
				{
					return false;
				}

				for (String Drive = 'A'; Drive <= 'Z'; ++Drive)
				{
					DosDirectoryName[0] = Drive;
					DosDirectoryName[1] = ':';
					DosDirectoryName[2] = '\0';

					if (0 == QueryDosDevice(DosDirectoryName, ResultString, StrSize))
					{
						continue;
					}

					if (0 == _tcsnicmp(ResultString, DosDirectoryFull, lstrlen(ResultString)))
					{
						lstrcpy(DosDirectoryFull, DosDirectoryFull + lstrlen(ResultString));
						lstrcpy(ResultString, DosDirectoryName);
						lstrcat(ResultString, DosDirectoryFull);
						return GetFileAttributes(ResultString);
					}
				}
				return QueryFullProcessImageName(hHandle, 0, ResultString, &StrSize);
			}
		}
		return false;
	}

	bool ProcessHelper::GetDirectory(String ResultString[256], const std::xstring& lpszProcessName, uint32 th32ProcessID)
	{
		Handle hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, th32ProcessID);
		if (INVALID_HANDLE_VALUE == hSnapshot) {
			return false;
		}
		return [&](Handle hHandle, const std::xstring& lpszProcessName, uint32 ProcessIndex) -> bool {
			CloseHandleEx CloseHandle(hHandle);
			{
				PROCESSENTRY32 ProcessEntry = { sizeof(ProcessEntry) };
				for (int32 Result = Process32First(hHandle, &ProcessEntry); Result; Result = Process32Next(hHandle, &ProcessEntry))
				{
					if (0 == _tcsicmp(ProcessEntry.szExeFile, lpszProcessName.c_str()))
					{
						return this->GetDirectory(0 == ProcessIndex ? (ProcessEntry.th32ProcessID == ProcessIndex ? ProcessIndex : ProcessEntry.th32ProcessID) : ProcessIndex, ResultString);
					}
				}
				return false;
			}
		}(hSnapshot, lpszProcessName, th32ProcessID);
	}

	uint32 ProcessHelper::GetProcessIndex(const std::xstring& lpWindowName, const std::xstring& lpClassName, const std::xstring& lpszProcessName) const
	{
		ulong ProcIndex;
		if (0 == GetWindowThreadProcessId(FindWindow(lpClassName.c_str(), lpWindowName.c_str()), &ProcIndex)) {

			return true == lpszProcessName.empty() ? 0 : this->GetProcessIndex(lpszProcessName, TH32CS_SNAPPROCESS);
		}
		return ProcIndex;
	}

	uint32 ProcessHelper::GetProcessIndex(const std::xstring& lpszProcessName, uint32 dwFlags, uint32 th32ProcessID) const
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