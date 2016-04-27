// Author: yuandong1@staff.sina.com.cn
//
#ifndef _AUC_LEARNER_H_
#define _AUC_LEARNER_H_

#include "problem.h"
#include "learner.h"

namespace fm {

class AucLearner : public Learner {
 public:
  virtual void Run(const Parameter& params);

};

}  // namespace fm

#endif  // _AUC_LEARNER_H_
