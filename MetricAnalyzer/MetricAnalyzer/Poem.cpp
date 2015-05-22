#include "stdafx.h"
#include "Poem.h"

using namespace std;
using namespace Hlib;

extern "C"
{
	ET_ReturnCode GetDictionary(IDictionary *& d);
}

Prediction::Prediction(enumMeter eMeter, enumClausula eClaus, unsigned int uiNum, unsigned int uiPen) :
																						m_eMeter(eMeter),
																						m_eClausula(eClaus),
																						m_uiNumOfFeet(uiNum),
																						m_uiPenalties(uiPen)
{}

// TODO  apply proper return codes for errorlog
ET_ReturnCode Verse::eDefineVowels(){
	unsigned int uiVowel;
	for (unsigned int i = 0; i < m_uiNumOfSyllables; i++){
		uiVowel = m_sText.uiGetVowelPos(i);
		m_Matrix.push_back(stressPos(uiVowel, UNSTRESSED));
	}
	return H_NO_ERROR;
}
ET_ReturnCode Verse::eDefineWordEnds(){
	//workaround for word-ends tracking
	int uiSetVowels = 0;
	for (unsigned int i = 0; i < m_uiNumOfWords; i++){
		CEString sTempWord = m_sText.sGetField(i);
		sTempWord.SetVowels(g_szRusVowels);
		m_wordSyllables.insert(make_pair(i, sTempWord.uiGetNumOfSyllables()));
		for (unsigned int j = 0; j < sTempWord.uiGetNumOfSyllables(); j++){
			m_wordMatrix.push_back(stressPos(i, m_Matrix[uiSetVowels + j].second));
		}
		uiSetVowels += sTempWord.uiGetNumOfSyllables();
	}
	
	return H_NO_ERROR;
}

// TODO apply proper return codes for errorlog
ET_ReturnCode Verse::eMakeMatrix(){
	if (m_pParser == NULL){
		return H_ERROR_POINTER;
	}

	IWordForm *pWF = NULL;
	ET_ReturnCode rc = H_FALSE;
	unsigned int uiSetVowels = 0;
	unsigned int uiWordVowels;
	CEString sWord;
	for (unsigned int i = 0; i < m_uiNumOfWords; i++){
		sWord = m_sText.sGetField(i);
		sWord.SetVowels(g_szRusVowels);
		uiWordVowels = sWord.uiGetNumOfSyllables();
		m_pParser->ClearResults();
		try{
			rc = m_pParser->eAnalyze(sWord);
			rc = m_pParser->eGetFirstWordForm(pWF);
		}
		catch (...)
		{
			for (unsigned int wordInd = 0; wordInd < uiWordVowels; wordInd++){
				m_Matrix[uiSetVowels + wordInd].second = UNKNOWN;
			}
			uiSetVowels += uiWordVowels;
			continue;
		}
		if (rc){
			rc = eTryEYoSwap(sWord, pWF);
			if (rc){
				for (unsigned int wordInd = 0; wordInd < uiWordVowels; wordInd++){
					m_Matrix[uiSetVowels + wordInd].second = UNKNOWN;
				}
				uiSetVowels += uiWordVowels;
				continue;
			}
		}
		int iStress = -1;
		ET_StressType eStress = STRESS_TYPE_UNDEFINED;
		ET_ReturnCode rcStress;
		bool bHasStress = false;
		bool bHasDouble = false;
		// FIXME needed only for threee DOUBLEs checking
		unsigned int uiDoubles = 0;
		
		
		while (rc == H_NO_ERROR){
			rcStress = pWF->eGetFirstStressPos(iStress, eStress);
			//if iStress is not initialized with proper stress position
			//it is still -1 and therefore cannot be equal uiMInd in any case
			if (rcStress == H_NO_ERROR){
				bHasStress = true;
				iStress += m_sText.uiGetFieldOffset(i);
			}
			unsigned int uiMInd;

			for (unsigned int wordInd = 0; wordInd < uiWordVowels; wordInd++){
				uiMInd = wordInd + uiSetVowels;
				if (m_Matrix[uiMInd].first == iStress){
					if (eStress == STRESS_PRIMARY && m_Matrix[uiMInd].second != STRESSED && m_Matrix[uiMInd].second != DOUBLE_STRESSED)
						m_Matrix[uiMInd].second = STRESSED;
					else if (eStress == STRESS_SECONDARY)
						m_Matrix[uiMInd].second = SECONDARY;

					rcStress = pWF->eGetNextStressPos(iStress, eStress);
					if (rcStress == H_NO_ERROR)
						iStress += m_sText.uiGetTokenOffset(i);
				}
				else{
					if (m_Matrix[uiMInd].second == STRESSED){
						m_Matrix[uiMInd].second = DOUBLE_STRESSED;
						bHasDouble = true;
						uiDoubles += 1;// FIXME
					}
					else if (m_Matrix[uiMInd].second == SECONDARY){
						m_Matrix[uiMInd].second = DOUBLE_SECONDARY;
					}
				}
			}
			rc = m_pParser->eGetNextWordForm(pWF);
		}

		if (!bHasStress){
			for (unsigned int wordInd = 0; wordInd < uiWordVowels; wordInd++){
				m_Matrix[uiSetVowels + wordInd].second = UNKNOWN;
			}
		}

		//workaround for double-stressed
		if (bHasDouble){		
			for (unsigned int wordInd = 0; wordInd < uiWordVowels; wordInd++){
				if (m_Matrix[uiSetVowels + wordInd].second == STRESSED){
					m_Matrix[uiSetVowels + wordInd].second = DOUBLE_STRESSED;
					uiDoubles += 1;// FIXME
				}
			} 
		}
		uiSetVowels += uiWordVowels;
		//assert(uiDoubles <= 2);
		if (uiDoubles > 2)
			cout << uiDoubles << " doubles" << endl;
	}
	return rc;
}

