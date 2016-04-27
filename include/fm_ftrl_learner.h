// Author: yuandong1@staff.sina.com.cn

#ifndef _FM_FTRL_LEARNER_H_
#define _FM_FTRL_LEARNER_H_

#include <vector>
#include "learner.h"

namespace fm {

class FmFtrlLearner : public Learner {
 public:
  FmFtrlLearner() : l2_(0.f), l2_fm_(0.f), 
    alpha_fm_(.01f), beta_fm_(1.f), alpha_(.1f), beta_(1.f) {}

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
  float alpha_fm_;
  float beta_fm_;
  float alpha_;
  float beta_;
  std::vector<float> n_;
  std::vector<float> z_;
  std::vector<bool> h_;
};


}  // namespace fm

#endif  // _FM_FTRL_LEARNER_H_
