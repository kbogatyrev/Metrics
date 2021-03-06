// Wrap.cpp: определяет экспортированные функции для приложения DLL.
//
#include "stdafx.h"
#include <boost/python.hpp>
#include <boost/python/exception_translator.hpp>

#include "poem.h"
#include "ReadTxts.h"

using namespace boost::python;
using namespace std;


wstring StrFromCES(CEString CES)
{
	wstring pWF = CES;
	return pWF;
}
CEString StrToCES(const wstring wStr)
{
	CEString sString(wStr.c_str());
	return sString;
}


class PoemWrap{
public:
	PoemWrap() : pPoem(NULL)
	{}
	PoemWrap(wstring sSongText, const wstring& sPath)
	{
		pPoem = new Poem(StrToCES(sSongText), StrToCES(sPath));
	}
public:
	Poem * pPoem;

};
BOOST_PYTHON_MODULE(Wrap)
{
	class_<PoemWrap>("Poem");
}