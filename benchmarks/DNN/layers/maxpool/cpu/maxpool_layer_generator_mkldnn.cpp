#include <chrono>
#include <iostream>
#include <fstream>
#include <numeric>
#include <math.h>
#include <string>
#include <time.h>
#include "mkldnn.hpp"
#include "configure.h"

using namespace mkldnn;
using namespace std;

void maxpool()
{
    auto cpu_engine = engine(engine::cpu, 0);
    std::vector<float> net_src(BATCH_SIZE * FIn * N * N);
    std::vector<float> net_dst(BATCH_SIZE * FIn * ((N - K_Y + 2 * P_Y) / S_Y + 1) * ((N - K_X + 2 * P_X) / S_X + 1));

    /* Initializing non-zero values for src */
    srand(1);
    for (size_t i = 0; i < net_src.size(); ++i)
        net_src[i] = rand() % 10;

    memory::dims src_tz = {BATCH_SIZE, FIn, N, N};

    /* Create memory for user data */
    auto user_src_memory = memory(
        {{{src_tz}, memory::data_type::f32, memory::format::nchw},
         cpu_engine},
        net_src.data());

    /* Create mmemory descriptors for source data */
    auto src_md = memory::desc({src_tz}, memory::data_type::f32,
                               memory::format::nchw);

    /* Pool */
    memory::dims pool_dst_tz = {BATCH_SIZE, FIn, ((N - K_Y + 2 * P_Y) / S_Y + 1), ((N - K_X + 2 * P_X) / S_X + 1)};
    memory::dims pool_kernel = {K_Y, K_X};
    memory::dims pool_strides = {S_Y, S_X};
    auto pool_padding = {P_Y, P_X};

    /* Create memory for pool dst data in user format */
    auto pool_user_dst_memory = memory(
        {{{pool_dst_tz}, memory::data_type::f32, memory::format::nchw},
         cpu_engine},
        net_dst.data());

    /* Create pool dst memory descriptor in format any */
    auto pool_dst_md = memory::desc({pool_dst_tz}, memory::data_type::f32,
                                    memory::format::any);

    /* Create a pooling primitive descriptor */
    auto pool_desc = pooling_forward::desc(
        prop_kind::forward, pooling_max,
        src_md, pool_dst_md,
        pool_strides, pool_kernel, pool_padding, pool_padding,
        padding_kind::zero);
    auto pool_pd = pooling_forward::primitive_desc(pool_desc, cpu_engine);

    /* Create reorder primitive between pool dst and user dst format if needed */
    auto pool_dst_memory = pool_user_dst_memory;
    bool reorder_pool_dst = false;
    primitive pool_reorder_dst;
    if (memory::primitive_desc(pool_pd.dst_primitive_desc()) != pool_user_dst_memory.get_primitive_desc())
    {
        pool_dst_memory = memory(pool_pd.dst_primitive_desc());
        pool_reorder_dst = reorder(pool_dst_memory, pool_user_dst_memory);
        reorder_pool_dst = true;
    }

    /* Create pooling workspace memory if training */
    auto pool_workspace_memory = memory(pool_pd.workspace_primitive_desc());

    /* Finally create a pooling primitive */
    auto pool = pooling_forward(pool_pd, user_src_memory, pool_dst_memory,
                                pool_workspace_memory);

    /* Build forward net */
    std::vector<primitive> net_fwd;
    net_fwd.push_back(pool);
    if (reorder_pool_dst)
        net_fwd.push_back(pool_reorder_dst);

    std::vector<double> duration_vector;
    for (int i = 0; i < NB_TESTS; i++)
    {
        double start = rtclock();
        stream(stream::kind::eager).submit(net_fwd).wait();

        double end = rtclock();
        duration_vector.push_back((end - start) * 1000);
    }
    std::cout << "\t\tMKL-DNN maxpool duration"
              << ": " << median(duration_vector) << "; " << std::endl;

    printf("writing result in file\n");
    ofstream resultfile;
    resultfile.open("mkldnn_result.txt");

    float *poolres = (float *)pool_dst_memory.get_data_handle();
    for (size_t i = 0; i < BATCH_SIZE; ++i)
        for (size_t j = 0; j < FIn; ++j)
            for (size_t k = 0; k < ((N - K_Y + 2 * P_Y) / S_Y + 1); ++k)
                for (size_t l = 0; l < ((N - K_X + 2 * P_X) / S_X + 1); ++l)
                    resultfile << poolres[i * FIn * ((N - K_Y + 2 * P_Y) / S_Y + 1) * ((N - K_X + 2 * P_X) / S_X + 1) + j * ((N - K_Y + 2 * P_Y) / S_Y + 1) * ((N - K_X + 2 * P_X) / S_X + 1) + k * ((N - K_X + 2 * P_X) / S_X + 1) + l];
    resultfile.close();
}

int main(int argc, char **argv)
{
    try
    {
        maxpool();
    }
    catch (error &e)
    {
        std::cerr << "status: " << e.status << std::endl;
        std::cerr << "message: " << e.message << std::endl;
    }
    return 0;
}