ET_ReturnCode Verse::eTryEYoSwap(CEString sWord, IWordForm *& pWF){
	ET_ReturnCode rc = H_FALSE;
	unsigned int uiAt = sWord.uiFind(L"е");

	while (uiAt != ecNotFound && uiAt < sWord.uiLength()){
		sWord.sReplace(uiAt, 1, L"ё");
		m_pParser->ClearResults();
		rc = m_pParser->eAnalyze(sWord);
		rc = m_pParser->eGetFirstWordForm(pWF);
		// TODO can there be multy-"e" word with different yo types?
		if (rc == H_NO_ERROR)
			return rc;
		sWord.sReplace(uiAt, 1, L"е");
		if (uiAt + 1 == sWord.uiLength())
			break;
		uiAt = sWord.uiFind(L"е", ++uiAt);
	}
	return rc;
}

void Verse::eIdentifyMeter(){
	CheckTrochee();
	CheckIambus();
	CheckDactyl();
	CheckAmphibrach();
	CheckAnapest();
	unsigned int uiPrevPen = 1000;
	for (pred_vec::iterator it = m_Predictions.begin(); it != m_Predictions.end(); it++){
		Prediction * Temp = *it;
		if (Temp->m_uiPenalties < uiPrevPen){
			uiPrevPen = Temp->m_uiPenalties;
			m_Best = Temp;
		}
	}
	if (m_Predictions.size() == 0){
		m_Best = new Prediction(UNIDENTIFIED, C_UNKNOWN, 0, 1000);
	}
}

