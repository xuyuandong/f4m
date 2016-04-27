// Author: yuandong1@staff.sina.com.cn

#ifndef _FFM_FTRL_LEARNER_H_
#define _FFM_FTRL_LEARNER_H_

#include <vector>
#include "learner.h"

namespace fm {

class FFmFtrlLearner : public Learner {
 public:
  FFmFtrlLearner():l2_fm_(0.f), alpha_fm_(0.1f), beta_fm_(1.f) {}

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
  float l2_fm_;
  float alpha_fm_;
  float beta_fm_;
  std::vector<float> z_;
  std::vector<float> n_;
  std::vector<bool> h_;
};


}  // namespace fm

#endif  // _FFM_FTRL_LEARNER_H_
