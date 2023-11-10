#include "CvGameCoreDLL.h"
#include "TimeMeasure.h"
#include <time.h>

using namespace std;

#define MAX_TASKS 100

//constructor
TimeMeasure::TimeMeasure()
{
	m_iMaxTasks = MAX_TASKS;
	m_sInvalid = "invalid";
	m_Tasks = new Task[m_iMaxTasks];
	for(int iI=0;iI<m_iMaxTasks;iI++)
	{
		m_Tasks[iI].sTaskName=new TCHAR[100];
		strcpy(m_Tasks[iI].sTaskName,m_sInvalid);
	}
}

//destructor
TimeMeasure::~TimeMeasure()
{
	for(int iI=0;iI<m_iMaxTasks;iI++)
	{
		SAFE_DELETE_ARRAY(m_Tasks[iI].sTaskName);
	}
	SAFE_DELETE_ARRAY(m_Tasks);
}

int TimeMeasure::maxTasks()
{
	return m_iMaxTasks;
}

void TimeMeasure::Start(const TCHAR* TaskName)
{
	double dTimeNow=clock();

	//Do we Already have such a Task?
	for(int iI=0;iI<maxTasks();iI++)
	{
		if(0 == strcmp(m_Tasks[iI].sTaskName,TaskName))
		{
			m_Tasks[iI].dLastStartTime=dTimeNow;
			return;
		}
		else if(0 == strcmp(m_Tasks[iI].sTaskName,m_sInvalid))
		{
			for(int iJ=0;iJ<100;iJ++)
			{
				m_Tasks[iI].sTaskName[iJ]=TaskName[iJ];
			}
			m_Tasks[iI].dTimePassed=0.0;
			m_Tasks[iI].dLastStartTime=dTimeNow;
			return;
		}
	}
}

void TimeMeasure::Stop(const TCHAR* TaskName)
{
	double dTimeNow=clock();

	//Do we Already have such a Task?
	for(int iI=0;iI<maxTasks();iI++)
	{
		if(0 == strcmp(m_Tasks[iI].sTaskName,TaskName))
		{
			if(dTimeNow-m_Tasks[iI].dLastStartTime>0)
				m_Tasks[iI].dTimePassed+=dTimeNow-m_Tasks[iI].dLastStartTime;
			return;
		}
	}
}

void TimeMeasure::doLog()
{
	TCHAR szOut[1024];

	int iNumUnits=0;
	float fTime;
	float fTimePercent;
	double dMaxTime;

	for(int iI=0;iI<maxTasks();iI++)
	{
		if(0 != strcmp(m_Tasks[iI].sTaskName,m_sInvalid))
		{
			dMaxTime=std::max(m_Tasks[iI].dTimePassed/CLK_TCK,dMaxTime);
		}
	}


	for(int iI=0;iI<GC.getMAX_PLAYERS();iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iNumUnits+=GET_PLAYER((PlayerTypes)iI).getNumUnits();
		}
	}

	sprintf(szOut, "Turn %d: Units %d: Cities %d\n",GC.getGame().getElapsedGameTurns(),iNumUnits,GC.getGame().getNumCities());
	gDLL->logMsg("TimeMeasure.log",szOut, false, false);
	sprintf(szOut, "------------------------------------------\n");
	gDLL->logMsg("TimeMeasure.log",szOut, false, false);

	for(int iI=0;iI<maxTasks();iI++)
	{
		if(0 != strcmp(m_Tasks[iI].sTaskName,m_sInvalid))
		{
			fTime=(float)m_Tasks[iI].dTimePassed/CLK_TCK;
			fTimePercent=(float)(100.0f*m_Tasks[iI].dTimePassed/CLK_TCK/dMaxTime);
			sprintf(szOut, "Task %s : Time Used: %.02f (%.02f%%)\n",m_Tasks[iI].sTaskName, fTime,fTimePercent);
			gDLL->logMsg("TimeMeasure.log",szOut, false, false);
		}
	}
}