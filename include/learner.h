// Author: yuandong1@staff.sina.com.cn
//
#ifndef _LEARNER_H_
#define _LEARNER_H_

#include <random>
#include "problem.h"
#include "parser.h"

namespace fm {

class Learner {
 public:
  Learner() : drop_rate_(0.f), negsampling_logr_(0.f), 
    e_(rd_()), u_(0.f, 1.f) {}
  virtual ~Learner() {}

  virtual void Run(const Parameter& params);
  virtual void Test(const Parameter& params);

 protected:
  virtual void Init(const Parameter& params);
  virtual float Predict(Example* ex);
  virtual float Evaluate(Example* ex);
  virtual void Update(Example* ex, float p) {}

  virtual void LoadModel(const Parameter& params) {}
  virtual void SaveModel(const Parameter& params) {}
  virtual void BackupModel() {
    wbk_.assign(w_.begin(), w_.end());
  }

 protected:
  virtual float PredictRaw(Example* ex) { return 0.f; }
  virtual float EvaluateRaw(Example* ex) { return 0.f; }

 protected:
  Parser parser_;

  int N_;
  int norm_;
  float drop_rate_;
  float negsampling_logr_;
  std::vector<float> w_;
  std::vector<float> wbk_;
    
  std::random_device rd_;
  std::default_random_engine e_; 
  std::uniform_real_distribution<float> u_;
};

}  // namespace fm

#endif  // _LEARNER_H_
