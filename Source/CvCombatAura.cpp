// CombatAura.cpp

#include "CvGameCoreDLL.h"
#include "CvCombatAura.h"

// Public Functions...


CvCombatAura::CvCombatAura()
{
	m_paiDamageTypeResist = NULL;

	reset(0,NO_PLAYER,NO_UNIT,NO_COMBATAURA,true);
}


CvCombatAura::~CvCombatAura()
{
	uninit();
}


void CvCombatAura::init(int iID, int iOwner, int iUnit, int iCombatAuraType)
{
/**
	FAssert(NO_COMBATAURA != iCombatAuraType);

	//--------------------------------
	// Init saved data
**/
	reset(iID,iOwner, iUnit, iCombatAuraType,false,true);
}

void CvCombatAura::uninit()
{
    SAFE_DELETE_ARRAY(m_paiDamageTypeResist);
}



// FUNCTION: reset()
// Initializes data members that are serialized.
void CvCombatAura::reset(int iID, int iOwner, int iUnit, int iCombatAuraType, bool bConstructorCall, bool bCheckExistingPromotions)
{
	//--------------------------------
	// Uninit class
	uninit();

	m_iID = iID;
	m_eCombatAuraType = (CombatAuraTypes)iCombatAuraType;
	m_iUnit = iUnit;
	m_eOwner = (PlayerTypes)iOwner;

	m_iPower = 0;
	m_iRange = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getRange() : 0;
	m_iTargets = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getTargets() : 0;
	m_iTargetsApplied = 0;
	m_iTier = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getTier() : 0;

	m_iFear = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getFear() : 0;
	m_iStrength = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getStrength() : 0;

	m_bLoyalty = false;
	m_bImmuneToFear = false;
	m_bWaterwalking = false;
	m_bEnchantedWeapons = false;
	m_iEnchantedWeaponBonus = 0;
	m_bImmunityRangedStrike = false;

	m_iAlignmentAttackBonus = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getAttackAlignmentBonus() : 0;
	m_iAttackPromotionBonus = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getAttackPromotionBonus() : 0;
	m_iCityAttack = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getCityAttack() : 0;
	m_iDamage = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getDamage() : 0;
	m_iExperience = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getExperience() : 0;
	m_iFirststrikes = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getFirststrikes() : 0;
	m_iGuardianAngel = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getGuardianAngel() : 0;
	m_iHealing = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getHealing() : 0;
	m_iHealthRegeneration = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getHealthRegeneration() : 0;
	m_iIgnoreFirststrikes = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getIgnoreFirststrikes() : 0;
	m_iMagicResistance = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getMagicResistance() : 0;
	m_iSpellPower = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getSpellPower() : 0;
	m_iRust = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getRust() : 0;
	m_iSpellcasterXP = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getSpellcasterXP() : 0;
	m_iUpgradePercent = (NO_COMBATAURA != m_eCombatAuraType) ? GC.getCombatAuraInfo(m_eCombatAuraType).getUpgradePercent() : 0;

	if (!bConstructorCall)
	{
        m_paiDamageTypeResist = new int[GC.getNumDamageTypeInfos()];
        for (int iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
        {
            m_paiDamageTypeResist[iI] = 0;
        }
	}

	//Modify Value by existing Promotions
	if(bCheckExistingPromotions)
	{
		CvUnit* pOwner=getUnit();

		changePower(4 * (pOwner->getLevel()-1));

		for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
		{
			if(pOwner->isHasPromotion((PromotionTypes)iI))
			{
				CvPromotionInfo& kPromotion=GC.getPromotionInfo((PromotionTypes)iI);
				changeRange(kPromotion.getCombatAuraRange());
				changePower(kPromotion.getCombatAuraEffect());
				changeExperience(kPromotion.getCombatAuraExperienceChange());
				changeFear(kPromotion.getCombatAuraFearEffectChange());
				changeIgnoreFirststrikes(kPromotion.getCombatAuraIgnoreFirstStrikes());
				changeFirststrikes(kPromotion.getCombatAuraIncreaseFirstStrikes());
				changeCityAttack(kPromotion.getCombatAuraCityAttackChange());
				changeHealthRegeneration(kPromotion.getCombatAuraHealthRegenerationChange());
				changeStrength(kPromotion.getCombatAuraStrengthChange());
				(kPromotion.isCombatAuraImmunityRangedStrike()) ? setImmunityRangedStrike(true) : 0;
				(kPromotion.isCombatAuraWaterwalking()) ? setWaterwalking(true) : 0;
				(kPromotion.isCombatAuraEnchantedWeapons()) ?	setEnchantedWeapons(true) : 0;
				(kPromotion.isCombatAuraLoyaltyChange()) ? setLoyalty(true) : 0;
				(kPromotion.isCombatAuraImmuneToFearChange()) ? setImmuneToFear(true) : 0;

				if(getTargets()>=0)
				{
					changeTargets(GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraTargets());				
				}
				for(int iI=0;iI<GC.getNumDamageTypeInfos();iI++)
				{
					changeDamageTypeResist((DamageTypes)iI,kPromotion.getCombatAuraDamageTypeResist(iI));
				}
			}
		}
	}
}

void CvCombatAura::kill()
{
	if(getTargetsApplied()>0)
	{
		int iLoop;

		for(int iI=0;iI<GC.getMAX_PLAYERS();iI++)
		{
			if(getTargetsApplied()==0)
			{
				break;
			}

			if(GET_PLAYER((PlayerTypes)iI).isAlive())
			{				
				for (CvUnit* pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
				{
					if(getTargetsApplied()==0)
					{
						break;
					}

					if (pLoopUnit->getBlessing1()==this)
					{
						Remove(pLoopUnit);
						pLoopUnit->setBlessing1(NULL);
					}

					if (pLoopUnit->getBlessing2()==this)
					{
						Remove(pLoopUnit);
						pLoopUnit->setBlessing2(NULL);
					}

					if (pLoopUnit->getCurse1()==this)
					{
						Remove(pLoopUnit);
						pLoopUnit->setCurse1(NULL);
					}

					if (pLoopUnit->getCurse2()==this)
					{
						Remove(pLoopUnit);
						pLoopUnit->setCurse2(NULL);
					}
				}
			}
		}
	}

	FAssertMsg(getTargetsApplied() ==0, "TargetsApplied is not assigned a valid value");

	GET_PLAYER(getOwnerINLINE()).deleteCombatAura(getID());
}

int CvCombatAura::getID() const
{
	return m_iID;
}


int CvCombatAura::getIndex() const
{
	return (getID() & FLTA_INDEX_MASK);
}

/**
IDInfo CvCombatAura::getIDInfo() const
{
	IDInfo CvCombatAura(getOwnerINLINE(), getID());
	return CvCombatAura;
}
**/

void CvCombatAura::setID(int iID)
{
	m_iID = iID;
}

const CombatAuraTypes CvCombatAura::getCombatAuraType() const
{
	return m_eCombatAuraType;
}

void CvCombatAura::read(FDataStreamBase* pStream)
{
	// Init data before load
	reset();

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iID);
	pStream->Read((int*)&m_eCombatAuraType);
	pStream->Read((int*)&m_eOwner);	
	pStream->Read(&m_iUnit);

	pStream->Read(&m_iPower);
	pStream->Read(&m_iRange);
	pStream->Read(&m_iTargets);
	pStream->Read(&m_iTargetsApplied);
	pStream->Read(&m_iTier);

	pStream->Read(&m_iStrength);	
	pStream->Read(&m_iFear);

	pStream->Read(&m_bLoyalty);
	pStream->Read(&m_bImmuneToFear);
	pStream->Read(&m_bWaterwalking);
	pStream->Read(&m_bEnchantedWeapons);
	pStream->Read(&m_iEnchantedWeaponBonus);
	pStream->Read(&m_bImmunityRangedStrike);
	pStream->Read(&m_iAlignmentAttackBonus);
	pStream->Read(&m_iAttackPromotionBonus);
	pStream->Read(&m_iCityAttack);
	pStream->Read(&m_iDamage);
	pStream->Read(&m_iExperience);
	pStream->Read(&m_iFirststrikes);
	pStream->Read(&m_iGuardianAngel);
	pStream->Read(&m_iHealing);
	pStream->Read(&m_iHealthRegeneration);
	pStream->Read(&m_iIgnoreFirststrikes);
	pStream->Read(&m_iMagicResistance);
	pStream->Read(&m_iSpellPower);
	pStream->Read(&m_iRust);
	pStream->Read(&m_iSpellcasterXP);
	pStream->Read(&m_iUpgradePercent);
	pStream->Read(GC.getNumDamageTypeInfos(), m_paiDamageTypeResist);

}


void CvCombatAura::write(FDataStreamBase* pStream)
{
	uint uiFlag=2;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iID);
	pStream->Write(m_eCombatAuraType);
	pStream->Write(m_eOwner);
	pStream->Write(m_iUnit);

	pStream->Write(m_iPower);
	pStream->Write(m_iRange);
	pStream->Write(m_iTargets);
	pStream->Write(m_iTargetsApplied);
	pStream->Write(m_iTier);

	pStream->Write(m_iStrength);	
	pStream->Write(m_iFear);

	pStream->Write(m_bLoyalty);
	pStream->Write(m_bImmuneToFear);
	pStream->Write(m_bWaterwalking);
	pStream->Write(m_bEnchantedWeapons);
	pStream->Write(m_iEnchantedWeaponBonus);
	pStream->Write(m_bImmunityRangedStrike);
	pStream->Write(m_iAlignmentAttackBonus);
	pStream->Write(m_iAttackPromotionBonus);
	pStream->Write(m_iCityAttack);
	pStream->Write(m_iDamage);
	pStream->Write(m_iExperience);
	pStream->Write(m_iFirststrikes);
	pStream->Write(m_iGuardianAngel);
	pStream->Write(m_iHealing);
	pStream->Write(m_iHealthRegeneration);
	pStream->Write(m_iIgnoreFirststrikes);
	pStream->Write(m_iMagicResistance);
	pStream->Write(m_iSpellPower);
	pStream->Write(m_iRust);
	pStream->Write(m_iSpellcasterXP);
	pStream->Write(m_iUpgradePercent);
	pStream->Write(GC.getNumDamageTypeInfos(), m_paiDamageTypeResist);
}