void Verse::CheckIambus(){
	unsigned int uiPenalties = 0;
	bool bChoriambStart = false;
	enumClausula eClaus = C_UNKNOWN;
	for (unsigned int uiInd = 0; uiInd < m_uiNumOfSyllables; uiInd++){
		unsigned int uiWord = m_wordMatrix[uiInd].first;
		// TODO  rethink the penalty-system and try to analyse lines with one unknown word 
		if (m_wordMatrix[uiInd].second == UNKNOWN)
			return;
		if (uiInd % 2){
			//uiInd is odd: stressed
			if (m_wordMatrix[uiInd].second == UNSTRESSED){
				//at this point previous possible choriamb is considered as inversion 
				//therefore add 5 to complete inversion-penalty
				if (bChoriambStart){
					uiPenalties += 5;
					bChoriambStart = false;
				}
				uiPenalties += 1;
				
				if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
					if (eClaus != HYPERDACTYLIC)
						eClaus = DACTYLIC;
			}
			else{
				if (bChoriambStart)
					bChoriambStart = false;

				if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
					eClaus = MASCULINE;
			}
		}
		else{
			//uiInd is even: unstressed
			if (m_wordMatrix[uiInd].second == STRESSED){
				if (m_wordSyllables[uiWord] == 1)
					uiPenalties += 1;
				//for two-syllabic words only allowed in the first two feets
				else if (m_wordSyllables[uiWord] == 2 && uiInd == 0){
					//at this point previous  possible choriamb is considered as inversion 
					//therefore add 5 to complete inversion-penalty
					if (bChoriambStart)
						uiPenalties += 5;
					bChoriambStart = true;
					uiPenalties += 5;
					//no need to analyse next vowel as penalty is added for choriamb
					// FIXME do I need to add penalty for skipped stress??
					uiInd++;
				}
				else
					uiPenalties += 10;
			}
			if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords){
				if (eClaus == DACTYLIC)
					eClaus = HYPERDACTYLIC;
				else
					eClaus = FEMININE;
			}
		}
	}
	if (bChoriambStart)
		uiPenalties += 5;
	if (uiPenalties < 10){
		int iFeet = m_uiNumOfSyllables / 2 - ((eClaus == DACTYLIC || eClaus == HYPERDACTYLIC) ? 1 : 0);
		Prediction * pPred = new Prediction(IAMBUS, eClaus, iFeet, uiPenalties);
		m_Predictions.push_back(pPred);
	}
}

void Verse::CheckTrochee(){
	unsigned int uiPenalties = 0;
	enumClausula eClaus = C_UNKNOWN;
	for (unsigned int uiInd = 0; uiInd < m_uiNumOfSyllables; uiInd++){
		unsigned int uiWord = m_wordMatrix[uiInd].first;
		if (m_wordMatrix[uiInd].second == UNKNOWN)
			return;
		if (uiInd % 2){
			//uiInd is odd: unstressed
			if (m_wordMatrix[uiInd].second == STRESSED){
				//at this point previous  possible choriamb is considered as inversion 
				//therefore add 5 to complete inversion-penalty
				if (m_wordSyllables[uiWord] == 1)
					uiPenalties += 1;
				else
					uiPenalties += 10;
			}
			if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords){
				if (eClaus == DACTYLIC)
					eClaus = HYPERDACTYLIC;
				else
					eClaus = FEMININE;
			}
		}
		else{
			//uiInd is even: stressed
			if (m_wordMatrix[uiInd].second == UNSTRESSED){
				uiPenalties += 1;
				if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
					if (eClaus != HYPERDACTYLIC)
						eClaus = DACTYLIC;
			}
			else {
				if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
					eClaus = MASCULINE;
			}
		}
	}
	if (uiPenalties <= 10){
		int iFeet = (m_uiNumOfSyllables+1) / 2 - ((eClaus == DACTYLIC || eClaus == HYPERDACTYLIC) ? 1 : 0);
		Prediction * pPred = new Prediction(TROCHEE, C_UNKNOWN, iFeet, uiPenalties);
		m_Predictions.push_back(pPred);
	}
}

