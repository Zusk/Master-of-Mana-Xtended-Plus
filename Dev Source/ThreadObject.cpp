#include "CvGameCoreDLL.h"
#include "ThreadObject.h"
#include "ThreadOrganizer.h"

ThreadObject::ThreadObject()
	:	m_bWork(false)
{
}

ThreadObject::~ThreadObject()
{
}

//main Function of Worker Thread
void ThreadObject::main()
{
	ThreadOrganizer &kOrganizer = GC.getGame().getThreadOrganizer();
	CvGameAI* pGame = &GC.getGame();
	int iValue;

	while(true) {
		if(hasWork()) {
			for(int i = m_iFirstLoop; i < m_iLastLoop; ++i) {

				iValue = (pGame->*kOrganizer.getParallelLoopFunction())(i);

				if(iValue > m_iBestValue) {
					m_iBestValue = iValue;
					m_iBest = i;
				}
			}
		}
		setHasWork(false);
		ReleaseSemaphore(m_Semaphore, 1, NULL);
		SuspendThread(m_Handle);		
	}
}

int ThreadObject::getLoopStart() { return m_iFirstLoop; }
int ThreadObject::getLoopEnd()   { return m_iLastLoop; }

bool ThreadObject::hasWork() { return m_bWork; }
void ThreadObject::setHasWork(bool bvalue) { m_bWork = bvalue; }
void ThreadObject::setHandle(HANDLE handle) { m_Handle = handle; }

void ThreadObject::doLoop(find_best_function best_function)
{
	int iValue;

	CvGameAI* pGame = &GC.getGame();

	for(int i = m_iFirstLoop; i < m_iLastLoop; ++i) {

		iValue = (pGame->*best_function)(i);

		if(iValue > m_iBestValue) {
			m_iBestValue = iValue;
			m_iBest = i;
		}
	}
}

void ThreadObject::setParallelLoop(int iStart, int iEnd, int iBestValue, int iBest)
{
	m_iFirstLoop = iStart;
	m_iLastLoop = iEnd;
	m_iBestValue = iBestValue;
	m_iBest = iBest;
	m_bWork = true;
}

int ThreadObject::getBestValue() {	return m_iBestValue; }

int ThreadObject::getBest()	{	return m_iBest; }
