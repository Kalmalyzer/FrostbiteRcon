
#include "WaitableThreadBase.h"

#include <stdexcept>
#include <process.h>

namespace {

	unsigned __stdcall run(void* arg)
	{
		WaitableThreadBase* thread = static_cast<WaitableThreadBase*>(arg);
		thread->run();
		_endthreadex(0);
		return 0;
	}

}

WaitableThreadBase::WaitableThreadBase()
{
	m_thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, ::run, static_cast<void*>(this), 0, nullptr)); 
	if (m_thread == 0)
		throw std::runtime_error("Unable to create thread");

	m_quitRequested = CreateEvent(0, FALSE, FALSE, "QuitRequested");
}

void WaitableThreadBase::wait()
{
	WaitForSingleObject(m_thread, INFINITE);
}

void WaitableThreadBase::requestQuit()
{
	SetEvent(m_quitRequested);
}

bool WaitableThreadBase::isQuitRequested()
{
	return WaitForSingleObject(m_quitRequested, 0) != WAIT_TIMEOUT;
}

WaitableThreadBase::~WaitableThreadBase()
{
	requestQuit();
	wait();
	CloseHandle(m_quitRequested);
	CloseHandle(m_thread);
}
