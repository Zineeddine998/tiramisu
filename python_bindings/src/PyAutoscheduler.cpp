#include "PyAutoscheduler.h"
#include "../../include/tiramisu/auto_scheduler/auto_scheduler.h"
#include "../../include/tiramisu/auto_scheduler/search_method.h"
#include "../../include/tiramisu/auto_scheduler/evaluator.h"
#include <vector>
#include "Halide.h"
// #include "Python.h"
// #include "numpy/arrayobject.h"
#include <pybind11/numpy.h>

namespace tiramisu
{
    namespace PythonBindings
    {
        void define_autoscheduler(py::module &m)
        {
            m.def("create_and_run_auto_scheduler", [](std::vector<tiramisu::buffer *> const &arguments, std::vector<py::buffer> py_func_arguments, std::string const &func_name, std::string const &obj_filename, std::string const &json_filename, int beam_size, int max_depth, tiramisu::function *fct) -> void
                  // m.def("create_and_run_auto_scheduler", [](std::vector<tiramisu::buffer *> const &arguments, std::string const &func_name, std::string const &obj_filename, std::string const &json_filename, int beam_size, int max_depth, tiramisu::function *fct) -> void
            {
                    std::vector<halide_buffer_t *> func_arguments;

                    for(py::buffer py_buf: py_func_arguments){
                        halide_dimension_t *halide_dim;
                        Halide::Buffer halide_buf;

                        for (i = 0; i < py_buf.ndim; ++i, j += j_step) {
                            halide_dim[i].min = 0;
                            halide_dim[i].stride = (int)(py_buf.strides[j] / py_buf.itemsize);  // strides is in bytes
                            halide_dim[i].extent = (int)py_buf.shape[j];
                            halide_dim[i].flags = 0;
                        }
                        
                        memset(&halide_buf, 0, sizeof(halide_buf));

                        if (!py_buf.format) {
                            halide_buf.type.code = halide_type_uint;
                            halide_buf.type.bits = 8;
                        } else {
                            /* Convert struct type code. See
                            * https://docs.python.org/2/library/struct.html#module-struct */
                            char *p = py_buf.format;
                            while (strchr("@<>!=", *p)) {
                                p++;  // ignore little/bit endian (and alignment)
                            }
                            if (*p == 'f' || *p == 'd' || *p == 'e') {
                                // 'f', 'd', and 'e' are float, double, and half, respectively.
                                halide_buf.type.code = halide_type_float;
                            } else if (*p >= 'a' && *p <= 'z') {
                                // lowercase is signed int.
                                halide_buf.type.code = halide_type_int;
                            } else {
                                // uppercase is unsigned int.
                                halide_buf.type.code = halide_type_uint;
                            }
                            const char *type_codes = "bBhHiIlLqQfde";  // integers and floats
                            if (*p == '?') {
                                // Special-case bool, so that it is a distinct type vs uint8_t
                                // (even though the memory layout is identical)
                                halide_buf.type.bits = 1;
                            } else if (strchr(type_codes, *p)) {
                                halide_buf.type.bits = (uint8_t)py_buf.itemsize * 8;
                            }
                        }
                        halide_buf.type.lanes = 1;
                        halide_buf.dimensions = py_buf.ndim;
                        halide_buf.dim = halide_dim;
                        halide_buf.host = (uint8_t *)py_buf.buf;

                        func_arguments.push_back(halide_buf.raw_buffer());
                    }

                    auto_scheduler::auto_scheduler::create_and_run_auto_scheduler(arguments, func_arguments, func_name, obj_filename, json_filename, beam_size, max_depth, fct); });
        }
    } // namespace PythonBindings
} // namespace tiramisu