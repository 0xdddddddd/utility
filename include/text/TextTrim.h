#pragma once
#ifndef _STRINGCROP_H_
#define _STRINGCROP_H_
#include <misc_config.h>
namespace Cry
{
	class MISC_EXPORTS TextTrim
	{
	public:
		explicit TextTrim() {};
	public:
		uint32 StringHash(const std::string& str);
		bool TrimText(std::xstring& sourceString, const std::xstring& targetString, uint32 length);
		void TrimTextSpace(std::xstring& sourceString);
		void TrimTextAllSpace(std::xstring& sourceString);
	};
}
#endif // !_STRINGCROP_H_