//
//
//
//
//

CvUnit* CvCombatAura::getUnit() const
{
	return GET_PLAYER(getOwnerINLINE()).getUnit(m_iUnit);
}

void CvCombatAura::ApplyEffects(CvUnit* pUnit)
{
	//if pUnit==NULL, apply Effects to all unit
	int iLoop;
	for(int iI=0;iI<GC.getMAX_PLAYERS();iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isAlive())
		{				
			for (CvUnit* pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
			{
				if(pUnit!=NULL)
					pLoopUnit=pUnit;

				if (pLoopUnit->getBlessing1()==this || pLoopUnit->getBlessing2()==this ||
					pLoopUnit->getCurse1()==this || pLoopUnit->getCurse2()==this)
				{
					ApplyEffectsToUnit(pLoopUnit);
				}

				if(pUnit!=NULL)
					return;
			}
		}
	}
}

void CvCombatAura::RemoveEffects(CvUnit* pUnit)
{
	//if pUnit==NULL, remove Effects from all units
	int iLoop;
	for(int iI=0;iI<GC.getMAX_PLAYERS();iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isAlive())
		{				
			for (CvUnit* pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
			{
				if(pUnit!=NULL)
					pLoopUnit=pUnit;

				if (pLoopUnit->getBlessing1()==this || pLoopUnit->getBlessing2()==this ||
					pLoopUnit->getCurse1()==this || pLoopUnit->getCurse2()==this)
				{
					RemoveEffectsFromUnit(pLoopUnit);
				}

				if(pUnit!=NULL)
					return;
			}
		}
	}
}

