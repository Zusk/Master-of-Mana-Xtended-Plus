#include "CvGameCoreDLL.h"
#include "CySelectionGroup.h"
#include "CyPlot.h"
#include "CyArea.h"
#include "CyUnit.h"
//#include "CvStructs.h"
//# include <boost/python/manage_new_object.hpp>
//# include <boost/python/return_value_policy.hpp>

//
// published python interface for CySelectionGroup
//

void CySelectionGroupInterface2(python::class_<CySelectionGroup>& x)
{
	OutputDebugString("Python Extension Module - CySelectionGroupInterface\n");

	// set the docstring of the current module scope
	python::scope().attr("__doc__") = "Civilization IV SelectionGroup Class";
	x
/*************************************************************************************************/
/**	BETTER AI (New Functions Definition) Sephi                                 					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
/** Better Naval AI **/
        .def("setTransportUnit", &CySelectionGroup::setTransportUnit, "void (CyUnit* pTransportUnit)")
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

		;
}
