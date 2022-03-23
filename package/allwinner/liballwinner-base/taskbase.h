#ifndef __TASK_BASE_H__
#define __TASK_BASE_H__

#include <locker.h>
#include <thread.h>
#include <list>

using namespace std;

template < typename T >
class TaskBase : public Thread {
public:
    TaskBase();
    virtual ~TaskBase(){};
    int setmaxtask(int max);
    int append(T data);
    int start();
    void stop();

    static const int APPEND_RETRY = 5;
    static const int APPEND_OK = 6;

protected:
    virtual int process(T data) = 0;

private:
    int loop();
    int mMaxTask;

    list< T > mWorkqueue;
	locker mQueuelocker;
	sem mQueuestat;
};

template < typename T >
TaskBase< T >::TaskBase(){
    mMaxTask = 20;
}

template < typename T >
int TaskBase< T >::setmaxtask(int max){
    mMaxTask = max;
}

template < typename T >
int TaskBase< T >::append(T data)
{
	mQueuelocker.lock();
	if ( mWorkqueue.size() > mMaxTask )
	{
		mQueuelocker.unlock();
		return APPEND_RETRY;
	}

	mWorkqueue.push_back( data );
	mQueuelocker.unlock();

	mQueuestat.post();
	return APPEND_OK;
}

template < typename T >
int TaskBase< T >::loop(){
	mQueuestat.wait();

    if(mStop) {
        printf("exit\n");
        return Thread::THREAD_EXIT;
    }

	mQueuelocker.lock();
	if ( mWorkqueue.empty() ){
		mQueuelocker.unlock();
		return 0;
	}
	T d = mWorkqueue.front();
	mWorkqueue.pop_front();
	mQueuelocker.unlock();

	return process(d);
}

template < typename T >
int TaskBase< T >::start(){
    mStop = false;
    return run();
}

template < typename T >
void TaskBase< T >::stop(){
    mStop = true;
    mQueuestat.post();
}
#endif /*__TASK_BASE_H__*/
