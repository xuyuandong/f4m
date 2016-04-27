// Author: yuandong1@staff.sina.com.cn

#include <string>
#include "gflags/gflags.h"
#include "util.hpp"
#include "parser.h"
#include "lr_learner.h"
#include "fm_adag_learner.h"
#include "fm_ftrl_learner.h"
#include "ffm_adag_learner.h"
#include "ffm_ftrl_learner.h"
#include "fxx_adag_learner.h"
#include "fxxh_adag_learner.h"
#include "auc_learner.h"

using namespace fm;
using namespace std;
using namespace google;

DEFINE_double(l1, 0, "1-order bias L1 regularization");
DEFINE_double(l2, 0, "1-order bias L2 regularization");
DEFINE_double(l1_fm, 0, "2-order factor L1 regularization");
DEFINE_double(l2_fm, 0.00002, "2-order factor L2 regularization");
DEFINE_double(alpha, 0.1, "sgd learning rate || ftrl 1-order bias learning rate");
DEFINE_double(beta, 1.0, "ftrl 1-order bias init accumulated gradient");
DEFINE_double(alpha_fm, 0.1, "ftrl 2-order factor learning rate");
DEFINE_double(beta_fm, 1.0, "ftrl 2-order factor init accumulated gradient");
DEFINE_double(init_dev, 0.0, "fm factor weight init std.dev");
DEFINE_double(drop_rate, 0.0, "random drop out features rate");
DEFINE_int32(no_self_cross, 1, "ffm feature factor do field-inner cross");
DEFINE_int32(normalization, 1, "ffm instance normalization");
DEFINE_int32(low_freq_filter, 0, "filter frequency < T features");
DEFINE_double(negsampling, 1, "neg sample sub-sampling");
DEFINE_string(ad_fields, "0", "ad feature field index vector");
DEFINE_int32(nbit, 0, "feature hash space");
DEFINE_int32(kdim, 8, "factor vector length");
DEFINE_int32(mfield, 0, "factor field number");
DEFINE_int32(iterations, 1, "max training iteration number");
DEFINE_int32(auto_stop, 1, "early stop");
DEFINE_double(stop_threshold, 0.001, "early stop threshold");
DEFINE_int32(mode, 0, "whether train or test mode");
DEFINE_string(train, "", "training data input path");
DEFINE_string(test, "", "testing data input path");
DEFINE_string(model, "", "model data output path");
DEFINE_string(predict, "", "testing data prediction output path");
DEFINE_string(method, "fm", "learning method");
DEFINE_string(cache, "tmp.cache", "cache file for training data from stdin");

void ParseArgument(Parameter* p) {
  p->l1 = FLAGS_l1;
  p->l2 = FLAGS_l2;
  p->l1_fm = FLAGS_l1_fm;
  p->l2_fm = FLAGS_l2_fm;
  p->alpha = FLAGS_alpha;
  p->beta = FLAGS_beta;
  p->alpha_fm = FLAGS_alpha_fm;
  p->beta_fm = FLAGS_beta_fm;
  p->init_dev = FLAGS_init_dev;
  p->drop_rate = FLAGS_drop_rate;
  p->no_self_cross = FLAGS_no_self_cross;
  p->normalization = FLAGS_normalization;
  p->low_freq_filter = FLAGS_low_freq_filter;
  p->negsampling = FLAGS_negsampling;
  p->nbit = FLAGS_nbit;
  p->kdim = FLAGS_kdim;
  p->mfield = FLAGS_mfield;
  p->iterations = FLAGS_iterations;
  p->auto_stop = (bool)FLAGS_auto_stop;
  p->stop_threshold = FLAGS_stop_threshold;
  p->mode = (bool)FLAGS_mode;
  p->method = FLAGS_method;
  p->train_path = FLAGS_train;
  p->test_path = FLAGS_test;
  p->pred_path = FLAGS_predict;
  p->model_path = FLAGS_model;
  p->cache_path = FLAGS_cache;

  // xx 
  std::vector<std::string> vec;
  SplitString(FLAGS_ad_fields, ',', &vec);
  for (size_t i = 0; i < vec.size(); ++i) {
    p->ad_fields.push_back(atoi(vec[i].c_str()));
  }

  // check
  if (p->test_path.empty() || p->model_path.empty()) {
    p->auto_stop = false;
    p->pred_path.clear();
  }

  if (p->method != "auc" && p->nbit < 1) {
    fprintf(stdout, "Nbit feature hash space is not assigned.\n");
    exit(0);
  }
}

int main(int argc, char* argv[]) {
  ParseCommandLineFlags(&argc, &argv, true);

  Parameter params;
  ParseArgument(&params);

  Learner* learner = NULL;

  if (params.method == "lr") {
    fprintf(stdout, "Use LR (ftrl) learner.\n");
    learner = new LrLearner();
  } else if (params.method == "fmf") {
    fprintf(stdout, "Use FM (ftrl) learner.\n");
    learner = new FmFtrlLearner();
  } else if (params.method == "fma") {
    fprintf(stdout, "Use FM (ada-grad) learner.\n");
    learner = new FmAdagLearner();
  } else if (params.method == "ffmf") {
    fprintf(stdout, "Use FFM (ftrl) learner.\n");
    learner = new FFmFtrlLearner();
  } else if (params.method == "ffma") {
    fprintf(stdout, "Use FFM (ada-grad) learner.\n");
    learner = new FFmAdagLearner();
  } else if (params.method == "fxx") {
    fprintf(stdout, "Use FFMXX (ada-grad) learner.\n");
    learner = new FxxAdagLearner();
  } else if (params.method == "fxxh") {
    fprintf(stdout, "Use FFMXXH (ada-grad) learner.\n");
    learner = new FxxhAdagLearner();
  } else if (params.method == "auc") {
    fprintf(stdout, "Calculate AUC.\n");
    learner = new AucLearner();
  }

  if (learner == NULL) {
    fprintf(stderr, "Bad learning method\n");
    return 1;
  }

  if (!params.mode) {
    learner->Run(params);
  } else {
    learner->Test(params);
  }
  delete learner;
  learner = NULL;

  return 0;
}
