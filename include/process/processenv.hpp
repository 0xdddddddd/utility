#pragma once
#ifndef _PROCESSENV_H_
#define _PROCESSENV_H_
#include <misc_config.h>
#include <vector>
#include <psapi.h>
#include <TlHelp32.h>
namespace Cry
{
	struct MISC_EXPORTS CloseHandleEx
	{
	public:
		explicit CloseHandleEx(Handle hHandle) :m_hHandle(hHandle) {};
		~CloseHandleEx() { CloseHandle(m_hHandle); };
	private:
		Handle					m_hHandle;
	};

	class MISC_EXPORTS ProcessHelper
	{
	public:
		explicit ProcessHelper() {};
	public:
		// ��ȡ������̵�������
		bool CommandLineArgs(const std::xstring& CommandLine, std::vector<std::xstring>&& v);
		// ��ȡ���̾��
		inline Handle GetProcessHandle(uint32 Index, uint32 dwDesiredAccess = PROCESS_ALL_ACCESS, uint32 bInheritHandle = 0);
	public:
		// ��ȡ��������·��
		std::xstring GetDirectory(const std::xstring& lpWindowName, const std::xstring& lpClassName = std::xstring(), const std::xstring& lpszProcessName = std::xstring());
		std::xstring GetDirectory(uint32 Index);
		std::xstring GetDirectory(const std::xstring& lpszProcessName, uint32 th32ProcessID = 0);
	public:
		// ���ݴ��ڱ����������ȡ����ProcessIndex
		uint32 GetProcessIndex(const std::xstring& lpWindowName, const std::xstring& lpClassName = std::xstring(), const std::xstring& lpszProcessName = std::xstring());
		// ����ϵͳ���ղ��ҽ�������ȡ����ProcessIndex
		uint32 GetProcessIndex(const std::xstring& lpszProcessName, uint32 dwFlags = TH32CS_SNAPPROCESS, uint32 th32ProcessID = 0);
	};
}
#endif