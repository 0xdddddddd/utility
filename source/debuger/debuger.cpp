#include <debuger\debuger.h>

namespace Cry
{
	void OutputMessage(PCString strOutputString, ...)
	{
		String debugMsg[512];
		va_list ap;
		va_start(ap, strOutputString);
		if (_vsntprintf_s(debugMsg, _countof(debugMsg), _TRUNCATE, strOutputString, ap))
		{
			OutputDebugString(debugMsg);
		}
		va_end(ap);
	}
	void OutputMessageEx(const char* strOutputString, ...)
	{
		char debugMsg[512];
		va_list ap;
		va_start(ap, strOutputString);
		if (_vsnprintf_s(debugMsg, _countof(debugMsg), _TRUNCATE, strOutputString, ap))
		{
			OutputDebugStringA(debugMsg);
		}
		va_end(ap);
	}
	void OutputMessageEx(const wchar_t* strOutputString, ...)
	{
		wchar_t debugMsg[512];
		va_list ap;
		va_start(ap, strOutputString);
		if (_vsnwprintf_s(debugMsg, _countof(debugMsg), _TRUNCATE, strOutputString, ap))
		{
			OutputDebugStringW(debugMsg);

		}
		va_end(ap);
	}
}
