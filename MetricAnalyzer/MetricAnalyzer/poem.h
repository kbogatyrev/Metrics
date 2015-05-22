#include"stdafx.h"

#include <vector>
#include<iterator>
#include <assert.h> 

#include "Enums.h"
#include "EString.h"

#include "IDictionary.h"
#include "ILexeme.h"
#include "IWordForm.h"
#include "IParser.h"


using namespace std;
using namespace Hlib;

typedef enum enumMeter{
	UNIDENTIFIED,
	TROCHEE,
	IAMBUS,
	DACTYL,
	AMPHIBRACH,
	ANAPEST
} enumMeter;

typedef enum enumStress{
	UNKNOWN = 0,
	UNSTRESSED = 11,
	STRESSED,
	SECONDARY,
	DOUBLE_STRESSED,
	DOUBLE_SECONDARY
} enumStress;

typedef enum enumClausula{
	C_UNKNOWN = 0,
	MASCULINE = 101,
	FEMININE,
	DACTYLIC,
	HYPERDACTYLIC
} enumClausula;

typedef pair<unsigned int, enumStress> stressPos;

class Prediction{
public:
	Prediction(enumMeter eMeter, enumClausula eClaus, unsigned int uiNum, unsigned int uiPen);

	bool operator ==(Prediction &Right);
	bool operator!= (Prediction &Right);

	bool operator> (Prediction &Right);
	bool operator<= (Prediction &Right);

	bool operator< (Prediction &Right);
	bool operator>= (Prediction &Right);

public:
	vector<stressPos> m_Accentuation;
	enumMeter m_eMeter;
	enumClausula m_eClausula;
	unsigned int m_uiNumOfFeet;
	unsigned int m_uiPenalties;
};

typedef vector<Prediction *> pred_vec;

class Verse{
public:
	Verse();
	Verse(const CEString sText, IParser * pPar);
	Verse(const Verse &CopiedVerse);
	~Verse();

	void ClearPredictions();

	Verse& operator=(const Verse &CopiedVerse);

	//Step One: define the placement of Vowels
	//initializes m_Matrix with proper num of stressPos-pairs
	ET_ReturnCode eDefineVowels();
	ET_ReturnCode eDefineWordEnds();
	//Step Two: make representation of verse in enumStress
	ET_ReturnCode eMakeMatrix();
	//TODO check yo-alternatives for wordform with one ot more analysis
	//TODO deal with cp1252-words
	//FIXME deal with hyphened wordforms

	void eIdentifyMeter();//general function that calls all eCheck(smth)s

	//each function compute the sum of penalties for each metric scheme
	//in case of even one inversion metric scheme is disqualified
	//TODO deal with SECONDARY
	//TODO process three DOUBLEs
	void CheckTrochee();
	void CheckIambus();
	void CheckDactyl();
	void CheckAmphibrach();
	void CheckAnapest();
	
	ET_ReturnCode eTryEYoSwap(CEString sWord, IWordForm *& pWF);//workaround as IParser->eAnalyze works only with proper yo

	vector<stressPos> m_Matrix;
	vector<stressPos> m_wordMatrix;
	map<unsigned int, unsigned int> m_wordSyllables;
	CEString m_sText;
	IParser * m_pParser;
	unsigned int m_uiNumOfSyllables;
	unsigned int m_uiNumOfWords;
	pred_vec m_Predictions;
	
	//FIXME
	Prediction * m_Best;

};

typedef vector<Verse *> verse_vec;

//not sure i need it possible profit: 
//to observe structural pattern if present (f.i. 3Iambus/2Amph/4Iambus/2Amph)
//to make multistage analysis
class Stanza{
private:
	unsigned int uiStart;
	unsigned int uiEnd;
	unsigned int uiNumOfVerses;
};
typedef vector<Stanza *> stanza_vec;


class Poem{
public:
	~Poem();

	Poem();
	//TODO
	Poem(const wstring wFilename, IParser * pPar); 
	//TODO: switch off whitespaces and punctuation to get raw lines for future output
	Poem(CEString songText, IParser * pPar);
	Poem(const wstring wFilename, const CEString& sPath);
	Poem(CEString songText, const CEString& sPath);
	Poem(Poem &CopiedPoem);

	void ClearVerses();

	ET_ReturnCode setParser(const CEString& sPath);

public://FIXME
	verse_vec m_Verses;
	//stanza_vec m_Stanzas;
	CEString m_sRawText;
	IParser * m_pParser;
	IDictionary * m_pDict;
	//need a way to store all rhytmicStructures for future analysis?? or just get it from Stanza/Verse??
};
