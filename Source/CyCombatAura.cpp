// CombatAura.cpp

#include "CvGameCoreDLL.h"
#include "CyCombatAura.h"
#include "CvCombatAura.h"
#include "CyGlobalContext.h"

CyCombatAura::CyCombatAura() : m_pCombatAura(NULL)
{
//	m_pWinamp = &GC.getCombatAura();
}

CyCombatAura::CyCombatAura(CvCombatAura* pCombatAura) : m_pCombatAura(pCombatAura)
{
}
