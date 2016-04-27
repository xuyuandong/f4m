// Author: yuandong1@staff.sina.com.cn
//
#include "lr_learner.h"
#include <iostream>
#include "util.hpp"

using namespace std;

namespace fm {

void LrLearner::Init(const Parameter& params) {
  N_ = 1 << params.nbit;
  w_.resize(N_, 0.0f);
  n_.resize(N_, 0.0f);
  z_.resize(N_, 0.0f);

  drop_rate_ = params.drop_rate;
  l1_ = params.l1;
  l2_ = params.l2;
  alpha_ = params.alpha;
  beta_ = params.beta;
}

float LrLearner::EvaluateRaw(Example* ex) {
  float raw_y = PredictRaw(ex);
  return (raw_y - w_[0]) * (1.f - drop_rate_) + w_[0];
}

float LrLearner::PredictRaw(Example* ex) {
  // bias
  float raw_y = w_[0];
  //std::cout << "after bias "  << raw_y << std::endl;

  // 1-order
  for (size_t x = 0; x < ex->nodes.size(); ++x) {
    int i = ex->nodes[x].index;  // i>0
    raw_y += w_[i];
  }
  //std::cout << "after 1-order " << raw_y << std::endl;
  
  return raw_y;
}

void LrLearner::Update(Example* ex, float p) {
  float g = p - ex->label;
  float g2 = g * g;
  
  // bias
  float sigma = (sqrt(n_[0] + g2) - sqrt(n_[0])) / alpha_;
  z_[0] += g - sigma * w_[0];
  n_[0] += g2;
  
  w_[0] = -z_[0] * alpha_ / (beta_ + sqrt(n_[0])); 
  //std::cout << "w[0] = " << w_[0] << std::endl;

  // 1-order
  for (size_t x = 0; x < ex->nodes.size(); ++x) {
    int i = ex->nodes[x].index;  // i>0
    sigma = (sqrt(n_[i] + g2) - sqrt(n_[i])) / alpha_;
    z_[i] += g - sigma * w_[i];
    n_[i] += g2;
    
    float sn = sign(z_[i]);
    if (sn * z_[i] <= l1_) {
      w_[i] = 0.f;
    } else {
      float step = (l2_ + (beta_ + sqrt(n_[i])) / alpha_);
      w_[i] = (sn * l1_ - z_[i]) / step;
    }
    //std::cout << "w[" << i << "] = " << w_[i] << std::endl;
  }

}

void LrLearner::SaveModel(const Parameter& params) {
  if (params.model_path.empty()) {
    return;
  }

  FILE* fp = fopen(params.model_path.c_str(), "w");
  if (fp == NULL) {
    fprintf(stderr, "Bad model path\n");
    return;
  }

  const vector<float>& w = wbk_;
  fprintf(fp, "#N=%d,drop_rate=%f,logr=%f\n", 
      N_, drop_rate_, negsampling_logr_);
  for (int i = 0; i< N_; ++i) {
    if (w[i] != 0.f) {
      fprintf(fp, "%d:%.5f\n", i, w[i]);
    }
  }
  
  fclose(fp);
}

void LrLearner::LoadModel(const Parameter& params) {
  if (params.model_path.empty()) {
    return;
  }

  FILE* fp = fopen(params.model_path.c_str(), "r");
  if (fp == NULL) {
    fprintf(stderr, "Bad model path\n");
    return;
  }

  size_t length = 0;
  char* line = NULL;
  if (getline(&line, &length, fp) != (ssize_t)(-1)) {
    if (-1 == sscanf(line, "#N=%d,drop_rate=%f,logr=%f", 
          &N_, &drop_rate_, &negsampling_logr_)) {
      fprintf(stderr, "failed to parse LR model header\n");
      exit(1);
    }
    w_.resize(N_, 0.f);
  } else {
    fprintf(stderr, "failed to read LR model\n");
    exit(1);
  }

  while (getline(&line, &length, fp) != (ssize_t)(-1)) {
    int idx;
    float weight;
    if (-1 != sscanf(line, "%d:%f", &idx, &weight)) {
      w_[idx] = weight; 
    }
  }

  fclose(fp);
}


}   // namespace fm
