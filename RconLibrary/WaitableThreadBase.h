
#ifndef WaitableThreadBase_h
#define WaitableThreadBase_h

#include "NonCopyable.h"

#include <Windows.h>

class WaitableThreadBase : NonCopyable
{
public:
	WaitableThreadBase();
	virtual ~WaitableThreadBase();

	void wait();
	void requestQuit();

	virtual void run() = 0;

protected:
	bool isQuitRequested();

	HANDLE m_thread;
	HANDLE m_quitRequested;
};

#endif
