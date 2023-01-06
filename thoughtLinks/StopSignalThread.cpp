/*
 * Modification History
 *
 * 2002-April-4    Jason Rohrer
 * Created.
 * Changed to reflect the fact that the base class
 * destructor is called *after* the derived class destructor.
 */



#include "konspire2b/reference/threads/StopSignalThread.h"



StopSignalThread::StopSignalThread()
    : mStopped( false ), mStopLock( new MutexLock() ) {

    }



StopSignalThread::~StopSignalThread() {

    delete mStopLock;
    }



char StopSignalThread::isStopped() {
    mStopLock->lock();
    char stoped = mStopped;
    mStopLock->unlock();

    return stoped;
    }



void StopSignalThread::stop() {
    mStopLock->lock();
    mStopped = true;
    mStopLock->unlock();
    }