void Verse::CheckAmphibrach(){
	unsigned int uiPenalties = 0;
	bool bPrevDouble = false;
	bool bIgnoreDoubles = false;
	enumClausula eClaus = C_UNKNOWN;
	for (unsigned int uiInd = 0; uiInd < m_uiNumOfSyllables; uiInd++){
		unsigned int uiWord = m_wordMatrix[uiInd].first;
		if (m_wordMatrix[uiInd].second == UNKNOWN)
			return;
		if (uiInd % 3 == 0){
			//0,3,6... unstressed
			//allowed alternation: [/] / _;  _ / [_ /] / _; 
			if (m_wordMatrix[uiInd].second == STRESSED){
				if (m_wordSyllables[uiWord] == 1)
					uiPenalties += 1;
				else if (m_wordSyllables[uiWord] == 2 && (uiInd != 0 && m_wordMatrix[uiInd - 1].first == uiWord))
					uiPenalties += 1;
				else
					uiPenalties += 10;
			}
			else if (m_wordMatrix[uiInd].second == DOUBLE_STRESSED){
				if (bPrevDouble){
					uiPenalties += 10;
					bPrevDouble = false;
				}
				else if (bIgnoreDoubles)
					bIgnoreDoubles = false;
				else
					bPrevDouble = true;
			}

			if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
				if (eClaus != HYPERDACTYLIC)
					eClaus = DACTYLIC;
		}
		else if (uiInd % 3 == 1){
			//1,4,7... stressed
			//allowed alternations : _ _ _
			if (m_wordMatrix[uiInd].second == UNSTRESSED){
				uiPenalties += 1;
				if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
						eClaus = HYPERDACTYLIC;
			}
			else if (m_wordMatrix[uiInd].second == DOUBLE_STRESSED){
				if (bPrevDouble){
					//considered STRESSED
					//DO NOT FORGET TO MENTION IN TEXT:
					//MY CAUSE WRONG CLAUSULA
					bPrevDouble = false;

					if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
						eClaus = MASCULINE;
				}
				else if (bIgnoreDoubles){
					//considered UNSTRESSED
					uiPenalties += 1;
					bIgnoreDoubles = false;

					if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
							eClaus = HYPERDACTYLIC;
				}
				else{
					//considered STRESSED
					bIgnoreDoubles = true;

					if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
						eClaus = MASCULINE;
				}
			}
			else{
				if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
					eClaus = MASCULINE;
			}
		}
		else if (uiInd % 3 == 2){
			//2,5,8... unstressed
			//allowed alternations : _ / [/];  _ / [/ _] / _
			if (m_wordMatrix[uiInd].second == STRESSED){
				if (m_wordSyllables[uiWord] == 1)
					uiPenalties += 1;
				else if (m_wordSyllables[uiWord] == 2 &&  m_wordMatrix[uiInd - 1].first == uiWord)
					uiPenalties += 1;
				else
					uiPenalties += 10;
			}
			else if (m_wordMatrix[uiInd].second == DOUBLE_STRESSED){
				if (m_wordSyllables[uiWord] == 2 && m_wordMatrix[uiInd - 1].first != uiWord){
					uiPenalties += 1;
					uiInd += 1;
				}
				else if (bIgnoreDoubles)
					bIgnoreDoubles = false;
				else if (bPrevDouble){
					uiPenalties += 10;
					bPrevDouble = false;
				}
				else
					bPrevDouble = true;
			}

			if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
				if (eClaus != HYPERDACTYLIC)
					eClaus = FEMININE;
		}
	}

	if (uiPenalties <= 10){
		int iFeet = m_uiNumOfSyllables / 3 + ((eClaus == MASCULINE) ? 1 : 0);
		Prediction * pPred = new Prediction(AMPHIBRACH, eClaus, iFeet, uiPenalties);
		m_Predictions.push_back(pPred);
	}
}

