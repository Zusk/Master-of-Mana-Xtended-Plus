"""
FfHUI 05/2020 lfgr
Configurable UI functionality for Fall from Heaven.
"""

from CvPythonExtensions import *

gc = CyGlobalContext()


def cyclePlotHelpForwards( argsList = None ) :
	gc.changePlotHelpCycleIdx( 1 )

def cyclePlotHelpBackwards( argsList = None ) :
	gc.changePlotHelpCycleIdx( -1 )
