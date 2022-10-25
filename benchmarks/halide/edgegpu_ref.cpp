#include "Halide.h"

using namespace Halide;

// !! This is a placeholder for Halide implementation
// It's just a copy of CPU version. The output should be ignored.

int main(int argc, char* argv[])
{
    ImageParam Img(UInt(8), 3, "Img");
    Func R("R"), Out("Out");
    Var i("i"), j("j"), c("c"), i1("i1"), j1("j1"), i2("i2"), j2("j2");

    /* Ring blur filter. */
    R(i, j, c) = (Img(i,   j, c) + Img(i,   j+1, c) + Img(i,   j+2, c)+
		  Img(i+1, j, c)                    + Img(i+1, j+2, c)+
		  Img(i+2, j, c) + Img(i+2, j+1, c) + Img(i+2, j+2,
		      c))/((uint8_t) 8);

    /* Robert's edge detection filter. */
    Out(i, j, c) = (R(i+1, j+1, c)-R(i+2, j, c)) + (R(i+2, j+1, c)-R(i+1, j, c));

    R.compute_root();
    Out.compute_root();

    Out.compile_to_object("build/generated_fct_edgegpu_ref.o", {Img}, "edge_ref");
    Out.compile_to_lowered_stmt("build/generated_fct_edgegpu_ref.txt", {Img}, Text);

  return 0;
}