void CvCombatAura::ApplyEffectsToUnit(CvUnit* pUnit, int iChange)
{
	int iTotalChange=iChange* (isBlessing() ? 1 : -1);

	pUnit->changeImmuneToCapture(isLoyalty() ? iChange : 0);
	pUnit->changeImmuneToCapture(isImmuneToFear() ? iChange : 0);
	pUnit->changeWaterWalking(isWaterwalking() ? iChange : 0);
	pUnit->changeImmuneToRangeStrike(isImmunityRangedStrike() ? iChange : 0);

	pUnit->changeExtraCombatPercent(iTotalChange * getStrength());
	pUnit->changeCombatPercentDefense(iTotalChange * getStrength());
	
	pUnit->changeFearEffect(iTotalChange * getFear());

	pUnit->changeExtraCityAttackPercent(iTotalChange * getCityAttack());
	pUnit->changeExperiencePercent(iTotalChange * getExperience());
	pUnit->changeExtraFirstStrikes(iTotalChange * getFirststrikes()/100);
	pUnit->changeExtraEnemyHeal(iTotalChange * getHealthRegeneration());
	pUnit->changeExtraFriendlyHeal(iTotalChange * getHealthRegeneration());
	pUnit->changeExtraNeutralHeal(iTotalChange * getHealthRegeneration());
	pUnit->changeIgnoreFirstStrikes(iTotalChange * getIgnoreFirststrikes()/100);
	pUnit->changeResist(iTotalChange * getMagicResistance());
	pUnit->changeSpellDamageModify(iTotalChange * getSpellPower());
	pUnit->changeSpellCasterXP(iTotalChange * getSpellcasterXP()/100);
	pUnit->changeUpgradeDiscount(iTotalChange * getUpgradePercent());

	for(int iI=0;iI<GC.getNumDamageTypeInfos();iI++)
	{
		pUnit->changeDamageTypeResist((DamageTypes)iI,getDamageTypeResist((DamageTypes)iI) * iTotalChange);
	}
}