void Verse::CheckAnapest(){
	unsigned int uiPenalties = 0;
	bool bPrevDouble = false;
	bool bIgnoreDoubles = false;
	enumClausula eClaus = C_UNKNOWN;
	for (unsigned int uiInd = 0; uiInd < m_uiNumOfSyllables; uiInd++){
		unsigned int uiWord = m_wordMatrix[uiInd].first;
		// TODO move to general function
		if (m_wordMatrix[uiInd].second == UNKNOWN)
			return;
		if (uiInd % 3 == 0){
			//0,3,6... unstressed
			//allowed alternation: [/] _ /;  [/ _] / 
			if (m_wordMatrix[uiInd].second == STRESSED){
				if (m_wordSyllables[uiWord] == 1)
					uiPenalties += 1;
				else if (m_wordSyllables[uiWord] == 2 && ((uiInd != 0 && m_wordMatrix[uiInd - 1].first != uiWord) || uiInd == 0))
					uiPenalties += 1;
				else
					uiPenalties += 10;

			}
			else if (m_wordMatrix[uiInd].second == DOUBLE_STRESSED){
				if (m_wordSyllables[uiWord] == 2 && ((uiInd != 0 && m_wordMatrix[uiInd - 1].first != uiWord) || uiInd == 0)){
					uiPenalties += 1;
					uiInd += 1;
				}
				else if (bIgnoreDoubles)
					bIgnoreDoubles = false;
				else if (bPrevDouble){
					uiPenalties += 10;
					bPrevDouble = false;
				}
				else
					bPrevDouble = true;
			}

			if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
				if (eClaus != HYPERDACTYLIC)
					eClaus = FEMININE;
		}
		else if (uiInd % 3 == 1){
			//1,4,7... unstressed
			//allowed alternations : _ [/] /; [_ /] /
			if (m_wordMatrix[uiInd].second == STRESSED){
				if (m_wordSyllables[uiWord] == 1)
					uiPenalties += 1;
				else if (m_wordSyllables[uiWord] == 2 && m_wordMatrix[uiInd - 1].first == uiWord)
					uiPenalties += 1;
				else
					uiPenalties += 10;

			}
			else if (m_wordMatrix[uiInd].second == DOUBLE_STRESSED){
				if (bPrevDouble){
					uiPenalties += 10;
					bPrevDouble = false;
				}
				else if (bIgnoreDoubles)
					bIgnoreDoubles = false;
				else
					bPrevDouble = true;
			}

			if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
				if (eClaus != HYPERDACTYLIC)
					eClaus = DACTYLIC;
		}
		else if (uiInd % 3 == 2){
			//2,5,8... stressed
			//allowed alternations : _ _ _
			if (m_wordMatrix[uiInd].second == UNSTRESSED){
				uiPenalties += 1;

				if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
					eClaus = HYPERDACTYLIC;
			}
			else if (m_wordMatrix[uiInd].second == DOUBLE_STRESSED){
				if (bPrevDouble){
					//considered STRESSED
					bPrevDouble = false;

					if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
						eClaus = MASCULINE;
				}
				else if (bIgnoreDoubles){
					//considered UNSTRESSED
					uiPenalties += 1;
					bIgnoreDoubles = false;

					if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
						eClaus = HYPERDACTYLIC;
				}
				else{
					//considered STRESSED
					bIgnoreDoubles = true;

					if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
						eClaus = MASCULINE;
				}
			}
			else{
				if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
					eClaus = MASCULINE;
			}
		}
	}

	if (uiPenalties <= 10){
		int iFeet = m_uiNumOfSyllables / 3 - ((eClaus == HYPERDACTYLIC) ? 1 : 0);
		Prediction * pPred = new Prediction(ANAPEST, eClaus, iFeet, uiPenalties);
		m_Predictions.push_back(pPred);
	}
}

