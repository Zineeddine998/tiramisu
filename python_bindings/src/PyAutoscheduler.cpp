#include "PyAutoscheduler.h"
#include "../../include/tiramisu/auto_scheduler/auto_scheduler.h"

namespace tiramisu
{
    namespace PythonBindings
    {

        void define_autoscheduler(py::module &m)
        {
            m.def("create_and_run_autoscheduler",
                  py::overload_cast<const std::vector<tiramisu::buffer *> &, const std::vector<halide_buffer_t *> &, std::function<int(halide_buffer_t *, halide_buffer_t *)>, const std::string, const std::string, int, int, tiramisu::function &>(&tiramisu::auto_scheduler::auto_scheduler::create_and_run_auto_scheduler),
                  "This function create and runs the autoscheduling process and print the results to a json file",
                  py::arg("arguments"), py::arg("func_arguments"), py::arg("func"), py::arg("obj_filename"), py::arg("json_filename"), py::arg("beam_size"), py::arg("max_depth"), py::arg("fct"));
        }

    } // namespace PythonBindings
} // namespace tiramisu
