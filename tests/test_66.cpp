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

#include "wrapper_test_66.h"

using namespace tiramisu;

/**
 * Test vectorization. 
 */

void generate_function(std::string name, int size, int val0)
{
    tiramisu::global::set_default_tiramisu_options();
    

    // -------------------------------------------------------
    // Layer I
    // -------------------------------------------------------

    tiramisu::function function0(name);
    tiramisu::constant N("N", tiramisu::expr((int32_t) size), p_int32, true, NULL, 0, &function0);
    tiramisu::var i("i");
    tiramisu::var j("j");
    tiramisu::computation S0("[N]->{S0[i,j]: 0<=i<N and 0<=j<N}", tiramisu::expr((uint8_t) val0), true, p_uint8, &function0);

    // -------------------------------------------------------
    // Layer II
    // -------------------------------------------------------

    S0.vectorize(j, 4);

    // -------------------------------------------------------
    // Layer III
    // -------------------------------------------------------

    function0.allocate_and_map_buffers_automatically();

    // -------------------------------------------------------
    // Code Generation
    // -------------------------------------------------------

    function0.set_arguments({S0.get_automatically_allocated_buffer()});
    function0.gen_time_space_domain();
    function0.gen_isl_ast();
    function0.gen_halide_stmt();
    function0.gen_halide_obj("build/generated_fct_test_" + std::string(TEST_NUMBER_STR) + ".o");
}

int main(int argc, char **argv)
{
    generate_function("tiramisu_generated_code", SIZE1, 5);

    return 0;
}
