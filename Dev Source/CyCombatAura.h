#ifndef CYWINAMP_H
#define CYWINAMP_H

class CvCombatAura;

class CyCombatAura
{
public:
	CyCombatAura();
	CyCombatAura(CvCombatAura* pCombatAura);
	CvCombatAura* getCombatAura() { return m_pCombatAura;	}
	bool isNone() { return (m_pCombatAura==NULL); }

protected:
	CvCombatAura* m_pCombatAura;
};

#endif