void CvCombatAura::RemoveEffectsFromUnit(CvUnit* pUnit)
{
	ApplyEffectsToUnit(pUnit,-1);
}

void CvCombatAura::Apply(CvUnit* pUnit)
{
	ApplyEffectsToUnit(pUnit);
	changeTargetsApplied(1);
}

void CvCombatAura::Remove(CvUnit* pUnit)
{
	RemoveEffects(pUnit);
	changeTargetsApplied(-1);
}

bool CvCombatAura::isValid(CvUnit* pUnit)
{
	FAssert(pUnit!=NULL);

	int difX=pUnit->getX_INLINE()-getUnit()->getX_INLINE();

	if(difX*difX>getRange()*getRange())
	{
		return false;
	}

	int difY=pUnit->getY_INLINE()-getUnit()->getY_INLINE();

	if(difY*difY>getRange()*getRange())
	{
		return false;
	}

	if(getTargets()!=-1)
	{
		if(getTargets()<getTargetsApplied())
		{
			return false;
		}
	}

	if(!isBlessing())
	{
		if(!pUnit->isEnemy(GET_PLAYER(getOwnerINLINE()).getTeam()))
		{
			return false;
		}
	}

	return true;
}

void CvCombatAura::updatePromotionAdded(PromotionTypes ePromotion, int iChange)
{
	RemoveEffects();
	ProcessPowerEffect(-1);

	CvPromotionInfo& kPromotion=GC.getPromotionInfo(ePromotion);

	changeRange(kPromotion.getCombatAuraRange() * iChange);
	changeTargets(kPromotion.getCombatAuraTargets() * iChange);

	changeExperience(kPromotion.getCombatAuraExperienceChange() * iChange);
	changeFear(kPromotion.getCombatAuraFearEffectChange() * iChange);
	changeIgnoreFirststrikes(kPromotion.getCombatAuraIgnoreFirstStrikes() * iChange);
	changeFirststrikes(kPromotion.getCombatAuraIncreaseFirstStrikes() * iChange);
	changeCityAttack(kPromotion.getCombatAuraCityAttackChange() * iChange);
	changeHealthRegeneration(kPromotion.getCombatAuraHealthRegenerationChange() * iChange);
	changeStrength(kPromotion.getCombatAuraStrengthChange() * iChange);
	(kPromotion.isCombatAuraImmunityRangedStrike()) ? setImmunityRangedStrike(iChange>0) : 0;
	(kPromotion.isCombatAuraLoyaltyChange()) ? setLoyalty(iChange>0) : 0;
	(kPromotion.isCombatAuraImmuneToFearChange()) ? setImmuneToFear(iChange>0) : 0;
	(kPromotion.isCombatAuraWaterwalking()) ? setWaterwalking(iChange>0) : 0;
	(kPromotion.isCombatAuraEnchantedWeapons()) ? setEnchantedWeapons(iChange>0) : 0;

	for(int iI=0;iI<GC.getNumDamageTypeInfos();iI++)
	{
		changeDamageTypeResist((DamageTypes)iI,kPromotion.getCombatAuraDamageTypeResist(iI) * iChange);
	}

	ProcessPowerEffect(1);
	ApplyEffects();

	//changePower already calls Processpower, ApplyEffects
	changePower(kPromotion.getCombatAuraEffect() * iChange);
}

