// Author: yuandong1@staff.sina.com.cn

#ifndef _PARSER_H_
#define _PARSER_H_

#include <fstream>
#include "problem.h"

namespace fm {

class Parser {
 public:
  ~Parser() {
    ftr_.close();
    fva_.close();
  }
  void Init(const Parameter& params);
  void TestRun();

  Example* GetExample() {
    return ParseLine(ftr_);
  }
  Example* GetVaExample() {
    return ParseLine(fva_);
  }
  
 private:
  void Reset(std::ifstream& fin) {
    fin.clear();
    fin.seekg(0, std::ios::beg);
  }
  Example* ParseLine(std::ifstream& fin);
 
 private:
  std::ifstream ftr_;
  std::ifstream fva_;
};

}  // namespace fm

#endif  // _PARSER_H_
