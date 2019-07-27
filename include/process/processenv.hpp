#pragma once
#ifndef _PROCESSENV_H_
#define _PROCESSENV_H_
#include <misc_config.h>
#include <native.h>
#include <vector>
#include <psapi.h>
#include <TlHelp32.h>
namespace Cry
{
	struct MISC_EXPORTS CloseHandleEx
	{
	public:
		explicit CloseHandleEx(const Handle& hHandle)
		{
			m_hHandle = hHandle;
		};
		~CloseHandleEx() { NT_SUCCESS(NtClose(m_hHandle)); };
	private:
		Handle					m_hHandle;
	};

	class MISC_EXPORTS ProcessBasic
	{
	public:
		explicit ProcessBasic() {};
	public:
		// ��ȡ������̵�������
		static bool CommandLineArgs(const std::xstring& CommandLine, std::vector<std::xstring>&& v);
		// ����Ȩ��
		static bool AdjustPrivilege();
		// ��ȡ���̾��
		inline Handle GetProcessHandle(uint32 Index, uint32 dwDesiredAccess = PROCESS_ALL_ACCESS, uint32 bInheritHandle = 0) const;
		// ռ�öϵ�
		static bool HideBreakpoint(Handle hThread);
	public:
		// ��ȡ��������·��
		bool GetDirectory(String ResultString[256], const std::xstring& lpWindowName, const std::xstring& lpClassName = std::xstring(), const std::xstring& lpszProcessName = std::xstring()) const;
		bool GetDirectory(String ResultString[256], const std::xstring& lpszProcessName, uint32 th32ProcessID = 0);
		// ��������
		bool KillProcess(uint32 Index) const;
	public:
		// ���ݴ��ڱ����������ȡ����ProcessIndex
		uint32 GetProcessIndex(const std::xstring& lpWindowName, const std::xstring& lpClassName, const std::xstring& lpszProcessName = std::xstring()) const;
		// ����ϵͳ���ղ��ҽ�������ȡ����ProcessIndex
		uint32 GetProcessIndex(const std::xstring& lpszProcessName, uint32 dwFlags = TH32CS_SNAPPROCESS, uint32 th32ProcessID = 0) const;
	private:
		bool GetDirectory(uint32 Index, PString ResultString, ulong StrSize = 256) const;
	};
}
#endif