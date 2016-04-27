// Author: yuadong1@staff.sina.com.cn
//
#ifndef _UTIL_H_
#define _UTIL_H_

#include <cmath>
#include <ctime>
#include <sys/time.h>
#include <string>
#include <vector>
#include <iostream>

namespace fm {

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

inline std::string IntToStr(int val) {
  const int kOutputBufSize = 3 * sizeof(int) + 1;
  std::string outbuf(kOutputBufSize, 0);

  bool is_neg = (val < 0);

  for (std::string::iterator it = outbuf.end();;) {
    --it;
    if (it == outbuf.begin()) {
      std::cerr << "ERROR: IntToStr out of buffer range." << std::endl;
      return "";
    } 
    
    *it = static_cast<char>((val % 10) + '0');
    val /= 10;
      
    if (val == 0) {
      if (is_neg) {
        --it;
        if (it == outbuf.begin()) {
          std::cerr << "ERROR: IntToStr out of buffer range." << std::endl;
          return "";
        } 
        *it = static_cast<char>('-');
      }
      return std::string(it, outbuf.end());
    }
  }
}

inline float sign(float x) {
  if (x < 0.0f) return -1.f;
  else return 1.f;
}

inline float LogLoss(float y, float p) {
  if (y > 0)
    return -std::log(p);
  return -std::log(1.f - p);
}

inline int64_t GetTimeInMs() {
  struct timeval now;
  gettimeofday(&now, NULL);
  return static_cast<int64_t>(now.tv_sec * 1000 + now.tv_usec / 1000);
}

}  // namespace fm

#endif  // _UTIL_H_
