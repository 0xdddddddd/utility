#pragma once
#ifndef _TEXTCONVE_H_
#define _TEXTCONVE_H_
#include <misc_config.h>
namespace Cry
{
	bool A2WHelper(LPWString lpw, LPString lpa, int32 nChars, uint32 acp);
	bool W2AHelper(LPString lpa, LPWString lpw, int32 nChars, uint32 acp);
	extern bool MISC_EXPORTS A2W(LPWString lpw, LPString lpa, int32 nChars);
	extern bool MISC_EXPORTS W2A(LPString lpa, LPWString lpw, int32 nChars);
}
#endif