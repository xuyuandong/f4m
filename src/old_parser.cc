// Author: yuandong1@staff.sina.com.cn

#include "parser.h"
#include <iostream>
#include "util.hpp"

using namespace std;

namespace fm {

void Parser::Init(const Parameter& params) {
  ftr_.open(params.train_path);
  if (ftr_.fail()) {
    std::cerr << "Bad training data" << std::endl;
    exit(1);
  }
  if (!params.test_path.empty()) {
    fva_.open(params.test_path);
    if (fva_.fail()) {
      std::cerr << "Bad testing data" << std::endl;
      exit(1);
    }
  }
}

void Parser::TestRun() {
  int64_t start_time = GetTimeInMs();
  int t = 1;
  while (1) {
    Example* ex = GetExample();
    if (ex == NULL) {
      std::cout << t << " times of parsing" << std::endl;
      if (t++ > 99)
        break;
    }
    delete ex;
    ex = NULL;
  }
  int64_t elapsed = GetTimeInMs() - start_time;
  fprintf(stdout, "repeated read times = %d, elapsed time = %ld\n", \
      t, elapsed);
}


Example* Parser::ParseLine(ifstream& fin) {
  char buf[4096];
  if (fin.eof() || fin.getline(buf, sizeof(buf)).fail()) {
    Reset(fin);
    return NULL;  // end of file
  }
  //std::cout << buf << std::endl;

  vector<string> arr;
  SplitString(buf, ' ',  &arr);
  
  Example* ex = new Example();
  ex->label = atoi(arr[0].c_str());
  ex->nodes.resize(arr.size() - 1);

  vector<string> inner_arr;
  for (size_t i = 1; i < arr.size(); ++i) {
    inner_arr.clear();
    SplitString(arr[i], ':', &inner_arr);
    ex->nodes[i-1].field = atoi(inner_arr[0].c_str());
    ex->nodes[i-1].index = atoi(inner_arr[1].c_str());
    ex->nodes[i-1].value = atof(inner_arr[2].c_str());    
  }

  return ex;
}


}  // namespace fm
