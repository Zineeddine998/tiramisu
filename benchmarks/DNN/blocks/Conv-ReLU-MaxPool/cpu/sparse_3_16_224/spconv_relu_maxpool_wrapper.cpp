#include "generated_spconv_relu_maxpool.o.h"

#include "Halide.h"
#include <tiramisu/utils.h>
#include <cstdlib>
#include <iostream>
#include "configure.h"

// Original version by: Kyle Spafford Adapted for CSR format
void initRandomWeights(float* filter_values, int* filter_idx, int* filter_finptr, const int n, const int KK, const int fin_size, const int fout_size, int seed)
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
      filter_finptr[fout] = nnzAssigned;
      for (int fin = 0; fin < fin_size; fin++)
      {
        for (int ky = 0; ky < KK; ky++)
        {
          for (int kx = 0; kx < KK; kx++)
          {
            int numEntriesLeft = total_num_entries - ((fout * KK * KK * fin_size) + (fin * KK * KK) + (ky * KK) + kx);
            int needToAssign   = n - nnzAssigned;
            if (numEntriesLeft <= needToAssign) {
                fillRemaining = 1;
            }
            if ((nnzAssigned < n && ((double) rand() / (RAND_MAX + 1.0)) <= prob) || fillRemaining)
            {
                filter_idx[nnzAssigned] = fin * (N + 2) * (N + 2) + ky * (N + 2) + kx;
                filter_values[nnzAssigned] = ((float)(rand()%256 - 128)) / 127.f;
                nnzAssigned++;
            }
          }
        }
      }
    }
    filter_finptr[fout_size] = nnzAssigned;
    assert(nnzAssigned == n);
}

int generateCSRWeights(float **filter_values, float density, int **filter_idx, int** filter_finptr, int KK, int fin_size, int fout_size, int seed) {
    int nNonzero = KK * KK * fin_size * fout_size * density;
    *filter_values = (float *) malloc(nNonzero * sizeof(float));
    *filter_idx = (int *) malloc(nNonzero * sizeof(int));
    *filter_finptr = (int *) malloc((fout_size + 1) * sizeof(int));
    initRandomWeights(*filter_values, *filter_idx, *filter_finptr, nNonzero, KK, fin_size, fout_size, seed);
    return nNonzero;
}

int main(int, char **)
{
    std::vector<double> duration_vector;
    double start, end;

    // ---------------------------------------------------------------------
    // ---------------------------------------------------------------------
    // ---------------------------------------------------------------------

    float *filter_values;
    int *filter_idx;
    int *filter_finptr;

    int FNNZ = generateCSRWeights(&filter_values, WEIGHTS_DENSITY, &filter_idx, &filter_finptr, K, FIn, FOut, 1);

    Halide::Buffer<int> b_SIZES(1);
    b_SIZES(0) = FNNZ;
    Halide::Buffer<float> b_input((N + 2) * (N + 2) * FIn, BATCH_SIZE);

    Halide::Buffer<float> b_result(N / 2 + 2 * PAD_OUTPUT, N / 2 + 2 * PAD_OUTPUT, FOut, BATCH_SIZE);
    Halide::Buffer<float> b_workspace(N, N, FOut, BATCH_SIZE);

    Halide::Buffer<float> b_filter_values(filter_values, FNNZ);
    Halide::Buffer<int> b_filter_idx(filter_idx, FNNZ);
    Halide::Buffer<int> b_filter_finptr(filter_finptr, FOut + 1);

    Halide::Buffer<float> b_bias(FOut);

    srand(3);
    for (int n=0; n < BATCH_SIZE; ++n)
      for (int z=0; z < FIn; ++z)
        for (int y=0; y < N + 2; ++y)
          for (int x=0; x < N + 2; ++x)
            b_input(x + y * (N + 2) + z * (N + 2) * (N + 2), n) = ((float)(rand()%256 - 128)) / 127.f;

    for (int q=0; q<FOut; q++)
      b_bias(q) = ((float)(rand()%256 - 128)) / 127.f;

    std::cout << "Buffers Initialized" << std::endl;
    for (int i = 0; i < NB_TESTS; i++)
    {
	    start = rtclock();
  		spconv_relu_maxpool(b_SIZES.raw_buffer(), b_input.raw_buffer(), b_filter_values.raw_buffer(), b_filter_idx.raw_buffer(), b_filter_finptr.raw_buffer(), b_bias.raw_buffer(), b_result.raw_buffer());
      end = rtclock();
      duration_vector.push_back((end - start) * 1000);
    }
    if (SHOW_OUTPUT)
      print_buffer(b_result);
    print_time("performance_CPU.csv", "SpConv-ReLU-MaxPool",
               {"Tiramisu"},
               {median(duration_vector)});

    if (CHECK_CORRECTNESS){
      // Compare results with Intel MKL
      std::ifstream mkldnn_result("mkl_result.txt");
      double tmp;
      long nb_correct = 0;

      for(int b=0; b<BATCH_SIZE; b++)
        for(int fout=0; fout<FOut; fout++)
          for(int y=0; y<N / 2; y++)
            for(int x=0; x< N / 2; x++){
              mkldnn_result >> tmp;
              if (std::abs(b_result(x + PAD_OUTPUT, y + PAD_OUTPUT, fout, b) - tmp) <= 0.0001)
                nb_correct++;
            }
      std::cout << "\t\tPercentage of correctness " << 100*(((double)nb_correct)/(BATCH_SIZE * FOut * (N / 2) * (N / 2))) << "%" << std::endl << std::endl;
    }
    free(filter_idx);
    free(filter_values);
    free(filter_finptr);
    return 0;
}
