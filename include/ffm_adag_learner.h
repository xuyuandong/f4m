// Author: yuandong1@staff.sina.com.cn

#ifndef _FFM_ADAG_LEARNER_H_
#define _FFM_ADAG_LEARNER_H_

#include <vector>
#include "learner.h"

namespace fm {

class FFmAdagLearner : public Learner {
 public:
  FFmAdagLearner() : nsc_(0), eta_(0.1f), l2_fm_(0.f) {}

 protected:
  virtual void Init(const Parameter& params);
  virtual void Update(Example* ex, float p);
  virtual void SaveModel(const Parameter& params);
  virtual void LoadModel(const Parameter& params);

 protected:
  virtual float PredictRaw(Example* ex);
  virtual float EvaluateRaw(Example* ex);
 
 protected:
  int M_;
  int K_;
  int nsc_;
  float eta_;
  float l2_fm_;
  std::vector<float> g_;
  std::vector<bool> h_;
};


}  // namespace fm

#endif  // _FFM_ADAG_LEARNER_H_
