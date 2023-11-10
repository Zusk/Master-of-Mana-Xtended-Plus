#pragma once

// ThreadObject.h

#ifndef CIV4_THREADOBJECT_H
#define CIV4_THREADOBJECT_H

typedef int (CvGameAI::*find_best_function)(int);

class ThreadObject
{
public:
	
	ThreadObject();
	virtual ~ThreadObject();
	void main();
	int getLoopStart();
	int getLoopEnd();
	void doLoop(find_best_function best_function);
	void setParallelLoop(int iStart, int iEnd, int iBestValue, int iBest);
	int getBestValue();
	int getBest();
	bool hasWork();
	void setHasWork(bool bvalue);
	void setHandle(HANDLE handle);
	HANDLE  m_Semaphore;

private:
	HANDLE  m_Handle;
	unsigned m_iThreadId;
	bool m_bWork;

	int m_iBestValue;
	int m_iBest;
	int m_iFirstLoop, m_iLastLoop;
};

#endif