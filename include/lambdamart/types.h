#ifndef LAMBDAMART_TYPES_H
#define LAMBDAMART_TYPES_H

#include <cstdint>

namespace LambdaMART {
    typedef uint32_t sample_t;  // this is the former datasize_t, 32bit is enough
    typedef uint32_t feature_t;
    typedef uint8_t  bin_t;
    typedef double   label_t;
    typedef uint32_t node_t;
    typedef double   gradient_t;
}

#endif //LAMBDAMART_TYPES_H
