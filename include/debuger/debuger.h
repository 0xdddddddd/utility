#pragma once
#ifndef _DEBUGER_H_
#define _DEBUGER_H_
#include <misc_config.h>

namespace Cry
{
	extern MISC_EXPORTS void OutputMessage(PCString strOutputString, ...);
	extern MISC_EXPORTS void OutputMessageEx(const char* strOutputString, ...);
	extern MISC_EXPORTS void OutputMessageEx(const wchar_t* strOutputString, ...);
}
#endif