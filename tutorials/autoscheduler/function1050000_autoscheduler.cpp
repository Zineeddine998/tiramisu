#include "Halide.h"
#include <tiramisu/tiramisu.h>
#include <tiramisu/auto_scheduler/evaluator.h>
#include <tiramisu/auto_scheduler/search_method.h>
#include "function1050000_wrapper.h"

using namespace tiramisu;

int main(int argc, char **argv)
{

	double *c_buf00 = (double *)malloc(64 * 769 * 384 * sizeof(double));
	parallel_init_buffer(c_buf00, 64 * 769 * 384, (double)44);
	Halide::Buffer<double> h_buf00(c_buf00, 64, 769, 384);

	double *c_buf01 = (double *)malloc(770 * sizeof(double));
	parallel_init_buffer(c_buf01, 770, (double)21);
	Halide::Buffer<double> h_buf01(c_buf01, 770);

	tiramisu::init("function1050000");
	var i0("i0"), i1("i1"), i2("i2"), i3("i3"), i2_p0("i2_p0"), i2_p1("i2_p1");
	input icomp00("icomp00", {i1, i2_p0, i3}, p_float64);
	input input01("input01", {i2_p1}, p_float64);
	computation comp00("{comp00[i0,i1,i2,i3]: 0<=i0<64 and 1<=i1<384 and 1<=i2<769 and 0<=i3<32}", expr(), true, p_float64, global::get_implicit_function());
	comp00.set_expression(icomp00(i1, i2, i3) * input01(i2 - 1) / input01(i2 + 1) + input01(i2));
	computation comp01("{comp01[i0,i1,i2,i3]: 0<=i0<64 and 1<=i1<384 and 1<=i2<769 and 0<=i3<32}", icomp00(i1, i2, i3) - input01(i2), true, p_float64, global::get_implicit_function());
	comp00.then(comp01, i3);
	buffer buf00("buf00", {384, 769, 64}, p_float64, a_output);
	buffer buf01("buf01", {770}, p_float64, a_input);
	icomp00.store_in(&buf00);
	input01.store_in(&buf01);
	comp00.store_in(&buf00, {i1, i2, i3});
	comp01.store_in(&buf00, {i1, i2, i3});

	// function1050000(h_buf00.raw_buffer(), h_buf01.raw_buffer());
	auto_scheduler::auto_scheduler::create_and_run_auto_scheduler({&buf00, &buf01}, {h_buf00.raw_buffer(), h_buf01.raw_buffer()}, &function1050000, "function1050000.o", "./function1050000_explored_schedules.json", 1, 2);

	return 0;
}
