#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvGameCoreUtils.h"
#include "CvCityAI.h"
#include "CvGameAI.h"
#include "CvPlot.h"
#include "CvArea.h"
#include "CvPlayerAI.h"
#include "CvTeamAI.h"
#include "CvInfos.h"
#include "FProfiler.h"

//updates the BestBuild Information for all Plots of the City
void CvCityAI::AI_updateBestBuild()
{
	//update cache
	update_AI_valueYield();
	AI_updateWorstPlot();

	CvPlot* pLoopPlot;
	int iI;

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	CvAIGroup* pWorker = getAIGroup_Worker();

	if(pWorker == NULL) {
		FAssertMsg(pWorker != NULL, "CIty has no AI worker Group");
		return;
	}

	//decide if the current Worker Action is still valid
	if(pWorker->getMissionPlot() != NULL)
	{
		if(pWorker->getMissionPlot()->getWorkingCity() == this
			&& (!kPlayer.AI_isPlotThreatened(pWorker->getMissionPlot(), 3))
			&& pWorker->getMissionPlot()->getArea() == getArea())
		{
			AI_bestPlotBuild(pWorker->getMissionPlot());
			if(AI_getBestBuild(getCityPlotIndex(pWorker->getMissionPlot())) == NO_BUILD)
			{
				pWorker->setMissionPlot(NULL);
			}
		}
		else
		{
			pWorker->setMissionPlot(NULL);
		}
	}

	// pick a new Plot
	if(pWorker->getMissionPlot() == NULL)
	{
		for (iI = 0; iI < getNumCityPlots(); iI++)
		{
			iBestValue = 50;
			m_aiBestBuildValue[iI] = 0;
			m_aeBestBuild[iI] = NO_BUILD;

			if (iI != CITY_HOME_PLOT)
			{
				pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

				if (NULL != pLoopPlot && pLoopPlot->getWorkingCity() == this)
				{
					if(pLoopPlot->getArea() == getArea())
					{
						if (!GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(pLoopPlot, 3))
						{
							AI_bestPlotBuild(pLoopPlot);

							if(iBestValue < AI_getBestBuildValue(iI))
							{
								iBestValue = AI_getBestBuildValue(iI);
								pBestPlot = pLoopPlot;
							}
						}
					}
				}
			}
		}

		pWorker->setMissionPlot(pBestPlot);
		if(pBestPlot != NULL)
		{
			pWorker->setMissionBuild(AI_getBestBuild(getCityPlotIndex(pBestPlot)));
		}
	}
}

//returns how much the city/player values the YieldAmount. 
//Amount is 100x the yieldvalue, so 1 Food is 100
//Values cached in update_AI_valueYield
int CvCityAI::AI_valueYield(YieldTypes eYield, int iAmount)
{
	return (iAmount * getAI_valueYield(eYield)) / 100;
}

int CvCityAI::getAI_valueYield(YieldTypes eYield) const
{
	return m_aiAI_valueYield[eYield];
}

// update AI_valueYield
// currently run whenever AI decides for new Builds for a City
void CvCityAI::update_AI_valueYield()
{
	CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());
	CitySpecializationTypes eSpecialization = AI_getCitySpecialization();
	int iMod;
	for(int i = 0; i < GC.getNUM_YIELD_TYPES(); ++i) {
		switch(i)
		{
			case YIELD_FOOD:
				if(kPlayer.isIgnoreFood()) {
					m_aiAI_valueYield[i] = 0;
				}
				else {
					iMod = 100;
					if(isFoodProduction())
					{
						iMod += 20;
					}
					if(happyLevel() > unhappyLevel())
					{
						iMod += 200 * (happyLevel() - unhappyLevel()) * (happyLevel() - unhappyLevel())/std::max(1,(2 + 3 * foodDifference()));
					}
					m_aiAI_valueYield[i] = iMod;
				}
				break;
			case YIELD_PRODUCTION:
				if(eSpecialization = CITYSPECIALIZATION_MILITARY)
					m_aiAI_valueYield[i] = 250;
				else 
					m_aiAI_valueYield[i] = 150;
				break;
			case YIELD_COMMERCE:
				if(eSpecialization != CITYSPECIALIZATION_MILITARY)
					m_aiAI_valueYield[i] = 100;
				else
					m_aiAI_valueYield[i] = 50;
				break;
			case YIELD_METAL:
			case YIELD_LEATHER:
				if(eSpecialization != CITYSPECIALIZATION_GOLD)
					m_aiAI_valueYield[i] = 33;
				else
					m_aiAI_valueYield[i] = 100;
				break;
			case YIELD_LUMBER:
			case YIELD_STONE:
				if(eSpecialization != CITYSPECIALIZATION_GOLD)
					m_aiAI_valueYield[i] = 50;
				else
					m_aiAI_valueYield[i] = 100;
				break;
			case YIELD_HERB:
				//deactivate for now
				//iMod=(GET_PLAYER(getOwnerINLINE()).getFlavorValue(FLAVOR_MAGIC)==0) ? 2 : 1;
				//iValue+=(iAmount*iMod*GET_PLAYER(getOwnerINLINE()).getNumCities()*4)/std::max(1,GET_PLAYER((PlayerTypes)getOwnerINLINE()).calculateGlobalYieldIncome(eYield));
				m_aiAI_valueYield[i] = 0;
				break;
			default:
				m_aiAI_valueYield[i] = 100;
				break;
		}
	}
}

