#include <iostream>
#include <vector>
#include <iterator>
#include <fstream>
#include <string>
#include <locale>
#include <codecvt>

#include "Enums.h"
#include "EString.h"

using namespace std;
using namespace Hlib;

CEString readFromTxt(const wstring filename){
	wifstream streamTxt(filename);
	streamTxt.imbue(locale(locale::empty(), new codecvt_utf8_utf16<wchar_t, 0x10ffff, generate_header>));
	wstring wTxt;
	
	streamTxt.seekg(0, ios::end);
	wTxt.reserve(streamTxt.tellg());
	streamTxt.seekg(0, ios::beg);

	wTxt.assign(istreambuf_iterator<wchar_t>(streamTxt),
					(istreambuf_iterator<wchar_t>()));

	if (wTxt.compare(0, 1, L"\xFEFF") == 0) 
	{ 
		wTxt.erase(0, 1);                  //get rid of the BOM.
	}

	CEString CESTxt(wTxt.c_str());
	CESTxt.SetBreakChars(L"\t\n");
	CESTxt.SetVowels(g_szRusVowels);
	int tokenSize = CESTxt.uiGetNumOfTokens();
	return CESTxt;
}
