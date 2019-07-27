#include <TextConve.h>
namespace Cry
{
	bool A2WHelper(LPWString lpw, LPString lpa, int32 nChars, uint32 acp)
	{
		if (nullptr == lpw || nullptr == lpa)
		{
			return false;
		}
		if (0 == MultiByteToWideChar(acp, 0, lpa, -1, lpw, nChars))
		{
			return false;
		}
		return true;
	}

	bool W2AHelper(LPString lpa, LPWString lpw, int32 nChars, uint32 acp)
	{
		if (nullptr == lpw || nullptr == lpa)
		{
			return false;
		}
		if (0 == WideCharToMultiByte(acp, 0, lpw, -1, lpa, nChars, NULL, NULL))
		{
			return false;
		}
		return true;
	}

	bool A2W(LPWString lpw, LPString lpa, int32 nChars)
	{
		return A2WHelper(lpw, lpa, nChars + 1, CP_ACP);
	}

	bool W2A(LPString lpa, LPWString lpw, int32 nChars)
	{
		return W2AHelper(lpa, lpw, nChars + 1, CP_OEMCP);
	}
}