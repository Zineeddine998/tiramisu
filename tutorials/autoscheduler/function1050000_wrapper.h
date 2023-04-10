#ifndef HALIDE__generated_function_blur_no_schedule_h
#define HALIDE__generated_function_blur_no_schedule_h
#include <tiramisu/utils.h>
#include <sys/time.h>
#include <cstdlib>
#include <algorithm>
#include <vector>

#define NB_THREAD_INIT 48
struct args
{
    double *buf;
    unsigned long long int part_start;
    unsigned long long int part_end;
    double value;
};

void *init_part(void *params)
{
    double *buffer = ((struct args *)params)->buf;
    unsigned long long int start = ((struct args *)params)->part_start;
    unsigned long long int end = ((struct args *)params)->part_end;
    double val = ((struct args *)params)->value;
    for (unsigned long long int k = start; k < end; k++)
    {
        buffer[k] = val;
    }
    pthread_exit(NULL);
}

void parallel_init_buffer(double *buf, unsigned long long int size, double value)
{
    pthread_t threads[NB_THREAD_INIT];
    struct args params[NB_THREAD_INIT];
    for (int i = 0; i < NB_THREAD_INIT; i++)
    {
        unsigned long long int start = i * size / NB_THREAD_INIT;
        unsigned long long int end = std::min((i + 1) * size / NB_THREAD_INIT, size);
        params[i] = (struct args){buf, start, end, value};
        pthread_create(&threads[i], NULL, init_part, (void *)&(params[i]));
    }
    for (int i = 0; i < NB_THREAD_INIT; i++)
        pthread_join(threads[i], NULL);
    return;
}
#ifdef __cplusplus
extern "C"
{
#endif
    int function1050000(halide_buffer_t *buf00, halide_buffer_t *buf01);
#ifdef __cplusplus
} // extern "C"
#endif

double get_init_exec_time()
{
    if (std::getenv("INIT_EXEC_TIME") != NULL)
        return std::stod(std::getenv("INIT_EXEC_TIME"));
    else
    {
        std::cerr << "error: Environment Variable INIT_EXEC_TIME not declared" << std::endl;
        exit(1);
    }
}

#endif