//Updates the cache, which plot of the City has the worst Yields
void CvCityAI::AI_updateWorstPlot()
{
	int iValue;
	int iBestValue = MAX_INT;
	int WorstCityIndex = -1;

	for (int iI = 0; iI < getNumCityPlots(); iI++)
	{
		CvPlot* pLoopPlot = getCityIndexPlot(iI);

		if (pLoopPlot != NULL)	{
			if(isWorkingPlot(iI) && !pLoopPlot->isCity()) {
				iValue = 0;

				for(int iJ = 0; iJ < GC.getNUM_YIELD_TYPES(); iJ++) {
					iValue += AI_valueYield((YieldTypes)iJ, pLoopPlot->calculateYield((YieldTypes)iJ));
				}

				if(iValue < iBestValue)  {
					iBestValue = iValue;
					WorstCityIndex = iI;
				}
			}
		}
	}

	setAI_WorstPlotIndex(WorstCityIndex);
}

void CvCityAI::AI_setBestPlotBuild(CvPlot* pPlot, int iBestValue, BuildTypes eBestBuild)
{
	int iI = getCityPlotIndex(pPlot);

	if(iI < 0 || iI >= getNumCityPlots()) {
		return;
	}
	
	m_aiBestBuildValue[iI] = iBestValue;
	m_aeBestBuild[iI] = eBestBuild;
}

// Updates AI information what Build is Best on gives Plot by calling AI_setBestPlotBuild
void CvCityAI::AI_bestPlotBuild(CvPlot* pPlot)
{
	BuildTypes eBestBuild;
	int iBestValue;

	AI_setBestPlotBuild(pPlot, 0, NO_BUILD);

	if (pPlot->getWorkingCity() != this)  {
		return;
	}

	//temporary -- make sure AI doesn't build improvements over other improvements
	if (pPlot->getImprovementType() != NO_IMPROVEMENT)
		return;

//added Sephi

	eBestBuild = NO_BUILD;
	iBestValue = 0;

	int iValue;
	int iI;
	for(iI = 0; iI < GC.getNumBuildInfos(); iI++) {
		iValue = 0;
		if(GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, (BuildTypes)iI, false))  {
			iValue = AI_PlotBuildValue((BuildTypes)iI, pPlot);
		}

		if(iValue > iBestValue)  {
			iBestValue = iValue;
			eBestBuild = (BuildTypes)iI;
		}
	}

/**
	GC.getGame().getThreadOrganizer().setLoopObject(pPlot);
	GC.getGame().getThreadOrganizer().setLoopTarget(this);
	GC.getGame().getThreadOrganizer().parallel_loop(&CvGameAI::testParallelFor, GC.getNumBuildInfos(), iBestValue, (int&)eBestBuild);
**/
	AI_setBestPlotBuild(pPlot, iBestValue, eBestBuild);	

	return;
}

