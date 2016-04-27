// AUthor: yuandong1@staff.sina.com.cn
//
#include "parser.h"
#include <unistd.h>
#include "util.hpp"
#include <iostream>

namespace fm {
  
const int MAX_BUF_SIZE = 1<<24;

Parser::Parser() : ftr_(NULL), fva_(NULL), fpd_(NULL), fch_(NULL),
    low_freq_filter_threshold_(0), negsampling_(1.f),  
    e_(rd_()), u_(0.f, 1.f) {
  fbuf_ = new char[MAX_BUF_SIZE];
  fbuf_va_ = new char[MAX_BUF_SIZE];
  fbuf_ch_ = new char[MAX_BUF_SIZE];
}

Parser::~Parser() {
  if (ftr_ != NULL)
    fclose(ftr_);
  if (fva_ != NULL)
    fclose(fva_);
  if (fpd_ != NULL)
    fclose(fpd_);
  if (fch_ != NULL)
    fclose(fch_);
  delete fbuf_; fbuf_ = NULL;
  delete fbuf_va_; fbuf_va_ = NULL;
  delete fbuf_ch_; fbuf_ch_ = NULL;
}

void Parser::Init(const Parameter& params) {
  std::cout << "====================================>" <<std::endl;
  if (!params.mode) {  //training
    if (params.train_path == "/dev/stdin") {
      fprintf(stdout, "train from stdin.\n");
      ftr_ = stdin;

      if (params.iterations > 1) {
        cache_file_ = params.cache_path;
        fch_ = fopen(cache_file_.c_str(), "w");
        if (fch_ == NULL) {
          fprintf(stderr, "Bad cache file\n");
          exit(1);
        }
        if (0 != setvbuf(fch_, fbuf_ch_, _IOFBF, MAX_BUF_SIZE)) {
          fprintf(stderr, "Failed to set buffer for FILE IO\n");
        }
        fprintf(stdout, "set input cache file: %s\n", cache_file_.c_str());
      }

    } else {
      ftr_ = fopen(params.train_path.c_str(), "r");
      if (ftr_ == NULL) {
        fprintf(stderr, "Bad training file\n");
        exit(1);
      }
      if (0 != setvbuf(ftr_, fbuf_, _IOFBF, MAX_BUF_SIZE)) {
        fprintf(stderr, "Failed to set buffer for FILE IO\n");
      }
      fprintf(stdout, "train from %s.\n", params.train_path.c_str());
    }
  }

  if (!params.test_path.empty()) {  // training or testing
    fva_ = fopen(params.test_path.c_str(), "r");
    if (fva_ == NULL) {
      fprintf(stderr, "Bad testing file\n");
      exit(1);
    }
    if (0 != setvbuf(fva_, fbuf_va_, _IOFBF, MAX_BUF_SIZE)) {
      fprintf(stderr, "Failed to set buffer for FILE IO\n");
    }
    fprintf(stdout, "test from %s.\n", params.test_path.c_str());

    if (params.mode && !params.pred_path.empty()) {
      fpd_ = fopen(params.pred_path.c_str(), "w");
      if (fpd_ == NULL) {
        fprintf(stderr, "Bad predict file\n");
        exit(1);
      }
      fprintf(stdout, "set output predict file: %s\n", params.pred_path.c_str());
    }
  }

  // pre-process feature
  nspace_ = 1 << params.nbit;
  fprintf(stdout, "feature hash space: %d\n", nspace_);

  if (!params.mode) {
    low_freq_filter_threshold_ = params.low_freq_filter;
    negsampling_ = params.negsampling;
    if (negsampling_ < 1.f) {
      fprintf(stdout, "Use neg sampling, rate=%.2f\n", negsampling_);
    }
  }
  std::cout << "====================================>" <<std::endl;
}

void Parser::PreRead(const Parameter& params) {
  if (low_freq_filter_threshold_ > 0) {
    //int tmp_negsampling = negsampling_;
    //negsampling_ = 1.f;
    int tmp_threshold = low_freq_filter_threshold_;
    low_freq_filter_threshold_ = 0;
    fprintf(stdout, "Use low frequency feature filter, after neg sampling, T=%d\n", tmp_threshold);
    
    Example* ex = NULL;
    counter_.resize(nspace_, 0);
    while (NULL != (ex = GetExample())) {
      for (size_t x = 0; x < ex->nodes.size(); ++x) {
        int i = ex->nodes[x].index;
        ++counter_[i];
      }
      delete ex;
      ex = NULL;
    }

    low_freq_filter_threshold_ = tmp_threshold;
    //negsampling_ = tmp_negsampling;
  }
}
  
void Parser::SetTrainFileToCache() {
  if (ftr_ != NULL) {
    fclose(ftr_);
    ftr_ = NULL;
  }

  sleep(1);
  negsampling_ = 1.f;  // make sure

  ftr_ = fopen(cache_file_.c_str(), "r");
  if (ftr_ == NULL) {
    fprintf(stderr, "Bad training file\n");
    exit(1);
  }
  if (0 != setvbuf(ftr_, fbuf_, _IOFBF, MAX_BUF_SIZE)) {
    fprintf(stderr, "Failed to set buffer for FILE IO\n");
  }
}

Example* Parser::ParseLine(FILE* fp, bool train) {
  const int MAX_LINE_SIZE = 4096;
  char buf[MAX_LINE_SIZE];
 
  Example* ex = ParseLineImpl(fp, buf, MAX_LINE_SIZE);
  if (train) {
    if (ex == NULL) {  // end of read
      if (fch_ != NULL) {  // from stdin
        fclose(fch_);
        fch_ = NULL;
        SetTrainFileToCache();

      } else {  // from file
        rewind(fp); //fseek(fp, 0, SEEK_SET);
      }

    } else {  // read a line
      if (fch_ != NULL) {
        fprintf(fch_, "%s", buf);
      }
    }
  } else {  // test file
    if (ex == NULL && fp != NULL) {
      rewind(fp); //fseek(fp, 0, SEEK_SET);
    } 
  }
  return ex;
}

Example* Parser::ParseLineImpl(FILE* fp, char* buf, int buf_size) {
  if (!fp || !fgets(buf, buf_size, fp) || feof(fp)) {
    //fprintf(stdout, "end of file\n");
    return NULL;
  }
  
  Node node;
  int label;
  Example* ex = NULL;

  int state = 0;
  char* p = buf;
  char* last = buf;
  for (; *p != '\0'; ++p) {
    if (*p == ' ' || *p == '\n') {
      *p = '\0';
      if (state == 0) {
        label = ((atoi(buf) + 1) >> 1);
        //fprintf(stdout, "label %s\n", buf);
        if (label < 1 && negsampling_ < 1.f) {
          if (u_(e_) > negsampling_) {
            return ParseLineImpl(fp, buf, buf_size);
          }
        }
        ex = new Example();
        ex->label = label;

      } else {
        if (state == 2) {
          node.index = (atoi(last) & (nspace_ - 1));
          //fprintf(stdout, "index %s\n", last);
        } else {
          node.value = atof(last);
          //fprintf(stdout, "value %s\n", last);
        }
        if (0 == low_freq_filter_threshold_ 
            || counter_[node.index] > low_freq_filter_threshold_) {
          ex->nodes.push_back(node);
        }
      }
      last = p + 1;
      state = 1;
      *p = ' ';  //
    } else if (*p == ':') {
      *p = '\0';
      if (state == 1) {
        node.field = atoi(last);
        //fprintf(stdout, "field %s\n", last);
      } else if (state == 2) {
        node.index = (atoi(last) & (nspace_ - 1));
        //fprintf(stdout, "index %s\n", last);
      } else {
        fprintf(stderr, "error format: %s at %ld\n", buf, p - buf);
        delete ex;
        ex = NULL;
        break;
      }
      last = p + 1;
      state += 1;
      *p = ':';  //
    }
  }
  p[-1] = '\n';

  return ex;
}


}  // namespace fm
