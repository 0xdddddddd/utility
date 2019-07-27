#include <processrestart.h>
#include <TextConve.h>
namespace Cry
{
	bool ApplicationRestart::InitApplication(PString CommandLine)
	{
		// HRESULT hResult = S_OK;
#if defined(UNICODE) || defined(_UNICODE)
		return SUCCEEDED(RegisterApplicationRestart(CommandLine, 0));
#else
		wChar CommandLineBuffer[256] = {0 };
		{
			if (A2W(CommandLineBuffer, CommandLine, lstrlen(CommandLine)))
			{
				return SUCCEEDED(RegisterApplicationRestart(CommandLineBuffer, 0));
			}
		}
#endif
		return false;
	}
}