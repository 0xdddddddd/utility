#include <processenv.hpp>
#include <debuger\debuger.h>
#include <future>
namespace Cry
{
	bool ProcessBasic::CommandLineArgs(const std::xstring& CommandLine, std::vector<std::xstring>&& v)
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
	bool ProcessBasic::AdjustPrivilege()
	{
		Handle hToken = nullptr;
		if (false == OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		{
			return false;
		}
		if (nullptr == hToken)
		{
			return false;
		}
		CloseHandleEx CloseHandle(hToken);
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (false == LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid))
		{
			return false;
		}
		return AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	}
	Handle ProcessBasic::GetProcessHandle(uint32 Index, uint32 dwDesiredAccess, uint32 bInheritHandle) const
	{
		/*
			PROCESS_TERMINATE			{允许 TerminateProcess 使用进程句柄来关闭进程}
			PROCESS_CREATE_THREAD		{允许 CreateRemoteThread 使用进程句柄来创建线程}
			PROCESS_VM_OPERATION		{允许 VirtualProtectEx 使用进程句柄来改变进程的虚拟内存}
			PROCESS_VM_READ				{允许 ReadProcessMemory 使用进程句柄从进程的虚拟内存中读取数据}
			PROCESS_VM_WRITE			{允许 WriteProcessMemory 使用进程句柄向进程的虚拟内存中写入数据}
			PROCESS_DUP_HANDLE			{允许 DuplicateHandle 把进程句柄当作源句柄或目标句柄进行复制}
			PROCESS_CREATE_PROCESS		{默认值}
			PROCESS_SET_QUOTA			{允许 SetProcessWorkingSetSize 使用进程句柄设置虚拟内存的上限值}
			PROCESS_SET_INFORMATION		{允许 SetPriorityClass 使用进程句柄来设置进程优先级}
			PROCESS_QUERY_INFORMATION	{允许 GetExitCodeProcess 或 GetPriorityClass 通过进程句柄读取进程信息}
			SYNCHRONIZE					{允许任何等待的函数使用进程句柄}
		*/
		return OpenProcess(dwDesiredAccess, bInheritHandle, Index);
	}

	bool ProcessBasic::HideBreakpoint(Handle hThread)
	{
		return NT_SUCCESS(NtSetInformationThread(hThread, ThreadHideFromDebugger, 0, 0));
	}

	bool ProcessBasic::GetDirectory(String ResultString[256], const std::xstring& lpWindowName, const std::xstring& lpClassName, const std::xstring& lpszProcessName) const
	{
		return this->GetDirectory(this->GetProcessIndex(lpWindowName, lpClassName, lpszProcessName), ResultString);
	}
	bool ProcessBasic::GetDirectory(uint32 Index, PString ResultString, ulong StrSize) const
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
					DosDirectoryName[3] = '\0';

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

	bool ProcessBasic::GetDirectory(String ResultString[256], const std::xstring& lpszProcessName, uint32 th32ProcessID)
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

	bool ProcessBasic::KillProcess(uint32 Index) const
	{
		if (Handle hTaskHandler = this->GetProcessHandle(Index, PROCESS_TERMINATE | SYNCHRONIZE); 0 != hTaskHandler)
		{
			CloseHandleEx CloseHandle(hTaskHandler);
			{
				std::future AsyncObject = std::async([](const Handle TaskHandler) -> bool {
					if (NTSTATUS ExitStatus = NtTerminateProcess(TaskHandler, 1); NT_SUCCESS(ExitStatus) || /* An attempt was made to access an exiting process. */ STATUS_PROCESS_IS_TERMINATING == ExitStatus)
					{
						return true;
					}
					return false;
				}, hTaskHandler);
				switch (MsgWaitForMultipleObjectsEx(1, &hTaskHandler, 3000, QS_ALLINPUT, MWMO_INPUTAVAILABLE))
				{
				case WAIT_OBJECT_0:		OutputMessage(TEXT("信号响应\n"));							break;
				case WAIT_FAILED:		OutputMessage(TEXT("信号失败:%d\n"), GetLastError());		break;
				case WAIT_TIMEOUT:																	break;
				default:																			return false;
				}
				return AsyncObject.get();
			}
		}
		return false;
	}

	uint32 ProcessBasic::GetProcessIndex(const std::xstring& lpWindowName, const std::xstring& lpClassName, const std::xstring& lpszProcessName) const
	{
		ulong ProcIndex = 0;
		if (0 == GetWindowThreadProcessId(FindWindow(lpClassName.c_str(), lpWindowName.c_str()), &ProcIndex)) {

			return true == lpszProcessName.empty() ? 0 : this->GetProcessIndex(lpszProcessName, TH32CS_SNAPPROCESS, ProcIndex);
		}
		return ProcIndex;
	}

	uint32 ProcessBasic::GetProcessIndex(const std::xstring& lpszProcessName, uint32 dwFlags, uint32 th32ProcessID) const
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