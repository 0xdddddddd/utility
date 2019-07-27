#pragma once
#ifndef _PROCESSRESTART_H_
#define _PROCESSRESTART_H_
#include <misc_config.h>
namespace Cry
{
	class MISC_EXPORTS ApplicationRestart
	{
	public:
		explicit ApplicationRestart() = default;
	public:
		static bool InitApplication(PString CommandLine);
	};
}
#endif