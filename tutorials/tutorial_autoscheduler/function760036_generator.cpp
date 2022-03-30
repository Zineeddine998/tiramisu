#include <tiramisu/tiramisu.h> 
#include <tiramisu/auto_scheduler/evaluator.h>
#include <tiramisu/auto_scheduler/search_method.h>
#include "function760036_wrapper.h"

using namespace tiramisu;

int main(int argc, char **argv){                
	tiramisu::init("function760036");
	var i0("i0", 1, 65), i1("i1", 0, 32), i2("i2", 0, 32), i3("i3", 0, 64), i0_p1("i0_p1", 0, 66);
	input icomp00("icomp00", {i0_p1,i1,i2}, p_float64);
	computation comp00("comp00", {i0,i1,i2,i3},  p_float64);
	comp00.set_expression(icomp00(i0, i1, i2) + icomp00(i0 - 1, i1, i2)*icomp00(i0 + 1, i1, i2));
	computation comp01("comp01", {i0,i1,i2,i3}, 2.470);
	comp00.then(comp01, i3);
	buffer buf00("buf00", {66,32,32}, p_float64, a_output);
	buffer buf01("buf01", {65,32,32,64}, p_float64, a_output);
	icomp00.store_in(&buf00);
	comp00.store_in(&buf00, {i0,i1,i2});
	comp01.store_in(&buf01);
	tiramisu::codegen({&buf00,&buf01}, "function760036.o"); 
	return 0; 
}