// Author: yuandong1@staff.sina.com.cn
//
#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <cstdio>
#include <string>
#include <random>
#include <unordered_map>

namespace fm {

struct Option {
  int threshold;
  int bypass_value;
  std::string filter;
  std::string dict_path;
  std::string output_path;
};

inline void SplitString(const std::string& str,
    const char s, std::vector<std::string>* r) {
  size_t last = 0;
  size_t c = str.size();
  for (size_t i = 0; i <= c; ++i) {
    if (i == c || str[i] == s) {
      size_t len = i - last;
      std::string tmp = str.substr(last, len);
      r->push_back(tmp);
      last = i + 1;
    }
  }
}
  
class Encoder {
 public:
  Encoder() : pv_(0), click_(0) {}
  ~Encoder() {}

  void Init(const Option& option);
  void Process(const Option& option);

  int64_t GetPv() { return pv_; }
  int64_t GetClick() { return click_; }
  float GetCtr() { return click_ / (pv_ + 0.01f); }

 private:
  int ParseAndTransform(FILE* fw, char* buf);

 private:
  int64_t pv_;
  int64_t click_;
  bool bypassv_;
  std::unordered_map<std::string, std::string> dict_;
  std::unordered_map<std::string, int> filter_fields_; 
};

}  // namespace fm

#endif  // _ENCODER_H_
