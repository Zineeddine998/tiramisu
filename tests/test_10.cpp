#include <isl/set.h>
#include <isl/union_map.h>
#include <isl/union_set.h>
#include <isl/ast_build.h>
#include <isl/schedule.h>
#include <isl/schedule_node.h>

#include <tiramisu/debug.h>
#include <tiramisu/core.h>

#include <string.h>
#include <Halide.h>

using namespace tiramisu;

void generate_function_1(std::string name, int size, int val0)
{
    tiramisu::global::set_default_tiramisu_options();


    tiramisu::function function0(name);
    int Msize = 2 * floor(size / 2);
    tiramisu::constant N("N", tiramisu::expr((int32_t) size), p_int32, true, NULL, 0, &function0);
    tiramisu::constant M("M", tiramisu::expr((int32_t) Msize), p_int32, true, NULL, 0, &function0);
    tiramisu::var i("i"), j("j");

    tiramisu::buffer buf0("buf0", {size, size}, tiramisu::p_uint8, a_output, &function0);

    tiramisu::expr e1 = tiramisu::expr((uint8_t) val0);
    tiramisu::computation S0("[N,M]->{S0[i,j]: 0<=i<N and 0<=j<N}", e1, true, p_uint8, &function0);
    S0.set_access("[N,M]->{S0[i,j]->buf0[i,j]: 0<=i<N and 0<=j<N}");
    S0.vectorize(i, 4);
 
    function0.set_arguments({&buf0});
    function0.gen_time_space_domain();
    function0.gen_isl_ast();
    function0.gen_halide_stmt();
    function0.gen_halide_obj("build/generated_fct_test_10.o");
}

int main(int argc, char **argv)
{
    generate_function_1("assign_7_to_10x10_2D_array_with_vectorization", 32, 7);

    return 0;
}
