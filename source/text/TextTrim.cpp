#include <TextTrim.h>
#include <debuger.h>
namespace Cry
{
	uint32 TextTrim::StringHash(const std::string& str)
	{
		uint32 hansh = 0;
		for (auto & Args : str)
		{
			hansh = hansh * 0xf4243 ^ Args;
		}
		return hansh;
	}
	bool TextTrim::TrimText(std::xstring& source, const std::xstring& target, uint32 length)
	{
		if (false == source.empty() && false == target.empty())
		{
			return false == source.erase(source.find(target), length).empty();
		}
		return false;
	}
	void TextTrim::TrimTextSpace(std::xstring& sourceString)
	{
		
	}
	void TextTrim::TrimTextAllSpace(std::xstring& sourceString)
	{

	}
}