//returns the Value of a Build on a given plot
//AI is blocked from building Improvements that have negative values
int CvCityAI::AI_PlotBuildValue(BuildTypes eBuild, CvPlot* pPlot)
{
	// TODO: What if we do not want to work the Improvement at all, only use it to improve other plots?
	BuildTypes eBestBuild = NO_BUILD;
	int iValue = 0;
	int iChange;
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	//check if we can build the eBuild with a standard worker
	if(!AI_isWorkerImprovement(eBuild))
		return 0;

	if(pPlot->getBonusType(NO_TEAM) == NO_BONUS) {
		if(kPlayer.getGold() < 2000) {
			if(kPlayer.getCultivationRate() > 66) {
				return 0;
			}
		}

		//City already has many improvements?
		CvCity* pCity = pPlot->getPlotCity();
		int iCount = 0;
		if(pCity != NULL) {
			for(int i = 0; i < pCity->getNumCityPlots(); ++i) {
				CvPlot* pLoopPlot = pCity->getCityIndexPlot(i);
				if(pLoopPlot->getImprovementType() != NO_IMPROVEMENT) {
					++iCount;
				}
			}

			if(iCount > pCity->getPopulation() + 2) {
				return 0;
			}
		}
	}

	ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
	if(eImprovement == NO_IMPROVEMENT)
		return 0;
	ImprovementTypes eOldImprovement = pPlot->getImprovementType();

	// Value Yield
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++) {
		iChange = pPlot->ActualImprovementChanges(getOwnerINLINE(), eBuild, (YieldTypes)iI, NO_COMMERCE, false);
		iValue += AI_valueYield((YieldTypes)iI, iChange);
	}

	// Value Commerce
	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++) {
		iChange = pPlot->ActualImprovementChanges(getOwnerINLINE(), eBuild, NO_YIELD, (CommerceTypes)iI, false);
		iValue += AI_valueYield(YIELD_COMMERCE, iChange);
	}

	// Value Bonuses
	// TODO: Allow AI to build on Bonuses sometimes (when it cannot see resource yet)
	if(pPlot->getBonusType(NO_TEAM) != NO_BONUS) {
//		TechTypes eTech = (TechTypes)GC.getBonusInfo(pPlot->getBonusType(getTeam())).getTechCityTrade();
//		if(eTech == NO_TECH || GET_TEAM(getTeam()).isHasTech(eTech)) {

			if(GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(pPlot->getBonusType(NO_TEAM))) {
				iValue += 2000;
			}
			else
				iValue -= 2000;
/**
			if(eOldImprovement != NO_IMPROVEMENT && (GC.getImprovementInfo(eOldImprovement).isImprovementBonusTrade(pPlot->getBonusType(getTeam()))))  {
				iValue -= 2000;
			}
**/
//		}						
	}

	//Special Value for Favored Improvements
	ImprovementTypes iFavoredImp = (ImprovementTypes)GC.getCivilizationInfo(GET_PLAYER(getOwnerINLINE()).getCivilizationType()).getImpInfrastructureHalfCost();
	if(iFavoredImp != NO_IMPROVEMENT && 
		((pPlot->getBonusType(getTeam()) == NO_BONUS) || GC.getImprovementInfo(iFavoredImp).isImprovementBonusTrade(pPlot->getBonusType(getTeam()))))
	{
		if(iFavoredImp == eImprovement) {
			if(GC.getFREE_FAVORED_IMPROVEMENTS_PER_CITY() > getNumFavoredImprovements())
				iValue += 1000;
		}
		if(pPlot->getImprovementType() == iFavoredImp)  {
			if(GC.getFREE_FAVORED_IMPROVEMENTS_PER_CITY() >= getNumFavoredImprovements()) {
				iValue-=1000;
			}
		}
	}

	//Special Value for Happiness
	//usually happiness isn't worth it ...
//	int iHappiness = GC.getImprovementInfo(eImprovement).getHappiness();
//	iValue += iHappiness;

	//Special Value for Health
	int iHealth = GC.getImprovementInfo(eImprovement).getHealth();

	if(iHealth < 0 &&
		GET_PLAYER(getOwnerINLINE()).isIgnoreUnHealthFromImprovements())
		iHealth = 0;

	if(iHealth != 0)
	{
		if(iHealth>0 && badHealth()>goodHealth())
		{
			iValue += 2*std::min(iHealth, 100*(badHealth()-goodHealth()));
		}
		else
		{
			iValue -= 2*std::max(0, -iHealth-100*(goodHealth()-badHealth()));
		}
	}

	//Special Economies
	if(kPlayer.AI_getEconomyType() == AIECONOMY_COTTAGE) {
		ImprovementTypes eCottage = (ImprovementTypes)GC.getIMPROVEMENT_COTTAGE();
		if(eImprovement == eCottage)
			iValue += 200;
		else if (eOldImprovement == eCottage)
			iValue -= 200;
	}
	else if(kPlayer.AI_getEconomyType() == AIECONOMY_TRADE) {
		ImprovementTypes eCottage = (ImprovementTypes)GC.getIMPROVEMENT_TRADE1();
		if(eImprovement == eCottage)
			iValue += 200;
		else if (eOldImprovement == eCottage)
			iValue -= 200;
	}
	else if(kPlayer.AI_getEconomyType() == AIECONOMY_FOL) {
		ImprovementTypes eCottage = (ImprovementTypes)GC.getIMPROVEMENT_CAMP();
		if(eImprovement == eCottage)
			iValue += 200;
		else if (eOldImprovement == eCottage)
			iValue -= 200;
	}

	return iValue;	
}

