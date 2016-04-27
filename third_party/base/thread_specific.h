// Copyright 2013 sina.com All Rights Reserved.
// Module: thread_specific.h
// Author: dongxue@staff.sina.com.cn (Dongxue Liu)
// Created on: 2013-11-1
#ifndef BASE_THREAD_SPECIFIC_H_
#define BASE_THREAD_SPECIFIC_H_

#include <pthread.h>
#include "base/singleton.h"

//
// ThreadSpecific (ThreadLocal) template class
// see pthread_key_create for detailed information of ThreadSpecific/
// ThreadLocal.
//
// Usage case:
// Since ChineseTranslator is not a thread-safe class,
// but we do not need creating multiple instance
// for each scenarios in each same thread context,
// we should use the ThreadSpecific/ThreadLocal object for saving memory usage.
//
// ChineseTranslator* translator =
//    ThreadSpecific<ChineseTranslator> >::GetInstance();
// translator is a per-thread singleton.
//
// NOTE:
// 1. You should *NOT* delete the thread-specific object, like translator
//    above.
// 2. You should *NOT* hold and/or pass the thread-specific object into
//    another thread executing context.
//
template <typename TYPE>
struct ThreadSpecific {
 public:
  static TYPE* GetInstance() {
    return Singleton<ThreadSpecific<TYPE> >::get()->
        GetThreadSpecificInstance();
  }

 private:
  pthread_key_t thread_key_;

  TYPE* GetThreadSpecificInstance() {
    void* data = pthread_getspecific(thread_key_);
    if (data == NULL) {
      data = new TYPE();
      pthread_setspecific(thread_key_, data);
    }
    return static_cast<TYPE*>(data);
  }

  static void Delete(void *p) {
    delete static_cast<TYPE*>(p);
  }

  ThreadSpecific() {
    pthread_key_create(&thread_key_, &Delete);
  }

  ~ThreadSpecific() {
    pthread_key_delete(thread_key_);
  }

  friend struct DefaultSingletonTraits<ThreadSpecific<TYPE> >;
};
#endif  // BASE_THREAD_SPECIFIC_H_
