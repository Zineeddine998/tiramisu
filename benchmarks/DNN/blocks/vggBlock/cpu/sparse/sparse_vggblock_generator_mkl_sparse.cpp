#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include <omp.h>
#include "mkl.h"

#include "mkldnn.hpp"
#include "im2col.hpp"
#include "mkl_spblas.h"


// Original version by: Kyle Spafford Adapted for CSR format
void initRandomWeights(float* filter_values, MKL_INT* filter_idx, MKL_INT* filter_finptr, const int n, const int KK, const int fin_size, const int fout_size, int fin_blocking, const int seed)
{
    int nnzAssigned = 0;
    // Figure out the probability that a nonzero should be assigned to a given
    // spot in the matrix
    int total_num_entries = KK * KK * fin_size * fout_size;
    double prob = (double)n / ((double) total_num_entries);

    // Seed random number generator
    srand(seed);

    // Randomly decide whether entry i,j gets a value, but ensure n values
    // are assigned
    int fillRemaining = 0;

    for (int fout = 0; fout < fout_size; fout++)
    {
      filter_finptr[fout] = (MKL_INT)nnzAssigned;
      for (int fin_b = 0; fin_b < fin_size/fin_blocking; fin_b++)
          for (int ky = 0; ky < KK; ky++)
            for (int kx = 0; kx < KK; kx++)
              for (int ffin = 0; ffin < fin_blocking; ffin++){
                int numEntriesLeft = total_num_entries - ((fout * KK * KK * fin_size) + (fin_b * KK * KK * fin_blocking) + (ky * KK * fin_blocking) + kx * fin_blocking + ffin);
                int needToAssign   = n - nnzAssigned;
                if (numEntriesLeft <= needToAssign) {
                    fillRemaining = 1;
                }
                if ((nnzAssigned < n && ((double) rand() / (RAND_MAX + 1.0)) <= prob) || fillRemaining)
                {
                    filter_idx[nnzAssigned] = (MKL_INT)((fin_b * fin_blocking + ffin) * KK * KK + ky * KK + kx);
                    filter_values[nnzAssigned] = ((float)(rand()%256 - 128)) / 127.f;
                    nnzAssigned++;
                }
              }
    }
    filter_finptr[fout_size] = nnzAssigned;
    if (nnzAssigned != n)
      exit(500);
}

int generateCSRWeights(float *filter_values, float density, MKL_INT *filter_idx, MKL_INT* filter_finptr, int KK, int fin_size, int fout_size, int fin_blocking, int seed) {
    int nNonzero = KK * KK * fin_size * fout_size * density;
    initRandomWeights(filter_values, filter_idx, filter_finptr, nNonzero, KK, fin_size, fout_size, fin_blocking, seed);
    return nNonzero;
}

using namespace mkldnn;

