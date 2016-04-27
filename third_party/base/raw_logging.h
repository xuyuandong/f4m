#ifndef BASE_RAW_LOGGING_H_
#define BASE_RAW_LOGGING_H_

#include "base/logging.h"

namespace logging {

#ifndef NDEBUG

#define RAW_DLOG(level, message) RAW_LOG(level, message)

#else

#define RAW_DLOG(level, message)                          \
  while (false)                                           \
    RAW_LOG(level,message)
#endif

}

#endif
