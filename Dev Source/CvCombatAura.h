#pragma once

#ifndef CIV4_COMBATAURA_H
#define CIV4_COMBATAURA_H

#include "CvDLLEntity.h"
#include "LinkedList.h"

class CvCombatAura
{
public:
	// stuff necessary to communicate with rest of DLL
	CvCombatAura();
	virtual ~CvCombatAura();

	void init(int iID, int iOwner, int iUnit, int iCombatAuraType);
	void uninit();
	void reset(int iID = 0, int iOwner = NO_PLAYER, int iUnit = NO_UNIT, int iCombatAuraType = NO_COMBATAURA, bool bConstructorCall = false, bool bCheckExistingPromotions = false);
	void kill();

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

	int getID() const;																			// Exposed to Python
	int getIndex() const;
//	IDInfo getIDInfo() const;
	void setID(int iID);

	DllExport PlayerTypes getOwner() const;	
#ifdef _USRDLL
	inline PlayerTypes getOwnerINLINE() const
	{
		return m_eOwner;
	}

#endif

	const CombatAuraTypes getCombatAuraType() const;

	// end

	CvUnit* getUnit() const;	

	void Apply(CvUnit* pUnit);
	void ApplyEffects(CvUnit* pUnit = NULL);
	void Remove(CvUnit* pUnit);
	void RemoveEffects(CvUnit* pUnit = NULL);
	void ApplyEffectsToUnit(CvUnit* pUnit, int iChange = 1);
	void RemoveEffectsFromUnit(CvUnit* pUnit);
	void ProcessPowerEffect(int iChange);

	bool isValid(CvUnit* pUnit);
	void updateEnchantedWeapons(CvUnit* pUnit);
	void updatePromotionAdded(PromotionTypes ePromotion, int iChange);
	int getRange() const;
	void changeRange(int iRange);
	int getTargets() const;
	void changeTargets(int iChange);
	int getTargetsApplied() const;
	void changeTargetsApplied(int iChange);

	int getTier() const;
	void changeTier(int iTier);
	int getPower() const;
	void changePower(int iChange);
	bool isBlessing() const;
	void ApplyPromotionsToUnit(CvUnit* pUnit) const;
	bool isPromotionApply(PromotionTypes iProm) const;
	bool isPromotionRemove(PromotionTypes iProm) const;

	int getStrength() const;
	void changeStrength(int iChange);
	int getFear() const;
	void changeFear(int iChange);

	bool isLoyalty() const;
	void setLoyalty(bool bNewValue);
	bool isImmuneToFear() const;
	void setImmuneToFear(bool bNewValue);
	bool isWaterwalking() const;
	void setWaterwalking(bool bNewValue);
	bool isEnchantedWeapons() const;
	void setEnchantedWeapons(bool bNewValue);
	int getEnchantedWeaponBonus() const;
	void setEnchantedWeaponBonus(int iNewValue);
	bool isImmunityRangedStrike() const;
	void setImmunityRangedStrike(bool bNewValue);	
	int getAlignmentAttackBonus() const;
	void changeAlignmentAttackBonus(int iChange);
	int getAttackPromotionBonus() const;
	void changeAttackPromotionBonus(int iChange);
	int getCityAttack() const;
	void changeCityAttack(int iChange);
	int getDamage() const;
	void changeDamage(int iChange);
	int getExperience() const;
	void changeExperience(int iChange);
	int getFirststrikes() const;
	void changeFirststrikes(int iChange);
	int getGuardianAngel() const;
	void changeGuardianAngel(int iChange);
	int getHealing() const;
	void changeHealing(int iChange);
	int getHealthRegeneration() const;
	void changeHealthRegeneration(int iChange);
	int getIgnoreFirststrikes() const;
	void changeIgnoreFirststrikes(int iChange);
	int getMagicResistance() const;
	void changeMagicResistance(int iChange);
	int getSpellPower() const;
	void changeSpellPower(int iChange);
	int getRust() const;
	void changeRust(int iChange);
	int getSpellcasterXP() const;
	void changeSpellcasterXP(int iChange);
	int getUpgradePercent() const;
	void changeUpgradePercent(int iChange);

	int getDamageTypeResist(DamageTypes eIndex) const;
	void changeDamageTypeResist(DamageTypes eIndex, int iChange);

protected:

	// stuff necessary to communicate with rest of DLL
	int m_iID;
	CombatAuraTypes m_eCombatAuraType;
	PlayerTypes m_eOwner;
	// end

	int m_iUnit;

	int m_iRange;
	int m_iTargets;
	int m_iTargetsApplied;
	int m_iTier;
	int m_iPower;

	bool m_bBlessing;
	bool m_bPrereqAlive;
	//save everything as 100x 
	int m_iFear;
	int m_iStrength;

	bool m_bLoyalty;
	bool m_bImmuneToFear;
	bool m_bWaterwalking;
	bool m_bEnchantedWeapons;
	bool m_iEnchantedWeaponBonus;
	bool m_bImmunityRangedStrike;
	int m_iAlignmentAttackBonus;
	int m_iAttackPromotionBonus;
	int m_iCityAttack;
	int m_iDamage;
	int m_iExperience;
	int m_iFirststrikes;
	int m_iGuardianAngel;
	int m_iHealing;
	int m_iHealthRegeneration;
	int m_iIgnoreFirststrikes;
	int m_iMagicResistance;
	int m_iSpellPower;
	int m_iRust;
	int m_iSpellcasterXP;
	int m_iUpgradePercent;

	int* m_paiDamageTypeResist;
};

#endif
