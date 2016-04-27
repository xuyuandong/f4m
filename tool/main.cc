// Author: yuandong1@staff.sina.com.cn

#include <string>
#include "gflags/gflags.h"
#include "encoder.h"

using namespace fm;
using namespace std;
using namespace google;

DEFINE_string(dict, "", "code dict path");
DEFINE_string(filter, "", "filter fields setting");
DEFINE_string(output, "", "output encoded training file");
DEFINE_int32(threshold, 5, "filter too low frequency features");
DEFINE_int32(bypassv, 1, "bypass value for short encoded output");

int main(int argc, char* argv[]) {
  ParseCommandLineFlags(&argc, &argv, true);
  
  Option opt;
  opt.filter = FLAGS_filter;
  opt.dict_path = FLAGS_dict;
  opt.output_path = FLAGS_output;
  opt.threshold = FLAGS_threshold;
  opt.bypass_value = FLAGS_bypassv;

  Encoder encoder;
  encoder.Init(opt);
  encoder.Process(opt);

  fprintf(stdout, "%ld %ld %.5f\n",
    encoder.GetPv(), encoder.GetClick(), encoder.GetCtr());
  return 0;
}