void CvCombatAura::ProcessPowerEffect(int iChange)
{
	CvCombatAuraInfo &kAura=GC.getCombatAuraInfo(getCombatAuraType());

	if(getAlignmentAttackBonus()!=0)
	{
		changeAlignmentAttackBonus(iChange * getPower()/4);
	}

	if(getStrength()!=0)
	{
		changeStrength(iChange * getPower()/4);
	}

	if(getDamage()!=0)
	{
		changeDamage(iChange * getPower()/12);
	}

	if(getFear()!=0)
	{
		changeFear(iChange * getPower()/4);
	}

	if(getCityAttack()!=0)
	{
		changeCityAttack(iChange * getPower()/2);
	}

	if(getFirststrikes()!=0)
	{
		changeFirststrikes(iChange * getPower());
	}

	if(getExperience()!=0)
	{
		changeExperience(iChange * getPower()/2);
	}

	if(getHealing()!=0)
	{
		changeHealing(iChange * getPower()/6);
	}

	if(getHealthRegeneration()!=0)
	{
		changeHealthRegeneration(iChange * getPower()/6);
	}

	if(getIgnoreFirststrikes()!=0)
	{
		changeIgnoreFirststrikes(iChange * getPower());
	}

	if(getMagicResistance()!=0)
	{
		changeMagicResistance(iChange * getPower()/4);
	}

	if(getSpellPower()!=0)
	{
		changeSpellPower(iChange * getPower()/4);
	}

	if(getRust()!=0)
	{
		changeRust(iChange * getPower()/4);
	}

	if(getSpellcasterXP()!=0)
	{
		changeSpellcasterXP(iChange * getPower()/10);
	}

	if(getUpgradePercent()!=0)
	{
		changeUpgradePercent(iChange * getPower());
	}

	for(int iI=0;iI<GC.getNumDamageTypeInfos();iI++)
	{
		if(getDamageTypeResist((DamageTypes)iI)!=0)
		{
			changeDamageTypeResist((DamageTypes)iI,iChange * getPower()/4);
		}
	}
}

