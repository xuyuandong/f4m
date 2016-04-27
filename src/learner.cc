// Author: yuandong1@staff.sina.com.cn
//
#include "learner.h"
#include <cmath>
#include <limits>
#include <memory>
#include <vector>
#include <iostream>
#include "util.hpp"

namespace fm {

void Learner::Init(const Parameter& params) {
  N_ = (1 << params.nbit);
  norm_ = params.normalization;
  drop_rate_ = params.drop_rate;
  if (params.negsampling < 1.f) {
    negsampling_logr_ = std::log(params.negsampling);
  }

  std::cout << "====================================>" <<std::endl;
  std::cout << "normalization =" << norm_ << std::endl;
  std::cout << "dropout rate =" << drop_rate_ << std::endl;
  std::cout << "neg sampling logr =" << negsampling_logr_ << std::endl;
  std::cout << "====================================>" <<std::endl;
}

float Learner::Predict(Example* ex) {
  if (drop_rate_ > 0.f) {
    std::vector<Node> left_nodes;
    for (size_t x = 0; x < ex->nodes.size(); ++x) {
      if (u_(e_) > drop_rate_) {
        left_nodes.push_back(ex->nodes[x]);
      }
    }
    ex->nodes.swap(left_nodes);
  }
  float raw_y = PredictRaw(ex);
  return 1.f / (1.f + exp(-std::max(std::min(raw_y, 30.f), -30.f)));
}

float Learner::Evaluate(Example* ex) {
  float raw_y = EvaluateRaw(ex) + negsampling_logr_;
  return 1.f / (1.f + exp(-std::max(std::min(raw_y, 30.f), -30.f)));
}

void Learner::Run(const Parameter& params) {
  Init(params);
  parser_.Init(params);
  
  int64_t begin_time = GetTimeInMs();
  parser_.PreRead(params);
  float end_time = (GetTimeInMs() - begin_time)/1000.f;
  fprintf(stdout, "pre-read elapsed time = %.1fs\n", end_time);
  
  int iter = 1;
  bool stop = false;
  float va_count = 0.f;      
  float va_logloss = 0.f;
  float progress_count = 0.f;
  float progress_logloss = 0.f;
  float best_logloss = std::numeric_limits<float>::max();

  int64_t start_time = GetTimeInMs();
  fprintf(stdout, "iter tr_loss  va_loss  time\n"); 
  
  while (!stop) {
    std::shared_ptr<Example> ex(parser_.GetExample());

    if (ex != NULL) {  // online training
      
      float p = Predict(ex.get());
      Update(ex.get(), p);
      progress_logloss += LogLoss(ex->label, p);
      progress_count += 1;

    } else {  // after one batch, validate
      
      while (true) {
        std::shared_ptr<Example> ec(parser_.GetVaExample());
        if (ec == NULL) {
          break;
        }
          
        float p = Evaluate(ec.get());
        va_logloss += LogLoss(ec->label, p);
        va_count += 1;
      }
     
      float va_per_logloss = va_logloss/va_count;
      float per_elapsed = (GetTimeInMs() - start_time)/(float)(1000*iter);
      fprintf(stdout, "%2d   %2.5f  %2.5f  %.1fs\n", 
          iter, progress_logloss/progress_count, va_per_logloss, per_elapsed);
        
      if (params.auto_stop && va_per_logloss > best_logloss * (1.f + params.stop_threshold)) {
        iter = params.iterations;
        fprintf(stdout, "early stop.\n");
      } else {
        if (va_per_logloss < best_logloss) {
          best_logloss = va_per_logloss;
          BackupModel();
        }
      }

      iter += 1;
      if (iter > params.iterations) {
        float elapsed = (GetTimeInMs() - start_time)/1000.f;
        fprintf(stdout, "training samples = %ld\n"
            "testing samples = %ld\n"
            "elapsed time = %.1fs\n",
            (int64_t)progress_count, (int64_t)va_count, elapsed);
        stop = true;
      }

      progress_logloss = 0.f;
      progress_count = 0.f;
      va_logloss = 0.f;
      va_count = 0.f;
    }
  }

  if (wbk_.empty()) {
    BackupModel();
  }
  SaveModel(params);
}

void Learner::Test(const Parameter& params) {
  LoadModel(params);
  parser_.Init(params);

  int64_t start_time = GetTimeInMs();
  float va_logloss = 0.f;
  float va_count = 0.f;
  while (true) {
    std::shared_ptr<Example> ec(parser_.GetVaExample());
    if (ec == NULL) {
      break;
    }
    float p = Evaluate(ec.get());
    parser_.WritePrediction(ec.get(), p);
    
    va_logloss += LogLoss(ec->label, p);
    va_count += 1;
  }

  float elapsed = (GetTimeInMs() - start_time)/1000.f;
  fprintf(stdout, "test samples = %ld\n"
      "logloss = %.4f\n" 
      "elapsed time = %.1fs\n",
      (int64_t)va_count, va_logloss / va_count, elapsed);
}

}  // namespace fm
