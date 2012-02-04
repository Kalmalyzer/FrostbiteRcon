
#ifndef NonCopyable_h
#define NonCopyable_h

class NonCopyable
{
protected:
	NonCopyable() {}
	~NonCopyable() {}
private:
	NonCopyable(const NonCopyable&);
	const NonCopyable& operator=(const NonCopyable&);
};

#endif