int main()
{
  std::vector<double> duration_vector;

  engine cpu_engine(engine::kind::cpu, 0);
  stream cpu_stream(cpu_engine);

  std::vector<primitive> net;
  std::vector<std::unordered_map<int, memory>> net_args;

  memory::dims pool_strides = {2, 2};
  memory::dims pool_kernel = {2, 2};
  memory::dims pool_padding = {0, 0};

  int FNNZ = FOut*FIn*K*K*WEIGHTS_DENSITY;
  float filter_values[FNNZ];
  MKL_INT filter_idx[FNNZ]; //MKL_INT
  MKL_INT filter_finptr[FOut+1];
  // Generate sparse weights matrix
  generateCSRWeights(filter_values, WEIGHTS_DENSITY, filter_idx, filter_finptr, K, FIn, FOut, FIN1_BLOCKING, 1);

  // Descriptor of main sparse matrix properties
  struct matrix_descr descrFilter;
  // // Structure with sparse matrix stored in CSR format
  sparse_matrix_t       csrFilter;
  float alpha = 1.0, beta = 0.0;

  // Create handle with matrix stored in CSR format
  mkl_sparse_s_create_csr (&csrFilter, SPARSE_INDEX_BASE_ZERO,
                                  FOut,  // number of rows
                                  FIn*K*K,  // number of cols
                                  filter_finptr,
                                  filter_finptr+1,
                                  filter_idx,
                                  filter_values);

  // Analyze sparse matrix; choose proper kernels and workload balancing strategy
  mkl_sparse_optimize(csrFilter);

  // Create matrix descriptor
  descrFilter.type = SPARSE_MATRIX_TYPE_GENERAL;

  // Second convolution weights
  int FNNZ2 = FOut*FOut*K*K*WEIGHTS_DENSITY;
  float filter_values2[FNNZ2];
  MKL_INT filter_idx2[FNNZ2]; //MKL_INT
  MKL_INT filter_finptr2[FOut+1];

  // Generate sparse weights matrix
  generateCSRWeights(filter_values2, WEIGHTS_DENSITY, filter_idx2, filter_finptr2, K, FOut, FOut, FIN2_BLOCKING, 2);

  // Descriptor of main sparse matrix properties
  struct matrix_descr descrFilter2;
  // // Structure with sparse matrix stored in CSR format
  sparse_matrix_t       csrFilter2;

  // Create handle with matrix stored in CSR format
  mkl_sparse_s_create_csr (&csrFilter2, SPARSE_INDEX_BASE_ZERO,
                                  FOut,  // number of rows
                                  FOut*K*K,  // number of cols
                                  filter_finptr2,
                                  filter_finptr2+1,
                                  filter_idx2,
                                  filter_values2);

  // Analyze sparse matrix; choose proper kernels and workload balancing strategy
  mkl_sparse_optimize(csrFilter2);

  // Create matrix descriptor
  descrFilter2.type = SPARSE_MATRIX_TYPE_GENERAL;

  // Allocate buffers
  float* input_buf = (float*)malloc(sizeof(float) * FIn * (N + 2) * (N + 2) * BATCH_SIZE);
  float* conv_bias_buf = (float*)malloc(sizeof(float) * FOut);
  float* conv2_bias_buf = (float*)malloc(sizeof(float) * FOut);
  float* result_buf = (float*)malloc(sizeof(float) * FIn * (N) * (N) * K * K * BATCH_SIZE);
  float* result2_buf = (float*)malloc(sizeof(float) * FOut * (N) * (N) * K * K * BATCH_SIZE);
  float* conv_output_buf = (float*)malloc(sizeof(float) * FOut * (N) * (N) * BATCH_SIZE);

  srand(3);
  for(int b = 0; b < BATCH_SIZE; ++b)
    for (int fin = 0; fin < FIn; ++fin)
      for (int y = 0; y < N + 2; ++y)
        for (int x = 0; x < N + 2; ++x)
          input_buf[x + y*(N+2) + fin*(N+2)*(N+2) + b*(N+2)*(N+2)*FIn] = ((float)(rand() % 256 - 128)) / 127.f;

  for (int i = 0; i < FOut; i++)
    conv_bias_buf[i] = ((float)(rand()%256 - 128)) / 127.f;

  for (int i = 0; i < FOut; i++)
    conv2_bias_buf[i] = ((float)(rand()%256 - 128)) / 127.f;

  printf("Buffers Initialized\n");

  auto conv_output_md = memory::desc(
    {BATCH_SIZE, FOut, N, N},
    memory::data_type::f32,
    memory::format_tag::nchw

  );
  auto conv_output_mem = memory(conv_output_md, cpu_engine, conv_output_buf);

  auto relu_desc = eltwise_forward::desc(prop_kind::forward_inference,
            algorithm::eltwise_relu, conv_output_md,
            0);
  auto relu_pd = eltwise_forward::primitive_desc(relu_desc, cpu_engine);
  net.push_back(eltwise_forward(relu_pd));
  net_args.push_back({
    {MKLDNN_ARG_SRC, conv_output_mem},
    {MKLDNN_ARG_DST, conv_output_mem}
  });

  auto pool_output_md = memory::desc(
    {BATCH_SIZE, FOut, N/2, N/2},
    memory::data_type::f32,
    memory::format_tag::any
  );

  auto pool_d = pooling_forward::desc(
    prop_kind::forward_inference,
    algorithm::pooling_max,
    conv_output_md,
    pool_output_md,
    pool_strides,
    pool_kernel,
    pool_padding,
    pool_padding
  );

  auto pool_pd = pooling_forward::primitive_desc(
    pool_d,
    cpu_engine
  );

  auto pool_dst_mem = memory(pool_pd.dst_desc(), cpu_engine);

  net.push_back(pooling_forward(pool_pd));
  net_args.push_back({
    {MKLDNN_ARG_SRC, conv_output_mem},
    {MKLDNN_ARG_DST, pool_dst_mem}
  });

  omp_set_num_threads(4);
  for (int i = 0; i < NB_TESTS; ++i) {
    double start = rtclock();

    // First convolution
    for(int batch = 0; batch<BATCH_SIZE; batch++){

      // First convolution
      im2col_cpu(&input_buf[batch*(FIn*(N+2)*(N+2))], FIn,
        N+2, N+2, K, K,
        1, 1,
        &result_buf[batch*(FIn*N*N*K*K)]
      );

      mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,
                      alpha,
                      csrFilter,
                      descrFilter,
                      SPARSE_LAYOUT_ROW_MAJOR,
                      &result_buf[batch*(FIn*N*N*K*K)],
                      N*N,   // number of columns in input
                      N*N,   // Number of columns in Filter = Number of rows in Input
                      beta,
                      &conv_output_buf[batch*(FOut*N*N)],
                      N*N // Number of rows in output matrix = Number of rows in filter
      );
      #pragma omp parallel for
      for(int fout = 0; fout<FOut; fout++){
        for(int y=0; y<N; y++)
          for(int x=0; x<N; x++)
            conv_output_buf[batch*(FOut*N*N) + fout*N*N + y*N + x] += conv_bias_buf[fout];
      }
    }

    // ReLU
    net[0].execute(cpu_stream, net_args[0]);
    cpu_stream.wait();

    // Second convolution
    for(int batch = 0; batch<BATCH_SIZE; batch++){

      im2col_cpu_addpadding(&conv_output_buf[batch*(FOut*(N)*(N))], FOut,
        N, N, K, K,
        1, 1,
        &result2_buf[batch*(FOut*N*N*K*K)]
      );

      // Filter weights are (FOut) * (FIn * K * K)
      // Input is           (FIn * K * K) * (N * N)
      // The result of the mult is : (FOut) * (N * N)
      // Calculates C = alpha*A*B + C
      mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,
                      alpha,
                      csrFilter2,
                      descrFilter2,
                      SPARSE_LAYOUT_ROW_MAJOR,
                      &result2_buf[batch*(FOut*N*N*K*K)],
                      N*N,   // number of columns in input
                      N*N,   // Number of columns in Filter = Number of rows in Input
                      beta,
                      &conv_output_buf[batch*(FOut*N*N)],
                      N*N // Number of rows in output matrix = Number of rows in filter
      );
      #pragma omp parallel for
      for(int fout = 0; fout<FOut; fout++){
        for(int y=0; y<N; y++)
          for(int x=0; x<N; x++)
            conv_output_buf[batch*(FOut*N*N) + fout*N*N + y*N + x] += conv2_bias_buf[fout];
      }
    }
    // Execute relu/maxpool
    for (size_t j = 0; j < net.size(); ++j)
      net[j].execute(cpu_stream, net_args[j]);
    cpu_stream.wait();

    double end = rtclock();
    duration_vector.push_back((end - start) * 1000);
  }

  std::cout << "\t\tSparse Lowered Convolution time : "
  << median(duration_vector) << " ms" << std::endl;

  auto output_usr_md = memory::desc(
    {BATCH_SIZE, FOut, N/2, N/2},
    memory::data_type::f32,
    memory::format_tag::nchw
  );

  auto output_mem = memory(output_usr_md, cpu_engine);
  reorder(pool_dst_mem, output_mem)
    .execute(cpu_stream, pool_dst_mem, output_mem);

  if (WRITE_RESULT_TO_FILE){
    float* output_buf = (float*)output_mem.get_data_handle();
    // Write results to file
    FILE* f = fopen("mkl_result.txt", "w");
    if (f == NULL) {
      printf("Error creating mkl_sparse_result.txt.\n");
      return 0;
    }

    for(int b=0; b<BATCH_SIZE; b++)
      for(int fout=0; fout<FOut; fout++)
        for(int y=0; y<N/2; y++)
          for(int x=0; x<N/2; x++)
            fprintf(f, "%.17g\n", output_buf[x + y*N/2 + fout*N/2*N/2 + b*N/2*N/2*FOut]);

    fclose(f);
  }
  mkl_sparse_destroy(csrFilter);
  free(input_buf);
  free(result_buf);
  free(result2_buf);
  free(conv_output_buf);
  return 0;
}
