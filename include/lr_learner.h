// Author: yuandong1@staff.sina.com.cn

#ifndef _LR_LEARNER_H_
#define _LR_LEARNER_H_

#include <vector>
#include "learner.h"

namespace fm {

class LrLearner : public Learner {
 public:
  LrLearner():Learner(), l1_(0.f), l2_(0.f), alpha_(.1f), beta_(1.f) {} 

 protected:
  virtual void Init(const Parameter& params);
  virtual void Update(Example* ex, float p);
  virtual void SaveModel(const Parameter& params);
  virtual void LoadModel(const Parameter& params);

 protected:
  virtual float PredictRaw(Example* ex);
  virtual float EvaluateRaw(Example* ex);
 
 protected:
  float l1_;
  float l2_;
  float alpha_;
  float beta_;
  std::vector<float> n_;
  std::vector<float> z_;
    
};


}  // namespace fm

#endif  // _LR_LEARNER_H_