void CvCombatAura::ApplyPromotionsToUnit(CvUnit* pUnit) const
{
	if(getUnit() == NULL)
	{
		return;
	}

	for(int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if(GC.getCombatAuraInfo(getCombatAuraType()).isPromotionApply(iI))
			pUnit->setHasPromotion((PromotionTypes)iI, true);
	}

	for(int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if(getUnit()->isHasPromotion((PromotionTypes)iI))
		{
			for(int iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
			{
				if(GC.getPromotionInfo((PromotionTypes)iI).isCombatAuraApplyPromotion(iJ))
				{
					pUnit->setHasPromotion((PromotionTypes)iJ, true);
				}
			}
		}
	}
}


bool CvCombatAura::isPromotionApply(PromotionTypes iProm) const
{
	if(GC.getCombatAuraInfo(getCombatAuraType()).isPromotionApply(iProm))
	{
		return true;
	}

	if(getUnit() == NULL)
	{
		return false;
	}

	for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
	{
		if(getUnit()->isHasPromotion((PromotionTypes)iI))
		{
			if(GC.getPromotionInfo((PromotionTypes)iI).isCombatAuraApplyPromotion(iProm))
			{
				return true;
			}
		}
	}

	return false;
}

bool CvCombatAura::isPromotionRemove(PromotionTypes iProm) const
{
	if(GC.getCombatAuraInfo(getCombatAuraType()).isPromotionRemove(iProm))
	{
		return true;
	}

	if(getUnit()==NULL)
	{
		return false;
	}

	for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
	{
		if(getUnit()->isHasPromotion((PromotionTypes)iI))
		{
			if(GC.getPromotionInfo((PromotionTypes)iI).isCombatAuraRemovePromotion(iProm))
			{
				return true;
			}
		}
	}

	return false;
}

bool CvCombatAura::isBlessing() const
{
	return GC.getCombatAuraInfo(getCombatAuraType()).isBlessing();
}


int CvCombatAura::getRange() const
{
	return m_iRange;
}

void CvCombatAura::changeRange(int iChange)
{
	m_iRange+=iChange;
}

int CvCombatAura::getTargets() const
{
	return m_iTargets;
}

void CvCombatAura::changeTargets(int iChange)
{
	m_iTargets+=iChange;
}

int CvCombatAura::getTargetsApplied() const
{
	return m_iTargetsApplied;
}

void CvCombatAura::changeTargetsApplied(int iChange)
{
	m_iTargetsApplied+=iChange;
}

int CvCombatAura::getTier() const
{
	return m_iTier;
}

void CvCombatAura::changeTier(int iChange)
{
	m_iTier+=iChange;
}

void CvCombatAura::changePower(int iChange) 
{
	if(iChange!=0)
	{
		RemoveEffects();
		ProcessPowerEffect(-1);
		m_iPower+=iChange;		
		ProcessPowerEffect(1);
		ApplyEffects();
	}
}

void CvCombatAura::updateEnchantedWeapons(CvUnit* pUnit)
{
	pUnit->changeExtraCombatPercent(-getEnchantedWeaponBonus());
	pUnit->changeCombatPercentDefense(-getEnchantedWeaponBonus());
	int iNewValue=0;

	for(int iI=0;iI<GC.getNumBonusInfos();iI++)
	{
		iNewValue+=GC.getBonusInfo((BonusTypes)iI).getEnchantedWeaponBonus();
	}
	setEnchantedWeaponBonus(iNewValue);
	pUnit->changeExtraCombatPercent(getEnchantedWeaponBonus());
	pUnit->changeCombatPercentDefense(getEnchantedWeaponBonus());
}

int CvCombatAura::getPower()const
{
	return m_iPower;
}

int CvCombatAura::getStrength() const
{
	return m_iStrength;
}

void CvCombatAura::changeStrength(int iChange)
{
	m_iStrength+=iChange;
}

int CvCombatAura::getFear() const
{
	return m_iFear;
}

void CvCombatAura::changeFear(int iChange)
{
	m_iFear+=iChange;
}

bool CvCombatAura::isImmunityRangedStrike() const {	return m_bImmunityRangedStrike;}
void CvCombatAura::setImmunityRangedStrike(bool bNewValue) { m_bImmunityRangedStrike=bNewValue;	}
bool CvCombatAura::isLoyalty() const {	return m_bLoyalty;}
void CvCombatAura::setLoyalty(bool bNewValue) { m_bLoyalty=bNewValue;	}
bool CvCombatAura::isImmuneToFear() const {	return m_bImmuneToFear;}
void CvCombatAura::setImmuneToFear(bool bNewValue) { m_bImmuneToFear=bNewValue;	}
bool CvCombatAura::isWaterwalking() const {	return m_bWaterwalking;}
void CvCombatAura::setWaterwalking(bool bNewValue) { m_bWaterwalking=bNewValue;	}
bool CvCombatAura::isEnchantedWeapons() const {	return m_bEnchantedWeapons;}
void CvCombatAura::setEnchantedWeapons(bool bNewValue) { m_bEnchantedWeapons=bNewValue;	}
int CvCombatAura::getEnchantedWeaponBonus() const {	return m_iEnchantedWeaponBonus;}
void CvCombatAura::setEnchantedWeaponBonus(int iNewValue) { m_iEnchantedWeaponBonus=iNewValue;	}
int CvCombatAura::getAlignmentAttackBonus() const {	return m_iAlignmentAttackBonus;}
void CvCombatAura::changeAlignmentAttackBonus(int iChange){	m_iAlignmentAttackBonus+=iChange;}
int CvCombatAura::getAttackPromotionBonus() const {	return m_iAttackPromotionBonus;};
void CvCombatAura::changeAttackPromotionBonus(int iChange){	m_iAttackPromotionBonus+=iChange;}
int CvCombatAura::getCityAttack() const {	return m_iCityAttack;}
void CvCombatAura::changeCityAttack(int iChange){	m_iCityAttack+=iChange;}
int CvCombatAura::getDamage() const {	return m_iDamage;}
void CvCombatAura::changeDamage(int iChange){	m_iDamage+=iChange;}
int CvCombatAura::getExperience() const {	return m_iExperience;}
void CvCombatAura::changeExperience(int iChange){	m_iExperience+=iChange;}
int CvCombatAura::getFirststrikes() const {	return m_iFirststrikes;}
void CvCombatAura::changeFirststrikes(int iChange){	m_iFirststrikes+=iChange;}
int CvCombatAura::getGuardianAngel() const { return m_iGuardianAngel;}
void CvCombatAura::changeGuardianAngel(int iChange){	m_iGuardianAngel+=iChange;}
int CvCombatAura::getHealing() const {	return m_iHealing;}
void CvCombatAura::changeHealing(int iChange){	m_iHealing+=iChange;}
int CvCombatAura::getHealthRegeneration() const {	return m_iHealthRegeneration;}
void CvCombatAura::changeHealthRegeneration(int iChange){	m_iHealthRegeneration+=iChange;}
int CvCombatAura::getIgnoreFirststrikes() const {	return m_iIgnoreFirststrikes;}
void CvCombatAura::changeIgnoreFirststrikes(int iChange){	m_iIgnoreFirststrikes+=iChange;}
int CvCombatAura::getMagicResistance() const {	return m_iMagicResistance;}
void CvCombatAura::changeMagicResistance(int iChange){	m_iMagicResistance+=iChange;}
int CvCombatAura::getSpellPower() const {	return m_iSpellPower;}
void CvCombatAura::changeSpellPower(int iChange){	m_iSpellPower+=iChange;}
int CvCombatAura::getRust() const {	return m_iRust;}
void CvCombatAura::changeRust(int iChange){	m_iRust+=iChange;}
int CvCombatAura::getSpellcasterXP() const {	return m_iSpellcasterXP;}
void CvCombatAura::changeSpellcasterXP(int iChange){	m_iSpellcasterXP+=iChange;}
int CvCombatAura::getUpgradePercent() const {	return m_iUpgradePercent;}
void CvCombatAura::changeUpgradePercent(int iChange){	m_iUpgradePercent+=iChange;}

int CvCombatAura::getDamageTypeResist(DamageTypes eIndex) const
{
    return m_paiDamageTypeResist[eIndex];
}

void CvCombatAura::changeDamageTypeResist(DamageTypes eIndex, int iChange)
{
	m_paiDamageTypeResist[eIndex] += iChange;
}
