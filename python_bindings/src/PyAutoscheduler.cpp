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
            m.def("create_and_run_auto_scheduler", [](std::vector<tiramisu::buffer *> const &arguments, std::string const &func_name, std::string const &obj_filename, std::string const &json_filename, int beam_size, int max_depth, tiramisu::function *fct) -> void
                  // m.def("create_and_run_auto_scheduler", [](std::vector<tiramisu::buffer *> const &arguments, std::string const &func_name, std::string const &obj_filename, std::string const &json_filename, int beam_size, int max_depth, tiramisu::function *fct) -> void
                  {
                    
                      std::vector<halide_buffer_t *> func_arguments;
                      Halide::Buffer<float> buf_1(16, "buf_1");
                      Halide::Buffer<float> buf_2(16, "buf_2");

                       for (int z = 0; z < buf_1.channels(); z++)
                        {
                            for (int y = 0; y < buf_1.height(); y++)
                            {
                                for (int x = 0; x < buf_1.width(); x++)
                                {
                                    buf_1(x, y, z) = (float)0;
                                }
                            }
                        }

                        for (int z = 0; z < buf_2.channels(); z++)
                        {
                            for (int y = 0; y < buf_2.height(); y++)
                            {
                                for (int x = 0; x < buf_2.width(); x++)
                                {
                                    buf_2(x, y, z) = (float)0;
                                }
                            }
                        }

                      func_arguments.push_back(buf_1.raw_buffer());
                      func_arguments.push_back(buf_2.raw_buffer());

                      // // Convert the numpy array to a halide_buffer_t object
                      // // int ndims = PyArray_NDIM((PyArrayObject *) np_array);
                      // // npy_intp *shape = PyArray_SHAPE((PyArrayObject *) np_array);
                      // // char *data_ptr = (char *) PyArray_DATA((PyArrayObject *) np_array);
                      // // npy_intp *strides = PyArray_STRIDES((PyArrayObject *) np_array);

                      // std::vector<Halide::Buffer<double>> buffers;
                      // for (int i = 0; i < PyList_Size(np_array); ++i) {
                      //         PyObject *np_array_item = PyList_GetItem(np_array, i);
                      //         int ndims = PyArray_NDIM((PyArrayObject *) np_array_item);
                      //         npy_intp *shape = PyArray_SHAPE((PyArrayObject *) np_array_item);
                      //         char *data_ptr = (char *) PyArray_DATA((PyArrayObject *) np_array_item);
                      //         npy_intp *strides = PyArray_STRIDES((PyArrayObject *) np_array_item);

                      //         std::vector<int> dimensions(shape, shape + ndims);
                      //         Halide::Buffer<double> buffer(nullptr, dimensions);
                      //         buffer.raw_buffer()->host = reinterpret_cast<uint8_t*>(data_ptr);

                      //         for (int j = 0; j < ndims; ++j) {
                      //             buffer.raw_buffer()->dim[j].stride = strides[j] / sizeof(double);
                      //         }
                      //         buffers.push_back(buffer);
                      // }

                      // for (auto &buffer : buffers) {
                      //     func_arguments.push_back(buffer.raw_buffer());
                      // }

                    //   auto data = np_array.mutable_unchecked<1>();

                      // Loop over the elements of the NumPy array and print them
                    //   for (ssize_t i = 0; i < data.shape(0); i++)
                    //   {
                    //       std::cout << data(i) << std::endl;
                    //   }

                      std::cout << "\nIt works !!";
                      auto_scheduler::auto_scheduler::create_and_run_auto_scheduler(arguments, func_arguments, func_name, obj_filename, json_filename, beam_size, max_depth, fct); });
        }
    } // namespace PythonBindings
} // namespace tiramisu