#include <tiramisu/tiramisu.h>
#include "wrapper_tutorial_11.h"

/*
 * This tutorial shows how to perform a distributed cvtcolor computation.
 * cvtcolor converts an RGB image into gray scale.
 * We assume that the data starts distributed by contiguous rows, so no communication is needed.
 * We hardcode this example to distribute across 10 different ranks (i.e. 10 unique processes). The user defines how
 * to map these ranks to physical machines using the configure.cmake file.
 * The cvtcolor computation is as follows:

for (int r = 0; r < ROWS; r++) {
  for (int c = 0; c < COLS; c++) {
  gray(r, c) = ((rgb(r, c, 0) * 4899 + rgb(r, c, 1) * 9617 + rgb(r, c, 2) * 1868) + 
               (1 << 13)) >> 13;
  }
}

 */

using namespace tiramisu;

int main(int argc, char **argv)
{
    // Set default tiramisu options.
    tiramisu::init();

    tiramisu::function cvtcolor("cvtcolor");

    // Define some constants and variables
    constant ROWS("ROWS", _ROWS, p_int32, true, NULL, 0, &cvtcolor);
    constant COLS("COLS", _COLS, p_int32, true, NULL, 0, &cvtcolor);
    var r("r"), c("c"), rr("rr"), rrr("rrr");

    // -------------------------------------------------------
    // Layer I
    // -------------------------------------------------------

    // The computations are defined across the entire compute space, not just what a single rank (i.e. node) computes.
    // Define a wrapper around the input
    computation input("[ROWS,COLS]->{input[r,c,chan]: 0<=r<ROWS and 0<=c<COLS and 0<=chan<3}", expr(), false, p_uint32, &cvtcolor);

    // Define the cvt color computation.
    expr convert = input(r, c, 0) * (uint32_t)4899 + input(r, c, 1) * (uint32_t)9617 + input(r, c, 2) * (uint32_t)1868;
    expr shift = (convert + ((uint32_t)1 << (uint32_t)13)) >> (uint32_t)13;
    computation rgb2gray("[ROWS,COLS]->{rgb2gray[r,c]: 0<=r<ROWS and 0<=c<COLS}", shift, true, p_uint32, &cvtcolor);

    // -------------------------------------------------------
    // Layer II
    // -------------------------------------------------------

    // Prepare the computations for distributing by splitting to create an outer loop over the 
    // number of nodes
    input.split(r, _ROWS/10, rr, rrr);
    rgb2gray.split(r, _ROWS/10, rr, rrr);

    // Tag the outer loop level over the number of nodes so that it is distributed. Internally,
    // this creates a new Var called "rank"
    input.tag_distribute_level(rr);
    rgb2gray.tag_distribute_level(rr);

    // Tell the code generator to not include the "rank" var when computing linearized indices (where the rank var is the tagged loop)
    input.drop_rank_iter(rr);
    rgb2gray.drop_rank_iter(rr);
    
    // -------------------------------------------------------
    // Layer III
    // -------------------------------------------------------

    // Create buffers for each node (the sizes should be relative to each node, so input is NUM_ROWS/10 x COLS x 3 and output is NUM_ROWS/10 X COLS)
    buffer input_buff("input_buff", {_ROWS/10, _COLS, 3}, p_uint32, a_input, &cvtcolor);
    buffer output_buff("output_buff", {_ROWS/10, _COLS}, p_uint32, a_output, &cvtcolor);

    // Define the mapping to the buffers (also relative to each node)
    input.set_access("{input[r,c,chan]->input_buff[r%" + std::to_string(_ROWS/10) + ",c,chan]}");
    rgb2gray.set_access("{rgb2gray[r,c]->output_buff[r%" + std::to_string(_ROWS/10) + ",c]}");

    cvtcolor.codegen({&input_buff, &output_buff}, "build/generated_fct_developers_tutorial_11.o");
    
    return 0;
}

