#include "CvGameCoreDLL.h"
/**
#include "CvMap.h"
using namespace std;

#define SECTOR_SIZE (10)

struct PathPlot
{
	int iX;
	int iY;
};

class Sibling
{
	private:

}

class PathTree
{
	public:
//	bool isPlotinPath(CvPlot* pLoopPlot);
	void setStart(CvPlot* pLoopPlot);
	private:
	PathPlot HeadElement;
	list<Sibling> Siblings;
};


void PathTree::setStart(CvPlot* pLoopPlot)
{
	HeadElement.iX=pLoopPlot->getX_INLINE();
	HeadElement.iY=pLoopPlot->getY_INLINE();
}


bool PathTree::isPlotinPath(CvPlot* pLoopPlot)
{
	tree<PathPlot>::iterator i;

	for(i = Tree.begin(); i!=Tree.end();i++)
	{
		if(pLoopPlot->getX_INLINE()==i->iX && pLoopPlot->getY_INLINE()==i->iY)
		{
			return true;
		}
	}
	return false;
}


void CvMap::calculateRegions()
{
	int iRegionCounter=0;
	//sectors first

	//sectors use the following enum
	// 0 | 1 | 2 | 3
	// 4 | 5 | 6 | 7
	// 5 | 6 | 7 | 8
	int iMaxSectorX=(getGridWidth()+SECTOR_SIZE-1)/SECTOR_SIZE;
	int iNumSectors=iMaxSectorX*((getGridHeight()+SECTOR_SIZE-1)/SECTOR_SIZE);
	for(int iI=0;iI<numPlots();iI++)
	{
		CvPlot* pLoopPlot=plotByIndex(iI);
		pLoopPlot->setSector(pLoopPlot->getX()/SECTOR_SIZE+(pLoopPlot->getY()/SECTOR_SIZE)*iMaxSectorX);
	}

	//now Regions
	for(int iJ=0;iJ<iNumSectors;iJ++)
	{
		for(int iI=0;iI<numPlots();iI++)
		{		
			CvPlot* pLoopPlot=plotByIndex(iI);
			//plot is in our LoopSector?
			if(pLoopPlot->getSector()==iJ && pLoopPlot->getRegion()==-1)
			{
				//check if there is a path to another plot that is in the sector
				int iNewRegion=-1;
				for(int iK=0;iK<numPlots();iK++)
				{		
					CvPlot* pDestPlot=plotByIndex(iK);
					//is Destination Valid?
					if(pDestPlot!=NULL && pDestPlot->getSector()==iJ && pDestPlot->getArea()==pLoopPlot->getArea() && pDestPlot->getRegion()!=-1)
					{
						//is There a Path to the Destination, that stays in the sector?
						//create a PathTree
						PathTree Tree;
						
					}
				}

				if(iNewRegion!=-1)
				{
					pLoopPlot->setRegion(iNewRegion);
				}
				else
				{
					pLoopPlot->setRegion(iRegionCounter);
					iRegionCounter++;
				}
			}
		}
	}
}

int CvMap::CheckforPathinRegion(CvPlot* pLoopPlot,CvPlot* pTargetPlot)
{
	if(stepDistance(pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE(),pTargetPlot->getX_INLINE(),pTargetPlot->getY_INLINE())==1)
	{
		return pTargetPlot->getRegion();
	}

/**
	int iBestValue=-1;
	if(iRecurseDepth>SECTOR_SIZE)
	{
		return -1;
	}

	for(int iI=0;iI<NUM_DIRECTION_TYPES;iI++)
	{
		CvPlot* pAdjacentPlot=plotDirection(pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE(),(DirectionTypes)iI);
		if(pAdjacentPlot!=NULL)
		{
			if(pAdjacentPlot->getSector()==pLoopPlot->getSector())
			{
				if(pAdjacentPlot->getArea()==pLoopPlot->getArea() && !pAdjacentPlot->isPeak())
				{
					if(pAdjacentPlot->getRegion()!=-1)
					{
						iBestValue=pAdjacentPlot->getRegion();
						break;
					}
				}
			}
		}
	}

	if(iBestValue==-1)
	{
		for(int iI=0;iI<NUM_DIRECTION_TYPES;iI++)
		{
			CvPlot* pAdjacentPlot=plotDirection(pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE(),(DirectionTypes)iI);
			if(pAdjacentPlot!=NULL)
			{
				if(pAdjacentPlot->getSector()==pLoopPlot->getSector())
				{
					if(pAdjacentPlot->getArea()==pLoopPlot->getArea() && !pAdjacentPlot->isPeak())
					{
						iBestValue =std::max(iBestValue,CheckAdjacentPlots(pAdjacentPlot,iRecurseDepth+1));
						if(iBestValue!=-1)
							break;
					}
				}
			}
		}
	}

	return iBestValue;

}
**/