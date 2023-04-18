#include "PyAutoscheduler.h"
#include "../../include/tiramisu/auto_scheduler/auto_scheduler.h"
#include "../../include/tiramisu/auto_scheduler/search_method.h"
#include "../../include/tiramisu/auto_scheduler/evaluator.h"
#include <vector>
#include "HalideBuffer.h"
#include "Python.h"
#include "numpy/arrayobject.h"

namespace tiramisu
{
    namespace PythonBindings
    {
        void define_autoscheduler(py::module &m)
        {
            m.def("create_and_run_auto_scheduler", [](std::vector<tiramisu::buffer *> const &arguments, PyObject *np_array, std::string const &func_name, std::string const &obj_filename, std::string const &json_filename, int beam_size, int max_depth, tiramisu::function *fct) -> void
                  {
        std::vector<halide_buffer_t *> func_arguments;
        
        // Convert the numpy array to a halide_buffer_t object
        int ndims = PyArray_NDIM((PyArrayObject *) np_array);
        npy_intp *shape = PyArray_SHAPE((PyArrayObject *) np_array);
        char *data_ptr = (char *) PyArray_DATA((PyArrayObject *) np_array);
        npy_intp *strides = PyArray_STRIDES((PyArrayObject *) np_array);

        halide_type_t dtype = halide_type_of<decltype(np_array[0])>();
        halide_buffer_t buf(data_ptr, shape[ndims-1], shape[ndims-1], ndims, halide_type_t(), nullptr);
        for (int i = 0; i < ndims-1; ++i) {
            buf.dim(i).set_bounds(0, shape[i]);
            buf.dim(i).set_stride(strides[i] / dtype.bytes());
        }
        
        // Append the halide_buffer_t object to the vector
        func_arguments.push_back(&buf);
        
        auto_scheduler::auto_scheduler::create_and_run_auto_scheduler(arguments, func_arguments, func_name, obj_filename, json_filename, beam_size, max_depth, fct); });
        }
    } // namespace PythonBindingsj
} // namespace tiramisu