void Verse::CheckDactyl(){
	unsigned int uiPenalties = 0;
	bool bPrevDouble = false;
	bool bIgnoreDoubles = false;
	enumClausula eClaus = C_UNKNOWN;
	for (unsigned int uiInd = 0; uiInd < m_uiNumOfSyllables; uiInd++){
		unsigned int uiWord = m_wordMatrix[uiInd].first;
		if (m_wordMatrix[uiInd].second == UNKNOWN)
			return;
		if (uiInd % 3 == 0){
			//0,3,6... stressed
			//allowed alternation: _ _ _
			if (m_wordMatrix[uiInd].second == UNSTRESSED){
				uiPenalties += 1;

				if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
					eClaus = HYPERDACTYLIC;
			}
			else if (m_wordMatrix[uiInd].second == DOUBLE_STRESSED){
				if (bPrevDouble){
					//considered STRESSED
					bPrevDouble = false;

					if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
						eClaus = MASCULINE;
				}
				else if (bIgnoreDoubles){
					//considered UNSTRESSED
					uiPenalties += 1;
					bIgnoreDoubles = false;
					if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
						eClaus = HYPERDACTYLIC;
				}
				else{
					//considered STRESSED
					bIgnoreDoubles = true;

					if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
						eClaus = MASCULINE;
				}
			}
			else{
				if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
					eClaus = MASCULINE;
			}
		}
		else if (uiInd % 3 == 1){
			//1,4,7... unstressed
			//allowed alternations : / [/] _; / [/] [/]; / [/ _]
			if (m_wordMatrix[uiInd].second == STRESSED){
				if (m_wordSyllables[uiWord] == 1)
					uiPenalties += 1;
				else if (m_wordSyllables[uiWord] == 2 && m_wordMatrix[uiInd - 1].first != uiWord)
					uiPenalties += 1;
				else
					uiPenalties += 10;
			}
			else if (m_wordMatrix[uiInd].second == DOUBLE_STRESSED){
				if (m_wordSyllables[uiWord] == 2 && m_wordMatrix[uiInd - 1].first != uiWord){
					uiPenalties += 1;
					uiInd += 1;
				}
				else if (bIgnoreDoubles)
					bIgnoreDoubles = false;
				else if (bPrevDouble){
					uiPenalties += 10;
					bPrevDouble = false;
				}
				else
					bPrevDouble = true;
			}

			if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
				if (eClaus != HYPERDACTYLIC)
					eClaus = FEMININE;
		}
		else if (uiInd % 3 == 2){
			//2,5,8... unstressed
			//allowed alternations : / _ [/]; / [/] [/]; / [_ /]
			if (m_wordMatrix[uiInd].second == STRESSED){
				if (m_wordSyllables[uiWord] == 1)
					uiPenalties += 1;
				else if (m_wordSyllables[uiWord] == 2 && m_wordMatrix[uiInd - 1].first == uiWord)
					uiPenalties += 1;
				else
					uiPenalties += 10;
			}
			else if (m_wordMatrix[uiInd].second == DOUBLE_STRESSED){
				if (bPrevDouble){
					uiPenalties += 10;
					bPrevDouble = false;
				}
				else if (bIgnoreDoubles)
					bIgnoreDoubles = false;
				else
					bPrevDouble = true;
			}

			if (m_wordMatrix[uiInd].first + 1 == m_uiNumOfWords)
				if (eClaus != HYPERDACTYLIC)
					eClaus = DACTYLIC;
		}
	}

	if (uiPenalties <= 10){
		int iFeet = m_uiNumOfSyllables / 3 + ((eClaus == FEMININE || eClaus == MASCULINE) ? 1 : 0);
		Prediction * pPred = new Prediction(DACTYL, C_UNKNOWN, iFeet, uiPenalties);
		m_Predictions.push_back(pPred);
	}
}

Verse::Verse(const CEString sText, IParser * pPar) : m_sText(sText), m_pParser(pPar), m_Best(NULL) {
	m_sText.ResetSeparators();
	m_sText.SetVowels(g_szRusVowels);
	m_sText.SetPunctuation(szDefaultPunctuation_);
	m_sText.SetBreakChars(L" ");
	m_uiNumOfSyllables = m_sText.uiGetNumOfSyllables();
	m_uiNumOfWords = m_sText.uiGetNumOfFields();
	eDefineVowels();
	eMakeMatrix();
	eDefineWordEnds();
	eIdentifyMeter();
}

