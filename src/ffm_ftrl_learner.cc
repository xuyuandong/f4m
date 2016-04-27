// Author: yuandong1@staff.sina.com.cn
//
#include "ffm_ftrl_learner.h"
#include <random>
#include <iostream>
#include <cassert>
#include <pmmintrin.h>
#include "util.hpp"

using namespace std;

namespace fm {
  
const int kAlignByte = 16;
const int kAlign = kAlignByte / sizeof(float);

void FFmFtrlLearner::Init(const Parameter& params) {
  Learner::Init(params);
  h_.resize(N_, false);
  
  M_ = params.mfield;
  K_ = (int)ceil((float)params.kdim / kAlign) * kAlign;

  l2_fm_ = params.l2_fm;
  beta_fm_ = params.beta_fm;
  alpha_fm_ = params.alpha_fm;

  w_.resize(N_* M_ * K_, 0.f);
  z_.resize(N_* M_ * K_, 1.f);
  n_.resize(N_* M_ * K_, 0.f);
  assert((uint64_t)w_.data()%16 == 0);
  assert((uint64_t)n_.data()%16 == 0);
  assert((uint64_t)z_.data()%16 == 0);

  float coef = params.init_dev / sqrt(K_);
  for (size_t i = 0; i < w_.size(); ++i) {
    w_[i] = coef;
    z_[i] = coef;
  }
  std::cout << "factorization K=" << K_ << " M=" << M_ << std::endl;
  std::cout << "2-order l2=" << l2_fm_ << " alpha=" << alpha_fm_ << std::endl;
  std::cout << "w uniform init_coef=" << coef << std::endl;
  std::cout << "z uniform init_coef=" << coef << std::endl;
}

float FFmFtrlLearner::EvaluateRaw(Example* ex) {
  return PredictRaw(ex) * (1.f - drop_rate_);
}

float FFmFtrlLearner::PredictRaw(Example* ex) {
  float r = 1.f;
  if (norm_) {  // Note: since all feature values are 0 or 1:
    r = 1.f / (float)ex->nodes.size();
  }
 
  int Maligned = M_ * K_;
  __m128 XMMt = _mm_setzero_ps();
  
  for (size_t x = 0; x < ex->nodes.size(); ++x) {
    int ix = ex->nodes[x].index;
    int fx = ex->nodes[x].field;
    //float vx = ex->nodes[x].value; 
    if (ix >= N_ || fx >= M_) continue;

    for (size_t y = x+1; y < ex->nodes.size(); ++y) {
      int iy = ex->nodes[y].index;
      int fy = ex->nodes[y].field;
      //float vy = ex->nodes[y].value; 
      if (iy >= N_ || fy >= M_) continue;
            
      float* w1 = w_.data() + ix*Maligned + fy*K_;
      float* w2 = w_.data() + iy * Maligned + fx * K_;
      
      __m128 XMMv = _mm_set1_ps(r); //(vx*vy*r);
        
      for (int d = 0; d < K_; d += 4) {
        __m128  XMMw1 = _mm_load_ps(w1+d);
        __m128  XMMw2 = _mm_load_ps(w2+d);

        XMMt = _mm_add_ps(XMMt, 
            _mm_mul_ps(_mm_mul_ps(XMMw1, XMMw2), XMMv));
      }
    }
    
  }
    
  XMMt = _mm_hadd_ps(XMMt, XMMt);
  XMMt = _mm_hadd_ps(XMMt, XMMt);
  
  float raw_y = 0.f; 
  _mm_store_ss(&raw_y, XMMt);
  return raw_y;
}

void FFmFtrlLearner::Update(Example* ex, float p) {
  float r = 1.f;
  if (norm_) {  // Note: since all feature values are 0 or 1:
    r = 1.f / (float)ex->nodes.size();
  }
  
  // gradient
  float kappa = p - ex->label;
  int Maligned = M_ * K_;
    
  __m128 XMMkappa = _mm_set1_ps(kappa);
  __m128 XMMalpha = _mm_set1_ps(alpha_fm_);
  __m128 XMMbeta = _mm_set1_ps(beta_fm_);
  __m128 XMMlambda = _mm_set1_ps(l2_fm_);
  __m128 XMMneg1 = _mm_set1_ps(-1);

  for (size_t x = 0; x < ex->nodes.size(); ++x) {
    int ix = ex->nodes[x].index;
    int fx = ex->nodes[x].field;
    //float vx = ex->nodes[x].value; 
    if (fx >= M_) continue;
    h_[ix] = true;

    for (size_t y = x+1; y < ex->nodes.size(); ++y) {
      int iy = ex->nodes[y].index;
      int fy = ex->nodes[y].field;
      //float vy = ex->nodes[y].value; 
      if (fy >= M_) continue;
      
      int offset1 = ix * Maligned + fy * K_;
      int offset2 = iy * Maligned + fx * K_;

      float* w1 = w_.data() + offset1;
      float* w2 = w_.data() + offset2;
      float* n1 = n_.data() + offset1;
      float* n2 = n_.data() + offset2;
      float* z1 = z_.data() + offset1;
      float* z2 = z_.data() + offset2;

      __m128 XMMv = _mm_set1_ps(r); //(vx*vy*r);
      __m128 XMMkappav = _mm_mul_ps(XMMkappa, XMMv);

      for (int d = 0; d < K_; d += 4) {
        __m128 XMMw1 = _mm_load_ps(w1+d);
        __m128 XMMw2 = _mm_load_ps(w2+d);

        __m128 XMMn1 = _mm_load_ps(n1+d);
        __m128 XMMn2 = _mm_load_ps(n2+d);

        __m128 XMMz1 = _mm_load_ps(z1+d);
        __m128 XMMz2 = _mm_load_ps(z2+d);

        // g(wi) = g * wj
        __m128 XMMg1 = _mm_mul_ps(XMMkappav, XMMw2);
        __m128 XMMg2 = _mm_mul_ps(XMMkappav, XMMw1);

        // n = n + g*g
        __m128 XMMng1 = _mm_add_ps(XMMn1, _mm_mul_ps(XMMg1, XMMg1));
        __m128 XMMng2 = _mm_add_ps(XMMn2, _mm_mul_ps(XMMg2, XMMg2));

        // sigma = (sqrt(n+g*g) - sqrt(n))/alpha
        __m128 XMMsgm1 = _mm_div_ps(_mm_sub_ps(_mm_sqrt_ps(XMMng1), _mm_sqrt_ps(XMMn1)), XMMalpha);
        __m128 XMMsgm2 = _mm_div_ps(_mm_sub_ps(_mm_sqrt_ps(XMMng2), _mm_sqrt_ps(XMMn2)), XMMalpha);

        // z = z + g - sigma * w
        XMMz1 = _mm_sub_ps(_mm_add_ps(XMMz1, XMMg1), _mm_mul_ps(XMMsgm1, XMMw1));
        XMMz2 = _mm_sub_ps(_mm_add_ps(XMMz2, XMMg2), _mm_mul_ps(XMMsgm2, XMMw2));

        // step = l2 + (beta + sqrt(n+g*g))/alpha
        __m128 XMMstep1 = _mm_add_ps(XMMlambda, _mm_div_ps(_mm_add_ps(XMMbeta, _mm_sqrt_ps(XMMng1)), XMMalpha));
        __m128 XMMstep2 = _mm_add_ps(XMMlambda, _mm_div_ps(_mm_add_ps(XMMbeta, _mm_sqrt_ps(XMMng2)), XMMalpha));

        // w = -z / step
        XMMw1 = _mm_mul_ps(XMMneg1, _mm_div_ps(XMMz1, XMMstep1));
        XMMw2 = _mm_mul_ps(XMMneg1, _mm_div_ps(XMMz2, XMMstep2));

        // save
        _mm_store_ps(n1+d, XMMng1);
        _mm_store_ps(n2+d, XMMng2);

        _mm_store_ps(z1+d, XMMz1);
        _mm_store_ps(z2+d, XMMz2);

        _mm_store_ps(w1+d, XMMw1);
        _mm_store_ps(w2+d, XMMw2);

      }  // end for d

    }  // end for y
  }  // end for x
}

void FFmFtrlLearner::SaveModel(const Parameter& params) {
  if (params.model_path.empty()) {
    return;
  }

  FILE* fp = fopen(params.model_path.c_str(), "w");
  if (fp == NULL) {
    fprintf(stderr, "Bad model path\n");
    return;
  }

  const vector<float>& w = wbk_;
  fprintf(fp, "#N=%d,K=%d,M=%d,norm=%d,drop_rate=%f,logr=%f\n", 
      N_, K_, M_, norm_, drop_rate_, negsampling_logr_);

  K_ = (int)ceil((float)K_ / kAlign) * kAlign;
  for (int i = 0; i< N_; ++i) {
    if (!h_[i]) continue;
    for (int f = 0; f < M_; ++f) {
      fprintf(fp, "%d,%d:", i, f);
      for (int k = 0; k < K_; ++k) {
        int v = k + (i*M_ + f)*K_;
        fprintf(fp, "%.5f ", w[v]);
      }
      fprintf(fp, "\n");
    }
  }

  fclose(fp);
}

void FFmFtrlLearner::LoadModel(const Parameter& params) {
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
    if (-1 == sscanf(line, "#N=%d,K=%d,M=%d,norm=%d,drop_rate=%f,logr=%f", 
          &N_, &K_, &M_, &norm_, &drop_rate_, &negsampling_logr_)) {
      fprintf(stderr, "failed to parse FM model header\n");
      exit(1);
    }

    K_ = (int)ceil((float)K_ / kAlign) * kAlign;
    w_.resize(N_*M_*K_, 0.f);
  } else {
    fprintf(stderr, "failed to read FFM model\n");
    exit(1);
  }

  while (getline(&line, &length, fp) != (ssize_t)(-1)) {
    int idx = 0;
    int field = 0;
    int k = 0;
    char* p = line;
    char* last = line;
    for (; *p != '\0'; ++p) {
      if (*p == ',') {
        *p = '\0';
        idx = atoi(last);
        last = p + 1;
        *p = ',';
      } else if (*p == ':') {
        *p = '\0';
        field = atoi(last);
        last = p + 1;
        *p = ':';
      } else if (*p == ' ') {
        *p = '\0';
        w_[(idx*M_+field)*K_ + k] = atof(last);
        ++k;
        last = p + 1;
        *p = ' ';
      }
    } // end for line

  }  // end while

  fclose(fp);
}

}   // namespace fm
