// Author: yuandong1@staff.sina.com.cn
//
#include "auc_learner.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "util.hpp"

namespace fm {

void AucLearner::Run(const Parameter& params) {
  float negsampling = params.negsampling;

  // log loss 
  float logloss = 0.f;
  float loss_count = 0.f;

  // auc array
  const int MAX_SPLIT = 10000;
  float pos_count[MAX_SPLIT];
  float neg_count[MAX_SPLIT];
  memset(pos_count, 0, sizeof(pos_count));
  memset(neg_count, 0, sizeof(neg_count));

  // make sure auc calculate only read from /dev/stdin
  float num_pv = 0.f;
  float num_click = 0.f;
  size_t length = 0;
  char* line = NULL;
  while (getline(&line, &length, stdin) != (ssize_t)(-1)) {
    float label, pred;
    if (-1 == sscanf(line, "%f\t%f", &label, &pred)) {
      fprintf(stderr, "failed to parse auc file line: [%s]\n", line);
      exit(1);
    }

    // correct negsampling prediction ctr
    pred = negsampling * pred / (1.f + (negsampling - 1.f) * pred);
    // correct negsampling logloss
    if (label < 1.f) {
      logloss += LogLoss(0.f, pred) / negsampling;
      loss_count += 1.f/negsampling;
    } else {
      logloss += LogLoss(1.f, pred);
      loss_count += 1.f;
    } 
    // correct negsampling auc
    size_t index = (size_t)(pred * MAX_SPLIT);
    if (label > 0.f) {
      num_pv += 1.f;
      num_click += 1.f;
      pos_count[index] += 1;
    } else {
      num_pv += 1.f/negsampling;
      neg_count[index] += 1.f/negsampling;
    }
  }

  double auc = 0.0;
  double pre_tpr = 1.0;
  double pre_fpr = 1.0;
  for (int i = 0; i < MAX_SPLIT; ++i) {
    float tp = 0.f;
    float fn = 0.f;
    float fp = 0.f;
    float tn = 0.f;
    for (int j = 0; j < i; ++j) {
      fn += pos_count[j];
      tn += neg_count[j];
    }
    for (int j = i; j < MAX_SPLIT; ++j) {
      tp += pos_count[j];
      fp += neg_count[j];
    }

    double tpr = 0.0;
    double fpr = 0.0;
    if (tp + fn > 0) 
      tpr = (double)tp / (double)(tp + fn);
    if (fp + tn > 0)
      fpr = (double)fp / (double)(fp + tn);

    auc += 0.5 * (pre_tpr + tpr) * (pre_fpr - fpr);
    pre_tpr = tpr;
    pre_fpr = fpr;
  }

  fprintf(stdout, "pv count: %d\n", (int)num_pv);
  fprintf(stdout, "click count: %d\n", (int)num_click);
  fprintf(stdout, "auc: %.4lf\n", auc);
  fprintf(stdout, "logloss: %.5lf\n", logloss/loss_count);
}


}  // namespace fm
