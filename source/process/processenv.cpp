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

	Handle ProcessBasic::GetProcessHandle(uint32 Index, uint32 dwDesiredAccess, uint32 bInheritHandle) const
	{
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
		if (Handle hTaskHandler = this->GetProcessHandle(Index); 0 != hTaskHandler)
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
				case WAIT_OBJECT_0:		OutputMessage(TEXT("ÐÅºÅÏìÓ¦\n"));							break;
				case WAIT_FAILED:		OutputMessage(TEXT("ÐÅºÅÊ§°Ü:%d\n"), GetLastError());		break;
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