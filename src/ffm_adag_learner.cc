// Author: yuandong1@staff.sina.com.cn
//
#include "ffm_adag_learner.h"
#include <random>
#include <iostream>
#include <cassert>
#include <pmmintrin.h>
#include "util.hpp"

using namespace std;

namespace fm {
  
const int kAlignByte = 16;
const int kAlign = kAlignByte / sizeof(float);

void FFmAdagLearner::Init(const Parameter& params) {
  Learner::Init(params);
  
  h_.resize(N_, false);
  M_ = params.mfield;
  K_ = (int)ceil((float)params.kdim / kAlign) * kAlign;

  l2_fm_ = params.l2_fm;
  eta_ = params.alpha_fm;
  nsc_ = params.no_self_cross;

  float coef = params.init_dev / sqrt(K_);
  w_.resize(N_ * M_ * K_, coef);
  g_.resize(N_ * M_ * K_, 1);
  assert((uint64_t)w_.data()%16 == 0);
  assert((uint64_t)g_.data()%16 == 0);
  //std::cout << "w addr: " << (uint64_t)w_.data() << " " << (uint64_t)w_.data()%16 << std::endl;
  //std::cout << "g addr: " << (uint64_t)g_.data() << " " << (uint64_t)g_.data()%16 << std::endl;
  
  std::uniform_real_distribution<float> ud(0.f, coef);
  for (size_t i = 0; i < w_.size(); ++i) {
    w_[i] = ud(e_);
  }

  std::cout << "factorization K=" << K_ << " M=" << M_ << std::endl;
  std::cout << "2-order l2=" << l2_fm_ << " eta=" << eta_ << std::endl;
  std::cout << "w uniform init_coef=" << coef << std::endl;
  if (nsc_ > 0) {
    std::cout << "Use no field-inner cross feature" << std::endl;
  }
}

float FFmAdagLearner::EvaluateRaw(Example* ex) {
  return PredictRaw(ex) * (1.f - drop_rate_);
}

float FFmAdagLearner::PredictRaw(Example* ex) {
  float r = 1.f;
  if (norm_) {  // Note: since all feature values are 0 or 1:
    r = 1.f / ex->nodes.size();
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

      if (nsc_ && fx == fy) continue;  // no field-inner cross
            
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

void FFmAdagLearner::Update(Example* ex, float p) {
  float r = 1.f;
  if (norm_) {  // Note: since all feature values are 0 or 1:
    r = 1.f / (float)ex->nodes.size();
  }
  
  // gradient
  float kappa = p - ex->label;
  int Maligned = M_ * K_;
    
  __m128 XMMkappa = _mm_set1_ps(kappa);
  __m128 XMMeta = _mm_set1_ps(eta_);
  __m128 XMMlambda = _mm_set1_ps(l2_fm_);

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
      
      if (nsc_ && fx == fy) continue;  // no field-inner cross
            
      int offset1 = ix * Maligned + fy * K_;
      int offset2 = iy * Maligned + fx * K_;
      float* w1 = w_.data() + offset1;
      float* w2 = w_.data() + offset2;
      float* wg1 = g_.data() + offset1;
      float* wg2 = g_.data() + offset2;

      __m128 XMMv = _mm_set1_ps(r); //(vx*vy*r);
      __m128 XMMkappav = _mm_mul_ps(XMMkappa, XMMv);
      
      for (int d = 0; d < K_; d += 4) {
        __m128 XMMw1 = _mm_load_ps(w1+d);
        __m128 XMMw2 = _mm_load_ps(w2+d);

        __m128 XMMwg1 = _mm_load_ps(wg1+d);
        __m128 XMMwg2 = _mm_load_ps(wg2+d);

        __m128 XMMg1 = _mm_add_ps(
            _mm_mul_ps(XMMlambda, XMMw1),
            _mm_mul_ps(XMMkappav, XMMw2));
        __m128 XMMg2 = _mm_add_ps(
            _mm_mul_ps(XMMlambda, XMMw2),
            _mm_mul_ps(XMMkappav, XMMw1));

        XMMwg1 = _mm_add_ps(XMMwg1, _mm_mul_ps(XMMg1, XMMg1));
        XMMwg2 = _mm_add_ps(XMMwg2, _mm_mul_ps(XMMg2, XMMg2));

        XMMw1 = _mm_sub_ps(XMMw1, _mm_mul_ps(XMMeta, 
              _mm_mul_ps(_mm_rsqrt_ps(XMMwg1), XMMg1)));
        XMMw2 = _mm_sub_ps(XMMw2, _mm_mul_ps(XMMeta, 
              _mm_mul_ps(_mm_rsqrt_ps(XMMwg2), XMMg2)));

        _mm_store_ps(w1+d, XMMw1);
        _mm_store_ps(w2+d, XMMw2);

        _mm_store_ps(wg1+d, XMMwg1);
        _mm_store_ps(wg2+d, XMMwg2);
      }  // end for d

    }  // end for y
  }  // end for x
}

void FFmAdagLearner::SaveModel(const Parameter& params) {
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

void FFmAdagLearner::LoadModel(const Parameter& params) {
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
    assert(0 == ((uint64_t)w_.data())%16); 
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
