#include "Halide.h"
#include <tiramisu/utils.h>
#include <cstdlib>
#include <iostream>

#include "wrapper_test_21.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t my_external(halide_buffer_t *buf0)
{
    return buf0->host[0];
}

#ifdef __cplusplus
}  // extern "C"
#endif

int main(int, char **)
{
    Halide::Buffer<uint8_t> reference_buf(SIZE0, SIZE1);
    init_buffer(reference_buf, (uint8_t)9);

    Halide::Buffer<uint8_t> output_buf1(SIZE0, SIZE1);
    Halide::Buffer<uint8_t> output_buf2(SIZE0, SIZE1);
    init_buffer(output_buf1, (uint8_t) 9);
    init_buffer(output_buf2, (uint8_t) 0);

    // Call the Tiramisu generated code
    tiramisu_generated_code(output_buf1.raw_buffer(), output_buf2.raw_buffer());

    compare_buffers(std::string(TEST_NAME_STR), output_buf1, reference_buf);

    return 0;
}
