#include "CvGameCoreDLL.h"
#include <process.h>
#include "ThreadObject.h"

ThreadOrganizer::ThreadOrganizer()
	:	m_bInit(false)
{
	m_iNumWorkerThreads = 101;
	m_aHandle = new HANDLE[m_iNumWorkerThreads];
	m_paThreads = new ThreadObject[m_iNumWorkerThreads];

	//m_iNumWorkerThreads = findOptimalNumWorkerThreads();
	m_iNumWorkerThreads = 2;
}

ThreadOrganizer::~ThreadOrganizer()
{
	if(m_bInit)
		for(int i = 1; i < m_iNumWorkerThreads; ++i) {
			CloseHandle(m_aHandle[i]);
		}

	SAFE_DELETE_ARRAY(m_aHandle);
	SAFE_DELETE_ARRAY(m_paThreads);
}


//do a test_run with X workerthreads to find optimal amount of workerthreads
int ThreadOrganizer::findOptimalNumWorkerThreads()
{
	int iNumWorkerThreads = 1;

	double fBestTime = test_run(m_iNumWorkerThreads);
	double fLoopTime;

	while(m_iNumWorkerThreads < 100) {
		fLoopTime = test_run(m_iNumWorkerThreads + 1);
		if(fLoopTime < fBestTime) {
			fBestTime = fLoopTime;
			++iNumWorkerThreads;
		}
		else
			break;
	}

	return iNumWorkerThreads;
}

//this method requires the ThreadOrganizer Object in GC.getGame(), so it cannot be included in the constructor
void ThreadOrganizer::CreateThreads()
{
	//create Threads
    SECURITY_ATTRIBUTES  sa = {
           sizeof(SECURITY_ATTRIBUTES), 
           0,     
           TRUE    
    };
	unsigned int iThreadId;  // not stored at the moment

	for(int i = 1; i < m_iNumWorkerThreads; ++i) {
		m_aHandle[i] = (HANDLE)_beginthreadex(&sa, 4096, &start_thread, (void *)i, 
												NULL, &iThreadId);
		m_paThreads[i].setHandle(m_aHandle[i]);
	}

	m_bInit = true; 
}


double ThreadOrganizer::test_run(int iThreads)
{
	double StartTime, EndTime;
    SECURITY_ATTRIBUTES  sa = {
           sizeof(SECURITY_ATTRIBUTES), 
           0,     
           TRUE    
    };

	unsigned int m_iThreadId;

	//find a good value for iLoop
	int iLoop = 1000 / std::max(1, iThreads);

	double Result = 0;
	while(Result < 100 && iLoop > 0) {
		iLoop = iLoop * 10;
		StartTime = clock();
		start_test_performance((void*)iLoop);
		EndTime = clock();
		Result = EndTime - StartTime;
	}	

	StartTime = clock();

	for(int i = 1; i < m_iNumWorkerThreads; ++i) {
		m_aHandle[i] = (HANDLE)_beginthreadex(&sa, 4096, &start_test_performance, (void *)iLoop, 
												NULL, &m_iThreadId);
	}

	start_test_performance((void*)iLoop);

	for(int i = 1; i < m_iNumWorkerThreads; ++i) {
		WaitForSingleObject(m_aHandle[i], INFINITE );
		CloseHandle(m_aHandle[i]);
	}

	EndTime = clock();

	Result = EndTime - StartTime;

	TCHAR szOut[1024];
	sprintf(szOut, "Thread Number: %d   --- Time used: %f\n", iThreads, Result);

	gDLL->logMsg("MP.log",szOut, false, false);

	return Result;
}

unsigned __stdcall start_test_performance(void *arg)
{
	int iNum = (int)arg;

	int l = 0;
	for(int i = 0; i < iNum; ++i) {
		for(int k = 0; k < 10; k++) {
			for(int j = 0; j < 1000; j++) {
				l += (i *j % 3) + k;
			}
		}
	}

	//use this so Compiler doesn't remove our pointless for loop 
	TCHAR szOut[1024];
	sprintf(szOut, "Thread Number: %d", l);			

	return l;
}

ThreadObject& ThreadOrganizer::getThread(int iThread)
{
	return m_paThreads[iThread];
}

find_best_function ThreadOrganizer::getParallelLoopFunction()
{
	return m_find_best_function;
}

void ThreadOrganizer::setParallelLoopFunction(find_best_function bestfunction)
{
	m_find_best_function = bestfunction;
}

void ThreadOrganizer::setLoopObject(void* pObject) { m_pLoopObject = pObject; }
void* ThreadOrganizer::getLoopObject() { return m_pLoopObject; }
void ThreadOrganizer::setLoopTarget(void* pObject) { m_pLoopTarget = pObject; }
void* ThreadOrganizer::getLoopTarget() { return m_pLoopTarget; }

unsigned __stdcall start_parallel_loop(void *arg)
{
	int iThread = (int)arg;
	ThreadOrganizer &kOrganizer = GC.getGame().getThreadOrganizer();
	ThreadObject &kThread = kOrganizer.getThread(iThread);

	kThread.doLoop(kOrganizer.getParallelLoopFunction());
	return 0;
}

unsigned __stdcall start_thread(void *arg)
{
	int iThread = (int)arg;

	ThreadOrganizer &kOrganizer = GC.getGame().getThreadOrganizer();
	ThreadObject &kThread = kOrganizer.getThread(iThread);

	kThread.main();
	return 0;
}

void ThreadOrganizer::parallel_loop(find_best_function best_function, int iLoop, int &iBestValue, int &iBest)
{
	//this has to run outside constructor, but before worker threads are used
	if(!m_bInit)  {
		CreateThreads();		
	}

	setParallelLoopFunction(best_function);

	int iEnd = iLoop / m_iNumWorkerThreads;
	int iLoopStart = iEnd;
	int iLoopEnd;

	for(int i = 1; i < m_iNumWorkerThreads; ++i) {
		iLoopEnd = ((i + 1) * iLoop) / m_iNumWorkerThreads;
		m_paThreads[i].setParallelLoop(iLoopStart, iLoopEnd, iBestValue, iBest);
		iLoopStart = iLoopEnd;
	}

	for(int i = 1; i < m_iNumWorkerThreads; ++i) {
		m_paThreads[i].m_Semaphore = CreateSemaphore(NULL, 10, 10, NULL);
		ResumeThread(m_aHandle[i]);
	}

	m_paThreads[0].setParallelLoop(0, iEnd, iBestValue, iBest);
	m_paThreads[0].doLoop(best_function);

	for(int i = 1; i < m_iNumWorkerThreads; ++i) {
		WaitForSingleObject(m_paThreads[i].m_Semaphore, INFINITE);
	}

	for(int i = 0; i < m_iNumWorkerThreads; ++i) {
		if(m_paThreads[i].getBestValue() > iBestValue) {
			iBestValue = m_paThreads[i].getBestValue();
			iBest = m_paThreads[i].getBest();
		}
	}
}
