#include <tiramisu/tiramisu.h>
#include <tiramisu/auto_scheduler/evaluator.h>
#include <tiramisu/auto_scheduler/search_method.h>
#include "nussinov_wrapper.h"

using namespace tiramisu;

const std::string py_cmd_path = "/usr/bin/python";
const std::string py_interface_path = "/home/zinou/tiramisu/tiramisu/tutorials/tutorial_autoscheduler/model/main.py";

int main(int argc, char **argv)
{
    tiramisu::init("nussinov");

    // -------------------------------------------------------
    // Layer I
    // -------------------------------------------------------
    constant NN("NN", 60);

    // Iteration variables
    var i("i", 0, 60), j("j", 0, 60), k("k");

    // inputs
    input table("table", {i, j}, p_float64);
    input seq("seq", {i}, p_float64);

    // Computations
    computation table_1("[NN]->{table_1[i,j]: 0<=i<NN and NN-i<=j<NN and 1<=j}", expr(), true, p_float64, global::get_implicit_function());
    table_1.set_expression(expr(o_max, table(60 - 1 - i, j), table(60 - 1 - i, j - 1)));
    computation table_2("[NN]->{table_2[i,j]: 0<=i<NN and NN-i<=j<NN and 0<i}", expr(), true, p_float64, global::get_implicit_function());
    table_2.set_expression(expr(o_max, table(60 - 1 - i, j), table(60 - 1 - i + 1, j)));
    computation table_3("[NN]->{table_3[i,j]: 0<=i<NN and NN-i<=j<NN and 1<=j and 0<i and NN-i<j}", expr(), true, p_float64, global::get_implicit_function());
    table_3.set_expression(expr(o_max, table(60 - 1 - i, j), table(60 - 1 - i + 1, j - 1) + ((seq(60 - 1 - i) + seq(j)) == 3.0)));
    computation table_4("[NN]->{table_4[i,j]: 0<=i<NN and NN-i<=j<NN and 1<=j and 0<i and j<=NN-i}", expr(), true, p_float64, global::get_implicit_function());
    table_4.set_expression(expr(o_max, table(60 - 1 - i, j), table(60 - 1 - i + 1, j - 1)));
    computation table_5("[NN]->{table_5[i,j,k]: 0<=i<NN and NN-i<=j<NN and NN-i<=k<j}", expr(), true, p_float64, global::get_implicit_function());
    table_5.set_expression(expr(o_max, table(60 - 1 - i, j), table(60 - 1 - i, k) + table(k + 1, j)));

    // -------------------------------------------------------
    // Layer II
    // -------------------------------------------------------
    table_1.then(table_2, j)
        .then(table_3, j)
        .then(table_4, j)
        .then(table_5, j);

    // -------------------------------------------------------
    // Layer III
    // -------------------------------------------------------
    // Input Buffers
    buffer b_table("b_table", {60, 60}, p_float64, a_output);
    buffer b_seq("b_seq", {60}, p_float64, a_input);

    // Store inputs
    table.store_in(&b_table);
    seq.store_in(&b_seq);

    // Store computations
    table_1.set_access("[NN]->{table_1[i,j]->b_table[NN-1-i,j]}");
    table_2.set_access("[NN]->{table_2[i,j]->b_table[NN-1-i,j]}");
    table_3.set_access("[NN]->{table_3[i,j]->b_table[NN-1-i,j]}");
    table_4.set_access("[NN]->{table_4[i,j]->b_table[NN-1-i,j]}");
    table_5.set_access("[NN]->{table_5[i,j,k]->b_table[NN-1-i,j]}");

    // -------------------------------------------------------
    // Code Generation
    // -------------------------------------------------------
    prepare_schedules_for_legality_checks();
    performe_full_dependency_analysis();

    const int beam_size = get_beam_size();
    const int max_depth = get_max_depth();
    declare_memory_usage();

    auto_scheduler::schedules_generator *scheds_gen = new auto_scheduler::ml_model_schedules_generator();
    auto_scheduler::evaluation_function *model_eval = new auto_scheduler::evaluate_by_learning_model(py_cmd_path, {py_interface_path});
    auto_scheduler::evaluate_by_execution *exec_eval = new auto_scheduler::evaluate_by_execution({&b_table, &b_seq}, "generated_nussinov.o", "./nussinov_wrapper");
    auto_scheduler::search_method *bs = new auto_scheduler::beam_search(beam_size, max_depth, model_eval, scheds_gen);
    auto_scheduler::auto_scheduler as(bs, exec_eval);
    as.set_exec_evaluator(exec_eval);
    as.sample_search_space("./nussinov_explored_schedules.json", true);
    delete scheds_gen;
    delete exec_eval;
    delete model_eval;
    delete bs;
    return 0;
}