//How much does an Improvement increase the Yield on a Plot for a Player
int CvPlot::ActualImprovementYield(PlayerTypes ePlayer, ImprovementTypes eImprovement, YieldTypes eYield)
{
	CvImprovementInfo &kImprovement = GC.getImprovementInfo(eImprovement);
	int iYield = kImprovement.getYieldChange(eYield);

	iYield += GET_PLAYER(ePlayer).getImprovementYieldChange(eImprovement, eYield);
	iYield += GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getImprovementYieldChange(eImprovement, eYield);

	if(isIrrigated())
	{
		iYield += kImprovement.getIrrigatedYieldChange(eYield);
	}
	if(getBonusType(getTeam()) != NO_BONUS)
	{
		iYield += kImprovement.getImprovementBonusYield(getBonusType(),eYield);
	}

	return iYield;
}

//How much will the Plot give with the Improvement tied to eBuild
int CvPlot::ActualImprovementValues(PlayerTypes ePlayer, BuildTypes eBuild, YieldTypes eYield)
{
	if(eBuild == NO_BUILD)
		return 0;

	ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();

	if(GC.getImprovementInfo(eImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
		eImprovement = (ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade();
	if(GC.getImprovementInfo(eImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
		eImprovement = (ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade();

	if(eYield != NO_YIELD)
	{
		if(eImprovement == getImprovementType())
		{
			return calculateYield(eYield);
		}
		else
		{
			int iChange=ActualImprovementYield(ePlayer,eImprovement,eYield);
			iChange-=(getImprovementType()!=NO_IMPROVEMENT) ? ActualImprovementYield(ePlayer,getImprovementType(),eYield) : 0;

			if(getFeatureType()!=NO_FEATURE)
			{
				if(!GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).isMaintainFeatures(getFeatureType()))
				{
					if(GC.getBuildInfo(eBuild).isFeatureRemove(getFeatureType()))
					{
						iChange-=GC.getFeatureInfo((FeatureTypes)getFeatureType()).getYieldChange(eYield);
					}
				}
			}

			int iExtra=GET_PLAYER(ePlayer).getExtraYieldThreshold(eYield);
			if(iExtra!=0)
			{
				if(iExtra<calculateYield(eYield) && iExtra>=calculateYield(eYield)+iChange)
				{
					iChange++;
				}
			}

			return calculateYield(eYield)+iChange;
		}
	}
	return 0;
}

// returns Actual Effects that constructing Secondary Improvement has to this City by ONLY influencing neighbour tiles
int CvPlot::ActualSecondaryImprovementChanges(PlayerTypes ePlayer, ImprovementTypes eNewImprovement, YieldTypes eYield, CommerceTypes eCommerce)
{
	if(eNewImprovement == NO_IMPROVEMENT)
	{
		return 0;
	}
	int iChange=0;

	if(eYield!=NO_YIELD)
	{
		for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; iDirection++)
		{
			CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)iDirection);
			{
				if(pLoopPlot != NULL && (pLoopPlot->getImprovementType() != NO_IMPROVEMENT) 
					&& pLoopPlot->getOwnerINLINE()==getOwnerINLINE() && pLoopPlot->getWorkingCity() == getWorkingCity())
				{
					//What pLoopPlot gains if Improvement Changes on this Plot
					if(pLoopPlot->isExtraYieldfromAdjacentImprovementValid(this, eNewImprovement))
					{
						iChange += pLoopPlot->calculateExtraYieldfromAdjacentPlot(this, eYield, pLoopPlot->getImprovementType(), eNewImprovement);
					}
					//What pLoopPlot looses if Improvement Changes on this Plot
					if(pLoopPlot->isExtraYieldfromAdjacentImprovementValid(this, getImprovementType()))
					{
						iChange -= pLoopPlot->calculateExtraYieldfromAdjacentPlot(this, eYield, pLoopPlot->getImprovementType(), getImprovementType());
					}
				}
			}
		}
	}

	if(eCommerce!=NO_COMMERCE)
	{
		iChange = ActualSecondaryImprovementChanges(ePlayer, eNewImprovement, YIELD_COMMERCE, NO_COMMERCE);
	}
	return iChange;
}
int CvPlot::ActualImprovementChanges(PlayerTypes ePlayer, BuildTypes eBuild, YieldTypes eYield, CommerceTypes eCommerce, bool bSecondary, bool bCheckPlotOnly)
{
	if(getOwnerINLINE() != ePlayer)
		return 0;

	CvCity* pWorkingCity = getWorkingCity();

	if(pWorkingCity == NULL || pWorkingCity->getOwnerINLINE() != ePlayer)
		return 0;

	ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
	if(eImprovement == NO_IMPROVEMENT)
		return 0;

	if(eYield != NO_YIELD)
	{
		if(eYield == YIELD_FOOD && GET_PLAYER(ePlayer).isIgnoreFood())
			return 0;

		//Value because Improvement unlocks Globalyield Extra income (from hills, forests, etc.)
		if(eYield > YIELD_COMMERCE) {

//			GC.getTimeMeasure().Start("AI_bestPlotBuild GlobalYield Extra");
			int iOld = 0;
			bool bOld = false;
			if(getImprovementType() != NO_IMPROVEMENT && GC.getImprovementInfo(getImprovementType()).getYieldChange(eYield) > 1) {
				bOld = true;
				iOld += calculateYield(eYield);
			}
			
			int iNew = 0;
			bool bNew = false;
			if(eImprovement != NO_IMPROVEMENT && GC.getImprovementInfo(eImprovement).getYieldChange(eYield) > 1) {

				bNew = true;
				iNew += GC.getImprovementInfo(eImprovement).getYieldChange(eYield);
				if(getBonusType(getTeam()) != NO_BONUS) {
					iNew += GC.getImprovementInfo(eImprovement).getImprovementBonusYield(getBonusType(getTeam()),eYield);
				}
				if(isFreshWater())
				{
					iNew += GC.getImprovementInfo(eImprovement).getIrrigatedYieldChange(eYield);
				}
				if(isOwned())
				{
					iNew += GET_PLAYER(getOwnerINLINE()).getImprovementYieldChange(eImprovement,eYield);
					iNew += GET_TEAM(getTeam()).getImprovementYieldChange(eImprovement,eYield);
				}
			}

			int iChange = iNew - iOld;
			if(bNew && !bOld)
			{				
				iChange += pWorkingCity->getExtraYieldIncome(eYield, 1);
			}
			else if(bOld && ! bNew)
			{
				iChange += pWorkingCity->getExtraYieldIncome(eYield, -1);
			}

			iChange *= 100;
			int iModifier = pWorkingCity->getYieldRateModifier(eYield);
			iModifier += GET_PLAYER(ePlayer).getYieldRateModifier(eYield);
			iModifier += (pWorkingCity->isCapital()) ? GET_PLAYER(ePlayer).getCapitalYieldRateModifier(eYield) : 0;
			iChange *= 100 + iModifier;
			iChange /= 100;

//			GC.getTimeMeasure().Stop("AI_bestPlotBuild GlobalYield Extra");
			return iChange;
		}

//		GC.getTimeMeasure().Start("AI_bestPlotBuild Actual Yieldchange");
		int eOldValue = 0;
		int iFavoredImp = GC.getCivilizationInfo(GET_PLAYER(pWorkingCity->getOwnerINLINE()).getCivilizationType()).getImpInfrastructureHalfCost();

		//the new Improvement will replace an Improvement on a different plot because of Improvement Limits	
		if((getImprovementType() == NO_IMPROVEMENT || getImprovementType() == iFavoredImp) && pWorkingCity->calculateImprovementInfrastructureCostFreeLeft() <= 0)
		{
			CvPlot* pBestPlot = pWorkingCity->getAI_WorstPlot();

			eOldValue = calculateYield(eYield);
			ImprovementTypes eFinalImprovement = getImprovementType();
			if(eFinalImprovement != NO_IMPROVEMENT && GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
				eFinalImprovement = (ImprovementTypes)GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade();
			if(eFinalImprovement != NO_IMPROVEMENT && GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
				eFinalImprovement = (ImprovementTypes)GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade();
			if(eFinalImprovement != getImprovementType()) {
				eOldValue += GC.getImprovementInfo(eFinalImprovement).getYieldChange(eYield);
				eOldValue -= GC.getImprovementInfo(getImprovementType()).getYieldChange(eYield);
			}
			if(pBestPlot != NULL && pBestPlot->getImprovementType() != NO_IMPROVEMENT) {
				//substract Improvement that will be removed
				eOldValue += GC.getImprovementInfo(pBestPlot->getImprovementType()).getYieldChange(eYield);
			}
		}
		else if(isBeingWorked() || bCheckPlotOnly)
		{
			eOldValue = calculateYield(eYield);
			ImprovementTypes eFinalImprovement = getImprovementType();
			if(eFinalImprovement != NO_IMPROVEMENT && GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
				eFinalImprovement = (ImprovementTypes)GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade();
			if(eFinalImprovement != NO_IMPROVEMENT && GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
				eFinalImprovement = (ImprovementTypes)GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade();
			if(eFinalImprovement != getImprovementType())
			{
				eOldValue += GC.getImprovementInfo(eFinalImprovement).getYieldChange(eYield);
				eOldValue -= GC.getImprovementInfo(getImprovementType()).getYieldChange(eYield);
			}
		}
		else
		{
			CvPlot* pBestPlot = pWorkingCity->getAI_WorstPlot();

			if(pBestPlot != NULL)
			{
				eOldValue=pBestPlot->calculateYield(eYield);
				ImprovementTypes eFinalImprovement=pBestPlot->getImprovementType();
				if(eFinalImprovement!=NO_IMPROVEMENT && GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade()!=NO_IMPROVEMENT)
					eFinalImprovement=(ImprovementTypes)GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade();
				if(eFinalImprovement!=NO_IMPROVEMENT && GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade()!=NO_IMPROVEMENT)
					eFinalImprovement=(ImprovementTypes)GC.getImprovementInfo(eFinalImprovement).getImprovementUpgrade();
				if(eFinalImprovement!=pBestPlot->getImprovementType())
				{
					eOldValue+=GC.getImprovementInfo(eFinalImprovement).getYieldChange(eYield);
					eOldValue-=GC.getImprovementInfo(pBestPlot->getImprovementType()).getYieldChange(eYield);
				}
			}
		}

		int eNewValue = ActualImprovementValues(ePlayer,eBuild,eYield);

		int iChange = eNewValue - eOldValue;
//		GC.getTimeMeasure().Stop("AI_bestPlotBuild Actual Yieldchange");
//		GC.getTimeMeasure().Start("AI_bestPlotBuild Adjacent Bonuses");
		for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; iDirection++)
		{
			CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)iDirection);
			{
				if(pLoopPlot!=NULL && pLoopPlot->getOwnerINLINE()==getOwnerINLINE())
				{
					if(pLoopPlot->getWorkingCity()==getWorkingCity())
					{
						//what Plot gains from Adjacent Plots if Improvement Changes
						if(isExtraYieldfromAdjacentImprovementValid(pLoopPlot, pLoopPlot->getImprovementType()))
						{
							iChange += calculateExtraYieldfromAdjacentPlot(pLoopPlot, eYield, eImprovement, pLoopPlot->getImprovementType());
						}
						if(isExtraYieldfromAdjacentImprovementValid(pLoopPlot, pLoopPlot->getImprovementType()))
						{
							iChange -= calculateExtraYieldfromAdjacentPlot(pLoopPlot, eYield, getImprovementType(), pLoopPlot->getImprovementType());
						}

						//What pLoopPlot gains if Improvement Changes on this Plot
						if(pLoopPlot->isExtraYieldfromAdjacentImprovementValid(this, eImprovement))
						{
							iChange += pLoopPlot->calculateExtraYieldfromAdjacentPlot(this, eYield, pLoopPlot->getImprovementType(), eImprovement);
						}
						//What pLoopPlot looses if Improvement Changes on this Plot
						if(pLoopPlot->isExtraYieldfromAdjacentImprovementValid(this, getImprovementType()))
						{
							iChange -= pLoopPlot->calculateExtraYieldfromAdjacentPlot(this, eYield, pLoopPlot->getImprovementType(), getImprovementType());
						}
					}
				}
			}
		}
//		GC.getTimeMeasure().Stop("AI_bestPlotBuild Adjacent Bonuses");

		iChange *= 100;
		int iModifier = pWorkingCity->getYieldRateModifier(eYield);
		iModifier += GET_PLAYER(ePlayer).getYieldRateModifier(eYield);
		iModifier += (pWorkingCity->isCapital()) ? GET_PLAYER(ePlayer).getCapitalYieldRateModifier(eYield) : 0;
		iChange *= 100 + iModifier;
		iChange /= 100;

		return iChange;
	}

	if(eCommerce != NO_COMMERCE)
	{
		int iChange = ActualImprovementChanges(ePlayer,eBuild,YIELD_COMMERCE,NO_COMMERCE, bSecondary, bCheckPlotOnly);
		if(pWorkingCity->getCommercePercentTypeForced() != NO_COMMERCE)
		{
			if(eCommerce != pWorkingCity->getCommercePercentTypeForced())
			{
				iChange = 0;
			}
		}
		else
		{			
			iChange = (iChange*GET_PLAYER(ePlayer).getCommercePercent(eCommerce)) / 100;
		}

		/**
		if(eCommerce==COMMERCE_GOLD)
		{
			if(getImprovementType()==NO_IMPROVEMENT && eImprovement!=NO_IMPROVEMENT)
			{
				iChange-=pWorkingCity->calculateImprovementInfrastructureCost((int)eImprovement);
			}
		}
		**/
		int iModifier=pWorkingCity->getCommerceRateModifier(eCommerce);
		iModifier+=GET_PLAYER(ePlayer).getCommerceRateModifier(eCommerce);
		iModifier+=(pWorkingCity->isCapital()) ? GET_PLAYER(ePlayer).getCapitalCommerceRateModifier(eCommerce) : 0;

		iChange*=(100+iModifier);
		iChange/=100;
		return iChange;
	}
	return 0;
}

// function returns Yieldincrease on this plot caused by Improvement on pPlot
int CvPlot::calculateExtraYieldfromAdjacentPlot(CvPlot* pPlot, YieldTypes eYield, ImprovementTypes eImprovement, ImprovementTypes eImprovementAdjacent) const
{
	if(pPlot == NULL)
		return 0;
	
	if(eImprovementAdjacent == NO_IMPROVEMENT)
		return 0;

	int iValue = 0;

	bool bHasCottage = false;
	if(eImprovement != NO_IMPROVEMENT) {
		ImprovementTypes eUpgrade = (ImprovementTypes)GC.getIMPROVEMENT_COTTAGE();
		while(eUpgrade != NO_IMPROVEMENT) {

			if(eImprovement == eUpgrade)
				bHasCottage = true;

			eUpgrade = (ImprovementTypes)GC.getImprovementInfo(eUpgrade).getImprovementUpgrade();
		}
	}

	switch(eYield)
	{
		case YIELD_FOOD:
			if(eImprovementAdjacent == GC.getIMPROVEMENT_GROUNDWATER_WELL()) {
				iValue += (eImprovement == GC.getIMPROVEMENT_FARM() || eImprovement == GC.getIMPROVEMENT_PASTURE()) ? 1 : 0;
			}
			break;
		case YIELD_PRODUCTION:
			if(bHasCottage) {
				if(eImprovementAdjacent == GC.getIMPROVEMENT_WORKSHOP()) {
					iValue += 1;
				}
			}
			break;
		default:
			break;
	}

	//Count Improvement Only Once
	return iValue;
}


//return true if Plot gets its extra Yield of eImprovement from pPlot
bool CvPlot::isExtraYieldfromAdjacentImprovementValid(const CvPlot* pPlot,ImprovementTypes eImprovement) const
{
	for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; iDirection++)
	{
		CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)iDirection);
		if(pLoopPlot != NULL && pLoopPlot->getImprovementType() == eImprovement
//			&& pLoopPlot->getWorkingCity() != NULL && pLoopPlot->getWorkingCity() == getWorkingCity()
			)
		{
			return (pPlot == pLoopPlot);
		}
	}

	return true;
}