Verse::Verse() : m_sText(L""), m_pParser(NULL), m_uiNumOfSyllables(0), m_uiNumOfWords(0){
}

Verse::Verse(const Verse &CopiedVerse){
	m_pParser = CopiedVerse.m_pParser;
	m_sText = CopiedVerse.m_sText;
	m_Matrix = CopiedVerse.m_Matrix;
}

Verse& Verse::operator=(const Verse &CopiedVerse){
	if (CopiedVerse.m_pParser){
		if (this->m_pParser){
			delete m_pParser;
			m_pParser = CopiedVerse.m_pParser;
		}
	}
	m_sText = CopiedVerse.m_sText;
	m_Matrix = CopiedVerse.m_Matrix;
	m_uiNumOfSyllables = CopiedVerse.m_uiNumOfSyllables;
	m_uiNumOfWords = CopiedVerse.m_uiNumOfWords;
	return *this;
}

void Verse::ClearPredictions(){

	for (pred_vec::iterator itP = m_Predictions.begin(); itP != m_Predictions.end(); itP++){
		delete *itP;
	}

}
Verse::~Verse(){
	ClearPredictions();
	delete m_Best;
}

Poem::Poem(const CEString songText, IParser * pPar): m_sRawText(songText), m_pParser(pPar), m_pDict(NULL){
	m_sRawText.ResetSeparators();
	m_sRawText.SetBreakChars(L"\t\n");
	unsigned int uiNumOfTokens = m_sRawText.uiGetNumOfTokens();
	for (unsigned int i = 0; i < uiNumOfTokens; i++){
		if (m_sRawText.eGetTokenType(i) != ecTokenBreakChars){
			CEString sTemp = m_sRawText.sGetToken(i);
			sTemp.ResetSeparators();
			sTemp.SetVowels(g_szRusVowels);
			sTemp.ToLower();
			if (sTemp.uiGetNumOfSyllables()){
				Verse * pLine = new Verse(sTemp, m_pParser);
				m_Verses.push_back(pLine);
			}
		}
	}
}

Poem::Poem(CEString songText, const CEString& sPath) : m_sRawText(songText),m_pParser(NULL) ,m_pDict(NULL){
	m_sRawText.ResetSeparators();
	m_sRawText.SetBreakChars(L"\t\n");
	ET_ReturnCode rc = setParser(sPath);
	if (rc)
		m_pParser = NULL;
	unsigned int uiNumOfTokens = m_sRawText.uiGetNumOfTokens();
	for (unsigned int i = 0; i < uiNumOfTokens; i++){
		if (m_sRawText.eGetTokenType(i) != ecTokenBreakChars){
			CEString sTemp = m_sRawText.sGetToken(i);
			sTemp.ResetSeparators();
			sTemp.SetVowels(g_szRusVowels);
			sTemp.ToLower();
			if (sTemp.uiGetNumOfSyllables()){
				Verse * pLine = new Verse(sTemp, m_pParser);
				m_Verses.push_back(pLine);
			}
		}
	}
}

ET_ReturnCode Poem::setParser(const CEString& sPath){
	ET_ReturnCode rc = GetDictionary(m_pDict);
	if (rc)
		return rc;
	rc = m_pDict->eSetDbPath(sPath);
	if (rc)
		return rc;
	rc = m_pDict->eGetParser(m_pParser);
	return rc;
}

void Poem::ClearVerses(){
	for (verse_vec::iterator itVers = m_Verses.begin; itVers != m_Verses.end(); itVers++){
		delete *itVers;
	}
	m_Verses.clear();
}

Poem::~Poem(){
	ClearVerses();
	delete m_pDict;
}