// Author: yuandong1@staff.sina.com.cn
//
#include "fm_adag_learner.h"
#include <random>
#include <cassert>
#include <iostream>
#include <pmmintrin.h>
#include "util.hpp"

using namespace std;

namespace fm {

const int kAlignByte = 16;
const int kAlign = kAlignByte / sizeof(float);

void FmAdagLearner::Init(const Parameter& params) {
  Learner::Init(params);
  
  K_ = (int)ceil((float)params.kdim / kAlign) * kAlign;  //16 aligned
 
  l2_ = params.l2;
  l2_fm_ = params.l2_fm;
  eta_ = params.alpha_fm;
  beta_ = params.beta;
  alpha_ = params.alpha;

  h_.resize(N_, false);
  w_.resize(N_*(K_+1), 0.f);
  g_.resize(N_*(K_+1), 1.f);
  assert((uint64_t)w_.data()%16 == 0);
  assert((uint64_t)g_.data()%16 == 0);

  // gauss random z
  float dev = params.init_dev;
  std::normal_distribution<float> nd(0.f, dev);
  for (int i = N_; i < N_*(K_+1); ++i) {
    w_[i] = nd(e_);
  }

  std::cout << "factorization K=" << K_ << std::endl;
  std::cout << "without 1-order bias" << std::endl;
  std::cout << "2-order l2=" << l2_fm_ << " eta=" << eta_ << std::endl;
  std::cout << "w gauss random init_dev=" << dev << std::endl;
}

float FmAdagLearner::EvaluateRaw(Example* ex) {
  float raw_y = (PredictRaw(ex) - w_[0]);
  return raw_y * (1.f - drop_rate_) + w_[0];
}

float FmAdagLearner::PredictRaw(Example* ex) {
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
  

  float raw_y = 0.f; 
  for (int k = 0; k < K_; ++k) {
    raw_y += (vn[k] * vn[k] - vk2[k]) / 2.f;
  }
  //std::cout << "after 2-order dot " << raw_y << std::endl;
 
  // bias
  raw_y = raw_y * r + w_[0];
  //std::cout << "after bias "  << raw_y << std::endl;
  
  return raw_y;
}

void FmAdagLearner::Update(Example* ex, float p) {
  float r = 1.f;
  if (norm_) {  // Note: since all feature values are 0 or 1:
    r = 1.f / (float)ex->nodes.size();
  }
  
  // gradient
  float g = p - ex->label;
  float g2 = g * g;
  
  // bias
  float sigma = (sqrt(w_[1] + g2) - sqrt(w_[1])) / alpha_;
  w_[2] += g - sigma * w_[0];
  w_[1] += g2;
  w_[0] = -w_[2] * alpha_ / (beta_ + sqrt(w_[1])); 
  //std::cout << "w[0] = " << w_[0] << std::endl;

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
  __m128 XMMeta = _mm_set1_ps(eta_);
  __m128 XMMlambda = _mm_set1_ps(l2_fm_);
  
  for (size_t x = 0; x < ex->nodes.size(); ++x) {
    int offset = N_ + K_ * ex->nodes[x].index;  // i>0
    
    float* w = w_.data() + offset;
    float* g = g_.data() + offset;
    
    for (int d = 0; d < K_; d += 4) {
      __m128 XMMw = _mm_load_ps(w+d);
      __m128 XMMng = _mm_load_ps(g+d);
      __m128 XMMsv = _mm_loadu_ps(svk.data() + d);
      
      //g(i,k) = g * (svk[k] - w_[ik])
      __m128 XMMg = _mm_mul_ps(XMMkappa, _mm_sub_ps(XMMsv, XMMw));
      //g(i,k) = g(i,k) + l2 * w_[ik]
      XMMg = _mm_add_ps(XMMg, _mm_mul_ps(XMMlambda, XMMw));

      // n = n + g*g
      XMMng = _mm_add_ps(XMMng, _mm_mul_ps(XMMg, XMMg));

      // w = w - g * eta/sqrt(ng)
      XMMw = _mm_sub_ps(XMMw, _mm_mul_ps(XMMeta, 
            _mm_mul_ps(_mm_rsqrt_ps(XMMng), XMMg)));

      // save
      _mm_store_ps(w+d, XMMw);
      _mm_store_ps(g+d, XMMng);
    }  // end for d
  }  // end for x
}

void FmAdagLearner::SaveModel(const Parameter& params) {
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
    fprintf(fp, "%d:0.0 ", i);
    //fprintf(fp, "%d:%.5f ", i, w[i]);
    for (int k = 0; k < K_; ++k) {
      int ik = N_ + i*K_ + k;
      fprintf(fp, "%.5f ", w[ik]);
    }
    fprintf(fp, "\n");
  }
  
  fclose(fp);
}

void FmAdagLearner::LoadModel(const Parameter& params) {
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
        if (k >= 0) {
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
