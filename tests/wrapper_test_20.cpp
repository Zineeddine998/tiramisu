#include "Halide.h"
#include <tiramisu/utils.h>
#include <cstdlib>
#include <iostream>

#include "wrapper_test_20.h"

int main(int, char **)
{
    Halide::Buffer<uint8_t> reference_buf(SIZE0, SIZE1);
    init_buffer(reference_buf, (uint8_t)4);

    Halide::Buffer<uint8_t> output_buf(SIZE0, SIZE1);
    init_buffer(output_buf, (uint8_t)99);

    // Call the Tiramisu generated code
    TEST_NAME(output_buf.raw_buffer());

    compare_buffers(std::string(TEST_NAME_STR), output_buf, reference_buf);

    return 0;
}
