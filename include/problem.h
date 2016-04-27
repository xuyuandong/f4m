// Author: yuandong1@staff.sina.com.cn

#ifndef _PROBLEM_H_
#define _PROBLEM_H_

#include <vector>
#include <string>

namespace fm {

struct Node {
  int   field;
  int   index;
  float value;
};

struct Example {
  float label;  // 0,1
  std::vector<Node> nodes;
};

struct Parameter {
  // train opt
  float eta;
  float l1;
  float l2;
  float l1_fm;
  float l2_fm;
  float alpha;
  float beta;
  float alpha_fm;
  float beta_fm; 
  float init_dev;
  // feature opt
  float drop_rate;
  int   no_self_cross;
  int   normalization;
  int   low_freq_filter;
  float negsampling;
  // feature spec
  std::vector<int> ad_fields;
  // problem opt
  int   nbit;
  int   kdim;
  int   mfield;
  // running opt
  int   iterations;
  bool  mode;
  bool  auto_stop;
  float stop_threshold;
  std::string method;
  // data opt
  std::string train_path;
  std::string test_path;
  std::string pred_path;
  std::string model_path;
  std::string cache_path;
};

}  // namespace fm

#endif  // _PROBLEM_H_
