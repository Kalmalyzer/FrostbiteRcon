
#ifndef ThreadBase_h
#define ThreadBase_h

#include <Windows.h>

class ThreadBase
{
public:
	ThreadBase();
	~ThreadBase();

	virtual void run() = 0;
private:
	HANDLE m_thread;
};

#endif
