#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <stdio.h>

class Thread{
public:
	Thread():mStop(false){
		//perror("Thread comstruct\n");
	};
	virtual ~Thread(){
		//perror("Thread discomstruct\n");
	};

public:
	static const int THREAD_EXIT = 0xff;
	static const int THREAD_CONTINUE = 0xf0;

public:
	pthread_t getTid(){
		return mTid;
	};

	int run(){
	    if(pthread_create( &mTid, NULL, run, this ) != 0){
	        perror("create read thread failed!\n");
	        return -1;
	    }
	    if( pthread_detach( mTid ) ){
	        perror("detach read thread failed!\n");
            return -1;
	    }
	    return 0;
	};

	virtual int loop(){
	return THREAD_EXIT;
	};

protected:
	//threads
	bool mStop;

private:
	static void* run(void* arg){
		Thread* t = (Thread*)arg;
	while(1){
		if( THREAD_EXIT == t->loop()) break;
	}
	//perror("Thread finish!\n");
	return (void*)0;
	};

private:
	pthread_t mTid;

};

#endif
