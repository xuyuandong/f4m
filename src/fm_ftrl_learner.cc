// Author: yuandong1@staff.sina.com.cn
//
#include "fm_ftrl_learner.h"
#include <random>
#include <cassert>
#include <iostream>
#include <pmmintrin.h>
#include "util.hpp"

using namespace std;

namespace fm {

const int kAlignByte = 16;
const int kAlign = kAlignByte / sizeof(float);

void FmFtrlLearner::Init(const Parameter& params) {
  Learner::Init(params);
  
  K_ = (int)ceil((float)params.kdim / kAlign) * kAlign;  //16 aligned

  l2_ = params.l2;
  l2_fm_ = params.l2_fm;
  beta_fm_ = params.beta_fm;
  alpha_fm_ = params.alpha_fm;
  beta_ = params.beta;
  alpha_ = params.alpha;

  h_.resize(N_, false);
  w_.resize(N_*(K_+1), 0.0f);
  n_.resize(N_*(K_+1), 0.0f);
  z_.resize(N_*(K_+1), 0.0f);
  assert((uint64_t)w_.data()%16 == 0);
  assert((uint64_t)n_.data()%16 == 0);
  assert((uint64_t)z_.data()%16 == 0);

  // gauss random z
  float dev = params.init_dev;
  std::normal_distribution<float> nd(0.f, dev);
  for (int i = N_; i < N_*(K_+1); ++i) {
    z_[i] = nd(e_);
  }

  std::cout << "factorization K=" << K_ << std::endl;
  if (l2_ > 0.f) {
    std::cout << "1-order l2=" << l2_ << " alpha=" << alpha_ << std::endl;
  } else {
    std::cout << "without 1-order bias" << std::endl;
  }
  std::cout << "2-order l2=" << l2_fm_ << " alpha=" << alpha_fm_ << std::endl;
  std::cout << "z gauss random init_dev=" << dev << std::endl;
}

float FmFtrlLearner::EvaluateRaw(Example* ex) {
  float raw_y = (PredictRaw(ex) - w_[0]);
  return raw_y * (1.f - drop_rate_) + w_[0];
}

float FmFtrlLearner::PredictRaw(Example* ex) {
  float r = 1.f;
  if (norm_) {  // Note: since all feature values are 0 or 1:
    r = 1.f / (float)ex->nodes.size();
  }

  // 2-order dot
  std::vector<float> vn(K_, 0.f);
  std::vector<float> vk2(K_, 0.f);
  for (size_t x = 0; x < ex->nodes.size(); ++x) {
    int offset = N_ + K_ * ex->nodes[x].index;  // i>0
    float* w = w_.data() + offset;
    for (int d = 0; d < K_; d += 4) {
      __m128 XMMw = _mm_load_ps(w+d);
      __m128 XMMvn = _mm_loadu_ps(vn.data() + d);
      __m128 XMMvk2 = _mm_loadu_ps(vk2.data() + d);
      
      XMMvn = _mm_add_ps(XMMvn, XMMw);
      XMMvk2 = _mm_add_ps(XMMvk2, _mm_mul_ps(XMMw, XMMw));
      
      _mm_storeu_ps(vn.data()+d, XMMvn);
      _mm_storeu_ps(vk2.data()+d, XMMvk2);
    }
  }
  
  // 2-order
  float raw_y = 0.f; 
  for (int k = 0; k < K_; ++k) {
    raw_y += (vn[k] * vn[k] - vk2[k]) / 2.f;
  }
  //std::cout << "after 2-order dot " << raw_y << std::endl;
 
  // bias
  raw_y = raw_y * r + w_[0];
  //std::cout << "after bias "  << raw_y << std::endl;
  
  // 1-order
  for (size_t x = 0; x < ex->nodes.size(); ++x) {
    int i = ex->nodes[x].index;  // i>0
    raw_y += w_[i];
  }
  //std::cout << "after 1-order " << raw_y << std::endl;

  return raw_y;
}

void FmFtrlLearner::Update(Example* ex, float p) {
  float r = 1.f;
  if (norm_) {  // Note: since all feature values are 0 or 1:
    r = 1.f / (float)ex->nodes.size();
  }
  
  // gradient
  float g = p - ex->label;
  float g2 = g * g;
  
  // bias
  float sigma = (sqrt(n_[0] + g2) - sqrt(n_[0])) / alpha_;
  z_[0] += g - sigma * w_[0];
  n_[0] += g2;
  w_[0] = -z_[0] * alpha_ / (beta_ + sqrt(n_[0])); 
  //std::cout << "w[0] = " << w_[0] << std::endl;

  // 1-order
  if (l2_ > 0.f) {
    for (size_t x = 0; x < ex->nodes.size(); ++x) {
      int i = ex->nodes[x].index;  // i>0
      sigma = (sqrt(n_[i] + g2) - sqrt(n_[i])) / alpha_;
      z_[i] += g - sigma * w_[i];
      n_[i] += g2;
      w_[i] = - z_[i] / (l2_ + (beta_ + sqrt(n_[i])) / alpha_);
      //std::cout << "w[" << i << "] = " << w_[i] << std::endl;
    }
  }

  // 2-order
  vector<float> svk(K_, 0.f);
  for (size_t x = 0; x < ex->nodes.size(); ++x) {
    int i = ex->nodes[x].index;  // i>0
    h_[i] = true;
    
    int offset = N_ + i*K_;
    float* w = w_.data() + offset;
    for (int d = 0; d < K_; d += 4) {
      __m128 XMMw = _mm_load_ps(w+d);
      __m128 XMMsv = _mm_loadu_ps(svk.data() + d);
      XMMsv = _mm_add_ps(XMMsv, XMMw);
      _mm_storeu_ps(svk.data()+d, XMMsv);
    } 
  }

  __m128 XMMkappa = _mm_set1_ps(g * r);
  __m128 XMMalpha = _mm_set1_ps(alpha_fm_);
  __m128 XMMbeta = _mm_set1_ps(beta_fm_);
  __m128 XMMlambda = _mm_set1_ps(l2_fm_);
  __m128 XMMneg1 = _mm_set1_ps(-1);
  
  for (size_t x = 0; x < ex->nodes.size(); ++x) {
    int offset = N_ + K_ * ex->nodes[x].index;  // i>0
    
    float* w = w_.data() + offset;
    float* n = n_.data() + offset;
    float* z = z_.data() + offset;
    
    for (int d = 0; d < K_; d += 4) {
      __m128 XMMw = _mm_load_ps(w+d);
      __m128 XMMn = _mm_load_ps(n+d);
      __m128 XMMz = _mm_load_ps(z+d);
      __m128 XMMsv = _mm_loadu_ps(svk.data() + d);
      
      //g(i,k) = g * (svk[k] - w_[ik]);
      __m128 XMMg = _mm_mul_ps(XMMkappa, _mm_sub_ps(XMMsv, XMMw));
      // n = n + g*g
      __m128 XMMng = _mm_add_ps(XMMn, _mm_mul_ps(XMMg, XMMg));
      // sigma = (sqrt(n+g*g) - sqrt(n))/alpha 
      __m128 XMMsgm = _mm_div_ps(_mm_sub_ps(_mm_sqrt_ps(XMMng), _mm_sqrt_ps(XMMn)), XMMalpha);
      // z = z + g - sigma * w  
      XMMz = _mm_sub_ps(_mm_add_ps(XMMz, XMMg), _mm_mul_ps(XMMsgm, XMMw));
      // step = l2 + (beta + sqrt(n + g*g)) / alpha
      __m128 XMMstep = _mm_add_ps(XMMlambda, _mm_div_ps(_mm_add_ps(XMMbeta, _mm_sqrt_ps(XMMng)), XMMalpha));
      // w = (sign * l1 - z) / step
      XMMw = _mm_mul_ps(XMMneg1, _mm_div_ps(XMMz, XMMstep));

      // save
      _mm_store_ps(n+d, XMMng);
      _mm_store_ps(z+d, XMMz);
      _mm_store_ps(w+d, XMMw);
    }  // end for d
  }  // end for x
}

void FmFtrlLearner::SaveModel(const Parameter& params) {
  if (params.model_path.empty()) {
    return;
  }

  FILE* fp = fopen(params.model_path.c_str(), "w");
  if (fp == NULL) {
    fprintf(stderr, "Bad model path\n");
    return;
  }

  const vector<float>& w = wbk_;
  fprintf(fp, "#N=%d,K=%d,norm=%d,drop_rate=%f,logr=%f\n", 
      N_, K_, norm_, drop_rate_, negsampling_logr_ + w[0]);
  for (int i = 1; i< N_; ++i) {
    if (!h_[i]) continue;
    fprintf(fp, "%d:%.5f ", i, w[i]);
    for (int k = 0; k < K_; ++k) {
      int ik = N_ + i*K_ + k;
      fprintf(fp, "%.5f ", w[ik]);
    }
    fprintf(fp, "\n");
  }
  
  fclose(fp);
}

void FmFtrlLearner::LoadModel(const Parameter& params) {
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
    if (-1 == sscanf(line, "#N=%d,K=%d,norm=%d,drop_rate=%f,logr=%f", 
          &N_, &K_, &norm_, &drop_rate_, &negsampling_logr_)) {
      fprintf(stderr, "failed to parse FM model header\n");
      exit(1);
    }
    w_.resize(N_*(K_+1), 0.f);
  } else {
    fprintf(stderr, "failed to read FM model\n");
    exit(1);
  }

  while (getline(&line, &length, fp) != (ssize_t)(-1)) {
    int idx = 0;
    int k = -1;
    char* p = line;
    char* last = line;
    for (; *p != '\0'; ++p) {
      if (*p == ':') {
        *p = '\0';
        idx = atoi(last);
        last = p + 1;
        *p = ':';
      } else if (*p == ' ') {
        *p = '\0';
        if (k < 0) {
          w_[idx] = atof(last);
        } else {
          w_[N_ + idx * K_ + k] = atof(last);
        }
        ++k;
        last = p + 1;
        *p = ' ';
      }
    }  // end for line

  }  // end while 


  fclose(fp);
}

}   // namespace fm
