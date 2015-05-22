#include <vector>
#include<iterator>

#include "Enums.h"
#include "EString.h"

using namespace std;
using namespace Hlib;



class IPoem{
public:
	virtual void WriteMeterToTxt() = 0;
	virtual ~Poem() = 0;
	virtual ET_ReturnCode getVerse(int index, Verse *&) = 0;//returns null-pointer if error or index exceeded
	virtual ET_ReturnCode getStanza(int index, Stanza *&) = 0;//returns null-pointer if error or index exceeded
	virtual rhythm getRhythm() = 0; //in case whole poem is written in one consistent rhythm, if not, returns UNKNOWN 0
	virtual void reducePredictions() = 0;
	virtual rhythm_vec getConsistentStructure() = 0;
	virtual int getNumOfVerses() = 0;
	virtual int getNumOfStances() = 0;
	virtual CESTring getRawText() = 0;
};

class Verse{
	virtual ~Verse() = 0;
	virtual rhythm_vec getRhythm() = 0;
	virtual stress_vec getMatrix() = 0;
	virtual CEString getRawText() = 0;
};

class Stanza{
	virtual ~Stanza() = 0;
	virtual rhythm_vec getStructure() = 0;
	virtual int getNumOfVerses() = 0;
	virtual ET_ReturnCode getVerse(int indexm, Verse *&) = 0;//returns null-pointer if error or index exceeded
};
