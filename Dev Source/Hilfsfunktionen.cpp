#include "CvGameCoreDLL.h"
#include "Hilfsfunktionen.h"

// List to Sort will store the sorted Values(from highest to lowest) and IDs will store the Information how to retrieve them
void sortList(int* ListToSort, int* IDs, int iIndex)
{
	int iBestValue, iBestElement;

	int* SortedList=new int[iIndex];
	int* SortedIDs=new int[iIndex];

	for (int iI = 0; iI < iIndex; iI++)
	{
		iBestValue=-MAX_INT;
		SortedList[iI]=-MAX_INT;
		iBestElement=-MAX_INT;
		//find best Value 
		for (int iJ = 0; iJ < iIndex; iJ++)
		{
			if(ListToSort[iJ]>iBestValue)
			{
				iBestValue=ListToSort[iJ];
				iBestElement=iJ;
			}
		}
		//and put it in iPillageSorted
		if(iBestElement!=-MAX_INT)
		{
			SortedList[iI]=ListToSort[iBestElement];
			SortedIDs[iI]=IDs[iBestElement];
			ListToSort[iBestElement]=-MAX_INT;
		}
	}

	for(int iI=0;iI<iIndex;iI++)
	{
		ListToSort[iI]=SortedList[iI];
		IDs[iI]=SortedIDs[iI];
	}

	delete(SortedList);
	delete(SortedIDs);
}


//Write AI Information into log file

void doAI_EconomyLog()
{

	for(int i = 0; i < MAX_CIV_PLAYERS; ++i) {
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)i);

		int TotalFood = 0;
		int TotalProduction = 0;
		int TotalScience = 0;
		int TotalFaith = 0;
		int TotalCulture = 0;
		int TotalMana = 0;
		int TotalGold = 0;
		int TotalSpellResearch = 0;

		//int TotalPlayers = 0;
		int iLoop;
		TCHAR szOut[1024];

		if(kPlayer.isAlive()) {
			//++TotalPlayers;

			for (CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
			{
				TotalFood += pLoopCity->getYieldRate(YIELD_FOOD);
				TotalProduction += pLoopCity->getYieldRate(YIELD_PRODUCTION);
				TotalScience += pLoopCity->getCommerceRate(COMMERCE_RESEARCH);
				TotalCulture += pLoopCity->getCommerceRate(COMMERCE_CULTURE);
				TotalGold += pLoopCity->getCommerceRate(COMMERCE_GOLD);
			}

			TotalFaith = kPlayer.getFaithIncome();
			TotalMana = kPlayer.getManaIncome();
			TotalSpellResearch = kPlayer.getArcaneIncome();

			sprintf(szOut, "Turn: %d,PlayerID: %d,Player: %S,Food: %d,Production: %d,Science: %d,Faith: %d,Culture: %d,Mana: %d,Gold: %d,SpellResearch: %d"
				,GC.getGame().getElapsedGameTurns()
				,kPlayer.getID()
				,kPlayer.getName()
				,TotalFood
				,TotalProduction
				,TotalScience
				,TotalFaith
				,TotalCulture
				,TotalMana
				,TotalGold
				,TotalSpellResearch
			);
			gDLL->logMsg("AI_economy.log",szOut, false, false);
		}
	}
	/*
	if(TotalPlayers == 0)
		return;
	
	sprintf(szOut, "------------------------------------------\n");
	gDLL->logMsg("AI_economy.log",szOut, false, false);
	sprintf(szOut, "------------------------------------------\n");
	gDLL->logMsg("AI_economy.log",szOut, false, false);
	sprintf(szOut, "Turn %d: \n",GC.getGame().getElapsedGameTurns());
	gDLL->logMsg("AI_economy.log",szOut, false, false);

	sprintf(szOut, "Food: %d", TotalFood / TotalPlayers);
	gDLL->logMsg("AI_economy.log",szOut, false, false);
	sprintf(szOut, "Production: %d", TotalProduction / TotalPlayers);
	gDLL->logMsg("AI_economy.log",szOut, false, false);
	sprintf(szOut, "Science: %d", TotalScience / TotalPlayers);
	gDLL->logMsg("AI_economy.log",szOut, false, false);
	sprintf(szOut, "Faith: %d", TotalFaith / TotalPlayers);
	gDLL->logMsg("AI_economy.log",szOut, false, false);
	sprintf(szOut, "Culture: %d", TotalCulture / TotalPlayers);
	gDLL->logMsg("AI_economy.log",szOut, false, false);
	sprintf(szOut, "Mana: %d", TotalMana / TotalPlayers);
	gDLL->logMsg("AI_economy.log",szOut, false, false);
	sprintf(szOut, "Gold: %d", TotalGold / TotalPlayers);
	gDLL->logMsg("AI_economy.log",szOut, false, false);
	sprintf(szOut, "SpellResearch: %d", TotalSpellResearch / TotalPlayers);
	gDLL->logMsg("AI_economy.log",szOut, false, false);
	*/
}

//New Class for Better Display - Sephi
StringList::StringList() :
m_bFirst(true),
m_bEmpty(true)
{
	m_szText.clear();
}

StringList::~StringList()
{
}

void StringList::newEntry(CvWString newEntry)
{
	m_bFirst = true;

	if(!m_bEmpty)
		m_szText.append(NEWLINE);

	m_szText.append(newEntry);
	m_bEmpty = false;
}

void StringList::append(CvWString newEntry)
{
	m_bEmpty = false;
	if(m_bFirst) {
		m_szText.append(newEntry);
		m_bFirst = false;
	}
	else {
		m_szText.append(L", ");
		m_szText.append(newEntry);
	}
}

CvWString StringList::get() const
{
	return m_szText;
}

bool StringList::isEmpty() const
{
	return m_bEmpty;
}
