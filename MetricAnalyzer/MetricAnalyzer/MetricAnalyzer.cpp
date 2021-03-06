// MetricAnalyzer.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "poem.h"
#include "ReadTxts.h"

using namespace std;
using namespace Hlib;

#include <boost/filesystem.hpp>

namespace fs = ::boost::filesystem;

// return the filenames of all files that have the specified extension
// in the specified directory and all subdirectories
void vGetAll(const fs::path& root, const string& ext, vector<wstring>& ret)
{
	if (!fs::exists(root) || !fs::is_directory(root)) return;

	fs::recursive_directory_iterator it(root);
	fs::recursive_directory_iterator endit;

	while (it != endit)
	{
		if (fs::is_regular_file(*it) && it->path().extension() == ext) 
			ret.push_back(it->path().wstring());
		++it;

	}

}

extern "C"
{
	ET_ReturnCode GetDictionary(IDictionary *& d);
}

int _tmain(int argc, _TCHAR* argv[])
{
	typedef pair<unsigned int, enumStress> stressPos;
	vector<wstring> poems;
	fs::path pRoot(L"C:\\git-repos\\Metrics\\TestData\\Ямб");
	vGetAll(pRoot, ".txt", poems);
	int iFalseBest = 0;
	int iUnident = 0;
	int iErrors = 0;
	int iAllVerses = 0;
	for (vector<wstring>::iterator itFile = poems.begin(); itFile != poems.end(); itFile++){
		CEString sText = readFromTxt(*itFile);
		Poem * pP = new Poem(sText, L"C:\\git-repos\\Phelp\\zal.db3");
		iAllVerses += pP->m_Verses.size();
		for (int i = 0; i < pP->m_Verses.size(); i++){
			if (pP->m_Verses[i]->m_Best->m_eClausula == HYPERDACTYLIC)
				cout << i << endl;
			/*bool b = false;
			if (pP->m_Verses[i]->m_Best->m_eMeter != 0 && pP->m_Verses[i]->m_Best->m_eMeter == ANAPEST){
				cout << i << endl;
				iFalseBest++;
			}
			for (vector<stressPos>::iterator it = pP->m_Verses[i]->m_Matrix.begin(); it != pP->m_Verses[i]->m_Matrix.end(); ++it){
				stressPos n = *it;
				if (n.second == UNKNOWN)
					b = true;
			}
			if (b)
				++iUnident;
			else if (pP->m_Verses[i]->m_Best->m_eMeter == 0){
				iErrors += 1;
			}*/
		}
	}
	return 0;
}
