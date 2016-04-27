// Author: yuandong1@staff.sina.com.cn

#ifndef _FXX_ADAG_LEARNER_H_
#define _FXX_ADAG_LEARNER_H_

#include <vector>
#include "learner.h"

namespace fm {

class FxxAdagLearner : public Learner {
 public:
  FxxAdagLearner():eta_(0.1f), l2_fm_(0.f), alpha_(.1f), beta_(1.f) {}

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
  float eta_;
  float l2_fm_;
  float alpha_;
  float beta_;
  std::vector<int> ads_;
  std::vector<float> g_;
  std::vector<bool> h_;
};


}  // namespace fm

#endif  // _FXX_ADAG_LEARNER_H_
