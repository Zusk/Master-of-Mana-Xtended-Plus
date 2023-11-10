#pragma once

// ThreadOrganizer.h

#include "ThreadObject.h"

#ifndef THREADORGANIZER_H
#define THREADORGANIZER_H

unsigned __stdcall start_thread(void *arg);
unsigned __stdcall start_parallel_loop(void *arg);
unsigned __stdcall start_test_performance(void *arg);

class ThreadOrganizer
{
public:
	
	ThreadOrganizer();
	virtual ~ThreadOrganizer();

	//Access to Members
	ThreadObject& getThread(int iThread);
	void setLoopObject(void* pObject);
	void* getLoopObject();
	void setLoopTarget(void* pObject);
	void* getLoopTarget();
	void setParallelLoopFunction(find_best_function);
	find_best_function getParallelLoopFunction();

	//Allows to Perform a Search
	void parallel_loop(find_best_function, int iLoop, int &iBestValue, int &iBest);

private:

	void CreateThreads();

	//find optimal Number of Worker Threads
	int findOptimalNumWorkerThreads();
	double test_run(int iThreads);

	//Meta Data required by Function that is run parallel
	find_best_function m_find_best_function;
	void* m_pLoopObject;
	void* m_pLoopTarget;

	//Workerthreads and Handles
	ThreadObject* m_paThreads;
	HANDLE* m_aHandle;
	
	int m_iNumWorkerThreads;

	bool m_bInit;	// Workerthreads already created? cannot create them in Constructor 
};

#endif