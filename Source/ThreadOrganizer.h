#pragma once

// ThreadOrganizer.h

#include "ThreadObject.h"

#ifndef CIV4_THREADORGANIZER_H
#define CIV4_THREADORGANIZER_H

unsigned __stdcall start_thread(void *arg);
unsigned __stdcall start_parallel_loop(void *arg);
unsigned __stdcall start_test_performance(void *arg);

class ThreadOrganizer
{
public:
	
	ThreadOrganizer();
	virtual ~ThreadOrganizer();

	void CreateThreads();
	ThreadObject& getThread(int iThread);

	double test_run(int iThreads);
	find_best_function getParallelLoopFunction();
	void setLoopObject(void* pObject);
	void* getLoopObject();
	void setLoopTarget(void* pObject);
	void* getLoopTarget();
	void setParallelLoopFunction(find_best_function);
	void parallel_loop(find_best_function, int iLoop, int &iBestValue, int &iBest);

private:
	find_best_function m_find_best_function;
	void* m_pLoopObject;
	void* m_pLoopTarget;
	ThreadObject* m_paThreads;
	HANDLE* m_aHandle;
	int m_iNumWorkerThreads;
	bool m_bInit;
};

#endif