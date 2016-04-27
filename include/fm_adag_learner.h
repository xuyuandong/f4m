// Author: yuandong1@staff.sina.com.cn

#ifndef _FM_ADAG_LEARNER_H_
#define _FM_ADAG_LEARNER_H_

#include <vector>
#include "learner.h"

namespace fm {

class FmAdagLearner : public Learner {
 public:
  FmAdagLearner() : l2_(0.f), l2_fm_(0.f),
    eta_(0.1f), alpha_(0.1f), beta_(1.f) {}

 protected:
  virtual void Init(const Parameter& params);
  virtual void Update(Example* ex, float p);
  virtual void SaveModel(const Parameter& params);
  virtual void LoadModel(const Parameter& params);

 protected:
  virtual float PredictRaw(Example* ex);
  virtual float EvaluateRaw(Example* ex);
 
 protected:
  int K_;
  float l2_;
  float l2_fm_;
  float eta_;
  float alpha_;
  float beta_;
  std::vector<float> g_;
  std::vector<bool> h_;
};


}  // namespace fm

#endif  // _FM_ADAG_LEARNER_H_
