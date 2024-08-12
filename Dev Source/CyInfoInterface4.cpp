#include "CvGameCoreDLL.h"
#include "CvInfos.h"

//
// Python interface for info classes (formerly structs)
// These are simple enough to be exposed directly - no wrappers
//

void CyInfoPythonInterface4()
{
	OutputDebugString("Python Extension Module - CyInfoPythonInterface4\n");

//FfH Spell System: Added by Kael 07/23/2007
    python::class_<CvSpellInfo, python::bases<CvInfoBase> >("CvSpellInfo")
		.def("getPromotionPrereq1", &CvSpellInfo::getPromotionPrereq1, "string ()")
		.def("getPromotionPrereq2", &CvSpellInfo::getPromotionPrereq2, "string ()")
		.def("getPromotionPrereq3", &CvSpellInfo::getPromotionPrereq3, "string ()")
		.def("getPromotionPrereq4", &CvSpellInfo::getPromotionPrereq4, "string ()")
		.def("getPyResult", &CvSpellInfo::getPyResult, "string ()")
		.def("getPyRequirement", &CvSpellInfo::getPyRequirement, "string ()")
		.def("isGlobal", &CvSpellInfo::isGlobal, "bool ()")
		.def("getCivilizationPrereq", &CvSpellInfo::getCivilizationPrereq, "int ()")

//>>>>BUGFfH: Added by Denev 2009/10/03
		.def("getDelay", &CvSpellInfo::getDelay, "int ()")
		.def("isAbility", &CvSpellInfo::isAbility, "bool ()")
		.def("getUnitCombatPrereq", &CvSpellInfo::getUnitCombatPrereq, "int ()")
		.def("getUnitClassPrereq", &CvSpellInfo::getUnitClassPrereq, "int ()")
		.def("getUnitPrereq", &CvSpellInfo::getUnitPrereq, "int ()")
		.def("getBuildingPrereq", &CvSpellInfo::getBuildingPrereq, "int ()")
		.def("getBuildingClassOwnedPrereq", &CvSpellInfo::getBuildingClassOwnedPrereq, "int ()")
		.def("getTerrainOrPrereq1", &CvSpellInfo::getTerrainOrPrereq1, "int ()")
		.def("getTerrainOrPrereq2", &CvSpellInfo::getTerrainOrPrereq2, "int ()")
		.def("getFeatureOrPrereq1", &CvSpellInfo::getFeatureOrPrereq1, "int ()")
		.def("getFeatureOrPrereq2", &CvSpellInfo::getFeatureOrPrereq2, "int ()")
		.def("getImprovementPrereq", &CvSpellInfo::getImprovementPrereq, "int ()")
		.def("getReligionPrereq", &CvSpellInfo::getReligionPrereq, "int ()")
		.def("getStateReligionPrereq", &CvSpellInfo::getStateReligionPrereq, "int ()")
		.def("getTechPrereq", &CvSpellInfo::getTechPrereq, "int ()")
		.def("getCost", &CvSpellInfo::getCost, "int ()")
		.def("getCreateBuildingType", &CvSpellInfo::getCreateBuildingType, "int ()")
		.def("getCreateBuildingType1", &CvSpellInfo::getCreateBuildingType1, "int ()")
		.def("getCreateBuildingType2", &CvSpellInfo::getCreateBuildingType2, "int ()")
		.def("getRemoveBuildingType", &CvSpellInfo::getRemoveBuildingType, "int ()")
		.def("getCreateFeatureType", &CvSpellInfo::getCreateFeatureType, "int ()")
		.def("getCreateImprovementType", &CvSpellInfo::getCreateImprovementType, "int ()")
		.def("getCreateUnitType", &CvSpellInfo::getCreateUnitType, "int ()")
		.def("getConvertUnitType", &CvSpellInfo::getConvertUnitType, "int ()")
		.def("getPyResultSecondary", &CvSpellInfo::getPyResultSecondary, "string ()")
//<<<<BUGFfH: End Add
		.def("isObsoletedBySpell", &CvSpellInfo::isObsoletedBySpell, "bool (int)")
/*************************************************************************************************/
/**	ADDON (Houses of Erebus) Sephi			                                 					**/
/*************************************************************************************************/
        .def("getCorporationSupport", &CvSpellInfo::getCorporationSupport, "int (int i)")
        .def("getCorporationSupportMultiplier", &CvSpellInfo::getCorporationSupportMultiplier, "int (int i)")
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
		;
//FfH: End Add

/*************************************************************************************************/
/**	ADDON (Adventure) Sephi                                                      				**/
/*************************************************************************************************/

    python::class_<CvAdventureInfo, python::bases<CvInfoBase> >("CvAdventureInfo")
		.def("isVictory", &CvAdventureInfo::isVictory, "bool ()")
		.def("isVictoryFailed", &CvAdventureInfo::isVictoryFailed, "bool ()")
		.def("getAdventureCounter", &CvAdventureInfo::getAdventureCounter, "int (int)")
		.def("getAdventureStep", &CvAdventureInfo::getAdventureStep, "int (int)")
		.def("isPrereqCiv", &CvAdventureInfo::isPrereqCiv, "bool (int)")
		;

    python::class_<CvAdventureStepInfo, python::bases<CvInfoBase> >("CvAdventureStepInfo")
		.def("isCivsDestroyed", &CvAdventureStepInfo::isCivsDestroyed, "bool ()")
		.def("isTroophyFromAnimal", &CvAdventureStepInfo::isTroophyFromAnimal, "bool ()")
		.def("isGovannonsLegacy", &CvAdventureStepInfo::isGovannonsLegacy, "bool ()")
		.def("isSheaimSummonDemon", &CvAdventureStepInfo::isSheaimSummonDemon, "bool ()")
		.def("getPurityCounter", &CvAdventureStepInfo::getPurityCounter, "int ()")
		.def("isMimicPromotion", &CvAdventureStepInfo::isMimicPromotion, "bool ()")
		;
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	ADDON (New Mana) Sephi                                                      				**/
/*************************************************************************************************/

    python::class_<CvManaschoolInfo, python::bases<CvInfoBase> >("CvManaschoolInfo")
		.def("getButton", &CvManaschoolInfo::getButton, "string ()")
		.def("isBonusLinked", &CvManaschoolInfo::isBonusLinked, "bool (int)")
		.def("getTechLevel", &CvManaschoolInfo::getTechLevel, "int (int)")

		.def("getBonusSummonStrength", &CvManaschoolInfo::getBonusSummonStrength, "int (int)")
		.def("getBonusReducedTerraformCost", &CvManaschoolInfo::getBonusReducedTerraformCost, "int (int)")
		.def("getBonusIncreasedSpellResearch", &CvManaschoolInfo::getBonusIncreasedSpellResearch, "int (int)")
		.def("getBonusReducedGlobalEnchantment", &CvManaschoolInfo::getBonusReducedGlobalEnchantment, "int (int)")
		.def("getBonusIncreasedTerraformEffect", &CvManaschoolInfo::getBonusIncreasedTerraformEffect, "int (int)")
		.def("getBonusIncreasedMana", &CvManaschoolInfo::getBonusIncreasedMana, "int (int)")

		;

    python::class_<CvLairResultInfo, python::bases<CvInfoBase> >("CvLairResultInfo")
		.def("getPyResult", &CvLairResultInfo::getPyResult, "string ()")
		.def("getPyRequirement", &CvLairResultInfo::getPyRequirement, "string ()")
		;

    python::class_<CvEquipmentCategoryInfo, python::bases<CvInfoBase> >("CvEquipmentCategoryInfo")
		;

    python::class_<CvCityStateInfo, python::bases<CvInfoBase> >("CvCityStateInfo")
		;
    python::class_<CvPlaneInfo, python::bases<CvInfoBase> >("CvPlaneInfo")
		.def("isBonusSpawn", &CvPlaneInfo::isBonusSpawn, "bool (int)")
		;
    python::class_<CvTerrainClassInfo, python::bases<CvInfoBase> >("CvTerrainClassInfo")
		;
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	ADDON (Combatauras) Sephi																	**/
/*************************************************************************************************/

    python::class_<CvCombatAuraInfo, python::bases<CvInfoBase> >("CvCombatAuraInfo")
		.def("getButton", &CvCombatAuraInfo::getButton, "string ()")
		;

    python::class_<CvCombatAuraClassInfo, python::bases<CvInfoBase> >("CvCombatAuraClassInfo")
		.def("getButton", &CvCombatAuraClassInfo::getButton, "string ()")
		;

/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/

}
