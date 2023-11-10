#include "CvGameCoreDLL.h"
#include "CyCombatAura.h"

void CyCombatAuraPythonInterface()
{
	OutputDebugString("Python Extension Module - CyCombatAuraPythonInterface\n");

	python::class_<CyCombatAura>("CyCombatAura")
		.def("isNone", &CyCombatAura::isNone, "CyCombatAura* () - is the instance valid?")
		;
}
