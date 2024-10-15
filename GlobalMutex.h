#ifndef GLOBAL_MUTEX_H
#define GLOBAL_MUTEX_H

#include <mutex>

//Global mutex variable to lock critical sections
//need to introduce multiple mutexes -- Update
extern std::mutex gmutex;


#endif 