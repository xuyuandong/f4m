// Copyright 2013 Sina Inc All Rights Reserved.
// Author: dongxue@staff.sina.com.cn (Dongxue Liu)

#ifndef BASE_CALLBACK_BINDER_H_
#define BASE_CALLBACK_BINDER_H_

#include "base/callback.h"

namespace base {
namespace bind {
template <bool del, class R, class T, class Arg1>
class _ConstMemberResultCallback_0_1 : public ResultCallback1<R, Arg1> {
 public:
  typedef ResultCallback1<R, Arg1> base;
  typedef const T ObjectT;
  typedef const T* ObjectPtrT;
  typedef R ResultT;
  typedef Arg1 Arg1T;
  typedef R (T::*MemberSignature)(Arg1) const;

  inline _ConstMemberResultCallback_0_1(MemberSignature member)
      : object_(NULL), member_(member) {}

  virtual R Run(Arg1 arg1) {
    if (object_ == NULL) {
      // having not bound yet!
      return R();
    }
    R r = (object_->*member_)(arg1);
    if (del) {
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
    return r;
  }

 protected:
  ObjectPtrT object_;
  MemberSignature member_;
  DISALLOW_COPY_AND_ASSIGN(_ConstMemberResultCallback_0_1);
};

template <bool del, class R, class T, class Arg1>
class _MemberResultCallback_0_1 : public ResultCallback1<R, Arg1> {
 public:
  typedef ResultCallback1<R, Arg1> base;
  typedef T ObjectT;
  typedef T* ObjectPtrT;
  typedef R ResultT;
  typedef Arg1 Arg1T;
  typedef R (T::*MemberSignature)(Arg1);
  inline _MemberResultCallback_0_1(MemberSignature member)
      : object_(NULL), member_(member) {}

  virtual R Run(Arg1 arg1) {
    if (object_ == NULL) {
      // having not bound yet!
      return R();
    }
    R r = (object_->*member_)(arg1);
    if (del) {
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
    return r;
  }

 protected:
  ObjectPtrT object_;
  MemberSignature member_;
  DISALLOW_COPY_AND_ASSIGN(_MemberResultCallback_0_1);
};

template <bool del, class T, class Arg1>
class _ConstMemberResultCallback_0_1<del, void, T, Arg1>
  : public Callback1<Arg1> {
 public:
  typedef Callback1<Arg1> base;
  typedef const T ObjectT;
  typedef const T* ObjectPtrT;
  typedef Arg1 Arg1T;
  typedef void (T::*MemberSignature)(Arg1) const;
  inline _ConstMemberResultCallback_0_1(MemberSignature member)
      : object_(NULL), member_(member) {}

  virtual void Run(Arg1 arg1) {
    if (object_ == NULL) {
      // having not bound yet!
      return;
    }
    (object_->*member_)(arg1);
    if (del) {
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }

 protected:
  ObjectPtrT object_;
  MemberSignature member_;
  DISALLOW_COPY_AND_ASSIGN(_ConstMemberResultCallback_0_1);
};

template <bool del, class T, class Arg1>
class _MemberResultCallback_0_1<del, void, T, Arg1>
  : public Callback1<Arg1> {
 public:
  typedef Callback1<Arg1> base;
  typedef T ObjectT;
  typedef T* ObjectPtrT;
  typedef Arg1 Arg1T;
  typedef void (T::*MemberSignature)(Arg1);
  inline _MemberResultCallback_0_1(MemberSignature member)
      : object_(NULL), member_(member) {}

  virtual void Run(Arg1 arg1) {
    if (object_ == NULL) {
      // having not bound yet!
      return;
    }
    (object_->*member_)(arg1);
    if (del) {
      //  zero out the pointer to ensure segfault if used again
      member_ = NULL;
      delete this;
    }
  }

 protected:
  ObjectPtrT object_;
  MemberSignature member_;
  DISALLOW_COPY_AND_ASSIGN(_MemberResultCallback_0_1);
};
}  // namespace bind
}  // namespace base

namespace base {
template <class Base>
class Binder : public Base {
 public:
  typedef Base base;
  typedef typename base::ObjectPtrT ObjectPtrT;
  typedef typename base::MemberSignature MemberSignature;

  explicit Binder(MemberSignature member) : Base(member) {}

  inline void Bind(ObjectPtrT object) {
    base::object_ = object;
  }

 protected:
  DISALLOW_COPY_AND_ASSIGN(Binder);
};

template <class T, class R, class Arg1>
inline Binder<bind::_ConstMemberResultCallback_0_1<true, R, T, Arg1> >*
NewOneTimeCallback(R (T::*member)(Arg1) const) {
  typedef bind::_ConstMemberResultCallback_0_1<true, R, T, Arg1> Callback;
  return new Binder<Callback>(member);
}

template <class T, class R, class Arg1>
inline Binder<bind::_ConstMemberResultCallback_0_1<false, R, T, Arg1> >*
NewPermanentCallback(R (T::*member)(Arg1) const) {
  typedef bind::_ConstMemberResultCallback_0_1<false, R, T, Arg1> Callback;
  return new Binder<Callback>(member);
}

template <class T, class R, class Arg1>
inline Binder<bind::_MemberResultCallback_0_1<true, R, T, Arg1> >*
NewOneTimeCallback(R (T::*member)(Arg1)) {
  typedef bind::_MemberResultCallback_0_1<true, R, T, Arg1> Callback;
  return new Binder<Callback>(member);
}

template <class T, class R, class Arg1>
inline Binder<bind::_MemberResultCallback_0_1<false, R, T, Arg1> >*
NewPermanentCallback(R (T::*member)(Arg1)) {
  typedef bind::_MemberResultCallback_0_1<false, R, T, Arg1> Callback;
  return new Binder<Callback>(member);
}
}  // namespace base

#endif  // BASE_CALLBACK_BINDER_H_
