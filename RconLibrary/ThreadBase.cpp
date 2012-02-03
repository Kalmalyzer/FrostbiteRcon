
#include "ThreadBase.h"

#include <stdexcept>

namespace {

	DWORD __stdcall run(void* arg)
	{
		ThreadBase* thread = static_cast<ThreadBase*>(arg);
		thread->run();
		return 0;
	}

}

ThreadBase::ThreadBase()
{
	m_thread = CreateThread(nullptr, 0, ::run, static_cast<void*>(this), 0, nullptr);
	if (m_thread == NULL)
		throw std::runtime_error("Unable to create thread");
}

ThreadBase::~ThreadBase()
{
}