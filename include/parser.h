// Author: yuandong1@staff.sina.com.cn
//
#ifndef _PARSER_H_
#define _PARSER_H_

#include <cstdio>
#include <string>
#include <random>
#include "problem.h"

namespace fm {
  
class Parser {
 public:
  Parser();
  ~Parser();
  void Init(const Parameter& params);
  void PreRead(const Parameter& params);

  Example* GetExample() {
    return ParseLine(ftr_, true);
  }
  Example* GetVaExample() {
    float tmp = negsampling_;
    negsampling_ = 1.f;
    Example* ex = ParseLine(fva_, false);
    negsampling_ = tmp;
    return ex;
  }
  void WritePrediction(Example* ex, float pred) {
    if (fpd_ != NULL) {
      fprintf(fpd_, "%.1f\t%.5f\n", ex->label, pred);
    }
  }

 private:
  void SetTrainFileToCache();
  Example* ParseLine(FILE* fp, bool train);
  Example* ParseLineImpl(FILE* fp, char* buf, int buf_size);

 private:
  // file pointer
  FILE* ftr_;  // train reader
  FILE* fva_;  // test reader
  FILE* fpd_;  // predict writer
  FILE* fch_;  // cache writer

  char* fbuf_;
  char* fbuf_va_;
  char* fbuf_ch_;
  std::string cache_file_;
  
  // feature hash space
  int nspace_;

  // pre-process
  std::vector<int> counter_;  // frequency counter
  int low_freq_filter_threshold_;
  
  // neg sample sampling rate
  float negsampling_; 

  std::random_device rd_;
  std::default_random_engine e_; 
  std::uniform_real_distribution<float> u_;
};

}  // namespace fm

#endif  // _PARSER_H_
