// AUthor: yuandong1@staff.sina.com.cn
//
#include "encoder.h"
#include <cstdio>
#include <iostream>
#include <vector>

namespace fm {
 
const int MAX_BUF_SIZE = 1<<24;
const int MAX_LINE_SIZE = 100000;
const char CTRL_A = '';
const char CTRL_B = '';

void Encoder::Init(const Option& option) {
  FILE* fd = fopen(option.dict_path.c_str(), "r");
  if (fd == NULL) {
    fprintf(stderr, "Bad encoding dict file\n");
    exit(1);
  }
 
  int no = 0;
  char buf[MAX_LINE_SIZE];
  while (fgets(buf, MAX_LINE_SIZE, fd)) {
    char* p = buf;
    for (; p != '\0'; ++p) {
      if (*p == CTRL_A) {
        *p = '\0';
        break;
      }
    }

    int pv = atoi(p+1);
    if (pv < option.threshold)
      break;
    
    char arr[16];
    snprintf(arr, 16, "%d", ++no);
    dict_[buf] = arr;
    //std::string item(buf);
    //std::cout << item << " (" << pv << ")" << " -> " << no << std::endl;
  }

  fclose(fd);

  // add filter
  if (option.filter.size() > 0) {
    std::vector<std::string> vec;
    SplitString(option.filter, ',', &vec);
    for (size_t i = 0; i < vec.size(); ++i) {
      filter_fields_[vec[i]] = 1;
    }
  }

  bypassv_ = option.bypass_value;
}

void Encoder::Process(const Option& option) {
  FILE* fw = fopen(option.output_path.c_str(), "w");
  if (fw == NULL) {
    fprintf(stderr, "Bad output file\n");
    exit(1);
  }

  int err_count = 0;
  char buf[MAX_LINE_SIZE];
  while (fgets(buf, MAX_LINE_SIZE, stdin)) {
    if (EOF == ParseAndTransform(fw, buf)) {
      fprintf(stderr, "ERROR when parse or output\n");
      if (++err_count > 10) {
        exit(1);
      }
    }
  }

  fclose(fw);
}

int Encoder::ParseAndTransform(FILE* fw, char* buf) {
  std::string label;
  std::vector<std::string> ex;

  // parse
  char* p = buf;
  char* last = buf;
  for (; *p != '\0'; ++p) {
    if (*p == CTRL_A) {
      *p = '\0';
      label = last;
      //fprintf(stdout, "label %s\n", last);
      last = p + 1;
      *p = CTRL_A;  //

    } else if (*p == CTRL_B || *p == '\n') {
      *p = '\0';
      ex.push_back(last);
      //fprintf(stdout, "field %s\n", last);
      last = p + 1;
      *p = CTRL_B;

    } else {
      // pass
    }
  } // end for
  p[-1] = '\n';

  // statistic
  int pn = atoi(label.c_str());
  if (pn > 0) {
    click_ += 1;
  }
  pv_ += 1;

  // transform
  for (size_t i = 0; i < ex.size(); ++i) {
    const std::string& node = ex[i];
    auto iter = dict_.find(node);
    if (iter == dict_.end()) {
      //fprintf(stderr, "error missing feature: [%s]\n", node.c_str());
      continue;
    }
    
    size_t p = 0;
    for (; p < node.size(); ++p) {
      if (node[p] == '-')
        break;
    }
    if (p == 0) {
      fprintf(stderr, "error format: [%s], line:%s\n", node.c_str(), buf);
      return EOF;
    }

    const std::string& field = node.substr(0, p);
    if (filter_fields_.find(field) != filter_fields_.end()) {
      continue;
    }

    if (bypassv_) {
      label += " " + field + ":" + iter->second;
    } else {
      label += " " + field + ":" + iter->second + ":1";
    }
  }

  if (label.size() < 4) {
    fprintf(stderr, "exception sample, line:%s\n", buf);
    return EOF;
  }

  // write
  //label += "\n";
  //return fputs(label.c_str(), fw);
  return fprintf(fw, "%s\n", label.c_str());
}


}  // namespace fm
