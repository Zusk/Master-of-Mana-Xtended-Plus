#ifndef TIME_MEASURE_H
#define TIME_MEASURE_H

//class Module that allows to measure how much time is used for specific tasks
//uses time functions from C++ Boost

struct Task
{
	const TCHAR* sTaskName;
	double dTimePassed;
	double dLastStartTime;
	bool bRunning;
};

class TimeMeasure
{
	public:
	TimeMeasure();
	virtual ~TimeMeasure();

		void Start(const TCHAR* TaskName);
		void Stop(const TCHAR* TaskName);
		void doLog();

	private:
		std::list<Task> m_Tasks;
		TCHAR* m_sInvalid;
};

#endif