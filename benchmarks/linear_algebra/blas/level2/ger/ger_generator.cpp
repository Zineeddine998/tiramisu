/*
    This program shows how to process BLAS LEVEL2 ger
    for i = 0 .. M
        for j = 0 .. N
            C[i,j] = A[i,j] + X[i]*Y[j]*alpha
*/

#include <tiramisu/tiramisu.h>
#include "benchmarks.h"

using namespace tiramisu;

int main(int argc, char **argv)
{
    tiramisu::init("ger");
    
    // -------------------------------------------------------
    // Layer I
    // -------------------------------------------------------
    
    constant p0("M",M);
    constant p1("N",N);
    var i("i",0,p0), j("j",0,p1);
    // Declare inputs : A(Matrix M*N) , X(Vector dim=M) , Y(Vector dim=N)
    input A("A",{i,j},p_float64);
    input X("X",{i},p_float64);
    input Y("Y",{j},p_float64);
    input alpha("alpha",{},p_float64);
    // Declare output C which is result of computation.
    computation C("C",{i,j},cast(p_float64,A(i,j)+X(i)*Y(j)*alpha(0)));
    
    // -------------------------------------------------------
    // Layer II
    // -------------------------------------------------------
    
     //Tile computations: C
    // This tiles the loop levels i and j and produces the loop levels by a 32x32 tile.
    // i0, j0, i1 and j1 where i0 is the outermost loop level and j1 is the innermost.
    var i0("i0"),j0("j0"),i1("i1"),j1("j1");
    C.tile(i,j,32,32,i0,j0,i1,j1);
   // Parallelize the outermost loop level i0
    C.parallelize(i0);
    
    // -------------------------------------------------------
    // Layer III
    // -------------------------------------------------------
    
    // Declare the buffers.
    buffer b_A("b_A", {expr(M), expr(N)},p_float64,a_input);
    buffer b_X("b_X", {expr(M)},p_float64,a_input);
    buffer b_Y("b_Y", {expr(N)},p_float64,a_input);
    buffer b_alpha("b_alpha", {expr(1)},p_float64,a_input);
    buffer b_C("b_C", {expr(M),expr(N)},p_float64,a_output);
    
    // Map the computations to a buffer.
    A.store_in(&b_A);
    X.store_in(&b_X);
    Y.store_in(&b_Y);
    alpha.store_in(&b_alpha);
    C.store_in(&b_C,{i,j});
    
    // -------------------------------------------------------
    // Code Generation
    // -------------------------------------------------------
    
    tiramisu::codegen({&b_A, &b_X,&b_Y,&b_alpha, &b_C}, "generated_ger.o");
    return 0;
}

