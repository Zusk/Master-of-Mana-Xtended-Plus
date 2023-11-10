#include "CvGameCoreDLL.h"
#include "CyWinamp.h"

void CyWinampPythonInterface()
{
	OutputDebugString("Python Extension Module - CyWinampPythonInterface\n");

	python::class_<CyWinamp>("CyWinamp")
		.def("isNone", &CyWinamp::isNone, "CyWinamp* () - is the instance valid?")
		
		.def("Play", &CyWinamp::Play, "void ()")
		.def("Stop", &CyWinamp::Stop, "void ()")
		.def("Prev", &CyWinamp::Prev, "void ()")
		.def("Next", &CyWinamp::Next, "void ()")
		.def("Pause", &CyWinamp::Pause, "void ()")

		.def("SetVolume", &CyWinamp::SetVolume, "void (int vol)")
		.def("GetVolume", &CyWinamp::GetVolume, "int ()")
		.def("GetTitleProgress", &CyWinamp::GetTitleProgress, "int ()")
		.def("GetTitleLength", &CyWinamp::GetTitleLength, "int ()")
		.def("GetTitleName", &CyWinamp::GetTitleName, "CvString ()")
		;
}