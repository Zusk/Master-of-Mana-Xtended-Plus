#ifndef TIME_MEASURE_H
#define TIME_MEASURE_H

//class Module that allows to measure how much time is used for specific tasks
//uses time functions from C++ Boost

//set it to true/false to activa/deactivate this module
#define DO_MEASURE true

//we allocate Memory static so that we prevent Memory fragmentation
#define MAX_TASKS 100

using namespace std;

struct Task
{
	TCHAR* sTaskName;
	double dTimePassed;
	double dLastStartTime;
};

class TimeMeasure
{
	public:
	TimeMeasure();
	virtual ~TimeMeasure();

		void Start(const TCHAR* TaskName);
		void Stop(const TCHAR* TaskName);
		void doLog();
		int maxTasks();

	private:
		int m_iMaxTasks;
		TCHAR* m_sInvalid;
		Task* m_Tasks;
};

#endif