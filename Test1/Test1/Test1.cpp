// Test1.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "poem.h"
#include "ReadTxts.h"

using namespace std;
using namespace Hlib;


int _tmain(int argc, _TCHAR* argv[])
{
	CEString sText = readFromTxt(L"C:\\git-repos\\Metrics\\folk_Мельница_Зов крови.txt");
	Poem * pP = new Poem(sText, NULL);
	return 0;
}

