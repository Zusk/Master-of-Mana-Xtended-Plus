#include "CvGameCoreDLL.h"
#include "TimeMeasure.h"
#include <time.h>

using namespace std;

//constructor
TimeMeasure::TimeMeasure()
{
}

//destructor
TimeMeasure::~TimeMeasure()
{
}

void TimeMeasure::Start(const TCHAR* TaskName)
{
	double dTimeNow = clock();

	//Do we Already have such a Task?
	for(list<Task>::iterator it = m_Tasks.begin(); it != m_Tasks.end(); ++it) 
	{
		if(0 == strcmp(it->sTaskName, TaskName)) {
			if(!it->bRunning) {
				it->dLastStartTime = dTimeNow;
				it->bRunning = true;
			}
			return;
		}
	}

	//Create new Task
	struct Task newTask = {TaskName, 0.0, dTimeNow, true};

	m_Tasks.push_back(newTask);
}

void TimeMeasure::Stop(const TCHAR* TaskName)
{
	double dTimeNow = clock();

	for(list<Task>::iterator it = m_Tasks.begin(); it != m_Tasks.end(); ++it) 
	{
		if(0 == strcmp(it->sTaskName, TaskName)) {
			it->dTimePassed += dTimeNow - it->dLastStartTime;
			it->bRunning = false;
			return;
		}
	}
}

void TimeMeasure::doLog()
{
	TCHAR szOut[1024];

	float fTime;
	float fTimePercent;
	double dMaxTime;

	for(list<Task>::iterator it = m_Tasks.begin(); it != m_Tasks.end(); ++it) 
	{
		dMaxTime = std::max(it->dTimePassed/CLK_TCK, dMaxTime);
	}

	if(dMaxTime > 0) {
		for(list<Task>::iterator it = m_Tasks.begin(); it != m_Tasks.end(); ++it) 
		{
			fTime = (float)it->dTimePassed / CLK_TCK;
			fTimePercent = (float)(100.0f * it->dTimePassed / CLK_TCK / dMaxTime);

			sprintf(szOut, "Task %s : Time Used: %.02f (%.02f%%)\n", it->sTaskName, fTime, fTimePercent);
			gDLL->logMsg("TimeMeasure.log",szOut, false, false);
		}
	}
}
