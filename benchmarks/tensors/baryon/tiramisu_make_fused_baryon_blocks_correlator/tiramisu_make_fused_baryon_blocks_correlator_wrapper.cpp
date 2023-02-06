#include "Halide.h"
#include <tiramisu/utils.h>
#include <cstdlib>
#include <iostream>
#include <complex>
#include "benchmarks.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "tiramisu_make_fused_baryon_blocks_correlator_wrapper.h"
#include "tiramisu_make_fused_baryon_blocks_correlator_ref.cpp"

#define RUN_REFERENCE 1
#define RUN_CHECK 1
   int nb_tests = 1;
   int randommode = 0;

   void tiramisu_make_nucleon_2pt(double *C_re,
                                  double *C_im,
                                  double *B1_prop_re,
                                  double *B1_prop_im,
                                  int *src_color_weights_r1,
                                  int *src_spin_weights_r1,
                                  double *src_weights_r1,
                                  int *src_color_weights_r2,
                                  int *src_spin_weights_r2,
                                  double *src_weights_r2,
                                  int *perms,
                                  int *sigs,
                                  double *src_psi_B1_re,
                                  double *src_psi_B1_im,
                                  double *snk_psi_B1_re,
                                  double *snk_psi_B1_im)
   {

      int q, t, iC, iS, jC, jS, y, x, x1, x2, m, n, k, wnum, b, rp, r;
      int iC1, iS1, iC2, iS2, jC1, jS1, jC2, jS2, kC1, kS1, kC2, kS2;

      int rank = 0;

      // printf("hi I'm rank %d \n", rank);

      if (rank == 0)
      {
         long mega = 1024 * 1024;
      }

      // Halide buffers
      Halide::Buffer<double> b_C_r(NsnkHex, B1Nrows, NsrcHex, B1Nrows, Vsnk / sites_per_rank, Lt, "C_r");
      Halide::Buffer<double> b_C_i(NsnkHex, B1Nrows, NsrcHex, B1Nrows, Vsnk / sites_per_rank, Lt, "C_i");

      Halide::Buffer<int> b_src_color_weights(Nq, Nw, B1Nrows, "src_color_weights");
      Halide::Buffer<int> b_src_spin_weights(Nq, Nw, B1Nrows, "src_spin_weights");
      Halide::Buffer<double> b_src_weights(Nw, B1Nrows, "src_weights");

      Halide::Buffer<int> b_src_spins(B1Nrows, "src_spins");
      Halide::Buffer<int> b_snk_color_weights(Nq, Nw, B1Nperms, B1Nrows, "snk_color_weights");
      Halide::Buffer<int> b_snk_spin_weights(Nq, Nw, B1Nperms, B1Nrows, "snk_spin_weights");
      Halide::Buffer<double> b_snk_weights(Nw, B1Nrows, "snk_weights");

      // prop
      Halide::Buffer<double> b_B1_prop_r((double *)B1_prop_re, {Vsrc, Vsnk, Ns, Nc, Ns, Nc, Lt, Nq});
      Halide::Buffer<double> b_B1_prop_i((double *)B1_prop_im, {Vsrc, Vsnk, Ns, Nc, Ns, Nc, Lt, Nq});

      // psi
      Halide::Buffer<double> b_B1_src_psi_r((double *)src_psi_B1_re, {NsrcHex, Vsrc});
      Halide::Buffer<double> b_B1_src_psi_i((double *)src_psi_B1_im, {NsrcHex, Vsrc});
      Halide::Buffer<double> b_B1_snk_psi_r((double *)snk_psi_B1_re, {NsnkHex, sites_per_rank, Vsnk / sites_per_rank});
      Halide::Buffer<double> b_B1_snk_psi_i((double *)snk_psi_B1_im, {NsnkHex, sites_per_rank, Vsnk / sites_per_rank});

      Halide::Buffer<int> b_sigs((int *)sigs, {B1Nperms});

      // Weights

      int *snk_color_weights_r1 = (int *)malloc(Nw * Nq * sizeof(int));
      int *snk_color_weights_r2 = (int *)malloc(Nw * Nq * sizeof(int));
      int *snk_spin_weights_r1 = (int *)malloc(Nw * Nq * sizeof(int));
      int *snk_spin_weights_r2 = (int *)malloc(Nw * Nq * sizeof(int));
      for (int nB1 = 0; nB1 < Nw; nB1++)
      {
         b_src_weights(nB1, 0) = src_weights_r1[nB1];
         b_src_weights(nB1, 1) = src_weights_r2[nB1];
         b_snk_weights(nB1, 0) = src_weights_r1[nB1];
         b_snk_weights(nB1, 1) = src_weights_r2[nB1];
         for (int nq = 0; nq < Nq; nq++)
         {
            // G1g_r1
            snk_color_weights_r1[index_2d(nB1, nq, Nq)] = src_color_weights_r1[index_2d(nB1, nq, Nq)];
            snk_spin_weights_r1[index_2d(nB1, nq, Nq)] = src_spin_weights_r1[index_2d(nB1, nq, Nq)];
            // G1g_r2
            snk_color_weights_r2[index_2d(nB1, nq, Nq)] = src_color_weights_r2[index_2d(nB1, nq, Nq)];
            snk_spin_weights_r2[index_2d(nB1, nq, Nq)] = src_spin_weights_r2[index_2d(nB1, nq, Nq)];
         }
      }
      b_src_spins(0) = 1;
      b_src_spins(1) = 2;
      for (int nperm = 0; nperm < B1Nperms; nperm++)
      {
         int snk_1 = perms[index_2d(nperm, 0, Nq)] - 1;
         int snk_2 = perms[index_2d(nperm, 1, Nq)] - 1;
         int snk_3 = perms[index_2d(nperm, 2, Nq)] - 1;
         for (int wnum = 0; wnum < Nw; wnum++)
         {
            b_src_color_weights(0, wnum, 0) = snk_color_weights_r1[index_2d(wnum, 0, Nq)];
            b_src_spin_weights(0, wnum, 0) = snk_spin_weights_r1[index_2d(wnum, 0, Nq)];
            b_src_color_weights(1, wnum, 0) = snk_color_weights_r1[index_2d(wnum, 1, Nq)];
            b_src_spin_weights(1, wnum, 0) = snk_spin_weights_r1[index_2d(wnum, 1, Nq)];
            b_src_color_weights(2, wnum, 0) = snk_color_weights_r1[index_2d(wnum, 2, Nq)];
            b_src_spin_weights(2, wnum, 0) = snk_spin_weights_r1[index_2d(wnum, 2, Nq)];

            b_src_color_weights(0, wnum, 1) = snk_color_weights_r2[index_2d(wnum, 0, Nq)];
            b_src_spin_weights(0, wnum, 1) = snk_spin_weights_r2[index_2d(wnum, 0, Nq)];
            b_src_color_weights(1, wnum, 1) = snk_color_weights_r2[index_2d(wnum, 1, Nq)];
            b_src_spin_weights(1, wnum, 1) = snk_spin_weights_r2[index_2d(wnum, 1, Nq)];
            b_src_color_weights(2, wnum, 1) = snk_color_weights_r2[index_2d(wnum, 2, Nq)];
            b_src_spin_weights(2, wnum, 1) = snk_spin_weights_r2[index_2d(wnum, 2, Nq)];

            b_snk_color_weights(0, wnum, nperm, 0) = snk_color_weights_r1[index_2d(wnum, snk_1, Nq)];
            b_snk_spin_weights(0, wnum, nperm, 0) = snk_spin_weights_r1[index_2d(wnum, snk_1, Nq)];
            b_snk_color_weights(1, wnum, nperm, 0) = snk_color_weights_r1[index_2d(wnum, snk_2, Nq)];
            b_snk_spin_weights(1, wnum, nperm, 0) = snk_spin_weights_r1[index_2d(wnum, snk_2, Nq)];
            b_snk_color_weights(2, wnum, nperm, 0) = snk_color_weights_r1[index_2d(wnum, snk_3, Nq)];
            b_snk_spin_weights(2, wnum, nperm, 0) = snk_spin_weights_r1[index_2d(wnum, snk_3, Nq)];

            b_snk_color_weights(0, wnum, nperm, 1) = snk_color_weights_r2[index_2d(wnum, snk_1, Nq)];
            b_snk_spin_weights(0, wnum, nperm, 1) = snk_spin_weights_r2[index_2d(wnum, snk_1, Nq)];
            b_snk_color_weights(1, wnum, nperm, 1) = snk_color_weights_r2[index_2d(wnum, snk_2, Nq)];
            b_snk_spin_weights(1, wnum, nperm, 1) = snk_spin_weights_r2[index_2d(wnum, snk_2, Nq)];
            b_snk_color_weights(2, wnum, nperm, 1) = snk_color_weights_r2[index_2d(wnum, snk_3, Nq)];
            b_snk_spin_weights(2, wnum, nperm, 1) = snk_spin_weights_r2[index_2d(wnum, snk_3, Nq)];
         }
      }

      for (int rp = 0; rp < B1Nrows; rp++)
         for (int m = 0; m < NsrcHex; m++)
            for (int r = 0; r < B1Nrows; r++)
               for (int n = 0; n < NsnkHex; n++)
                  for (int t = 0; t < Lt; t++)
                     for (int x = 0; x < Vsnk / sites_per_rank; x++)
                     {
                        b_C_r(n, r, m, rp, x, t) = 0.0;
                        b_C_i(n, r, m, rp, x, t) = 0.0;
                     }

      tiramisu_make_fused_baryon_blocks_correlator(
          b_C_r.raw_buffer(),
          b_C_i.raw_buffer(),
          b_B1_prop_r.raw_buffer(),
          b_B1_prop_i.raw_buffer(),
          b_B1_src_psi_r.raw_buffer(),
          b_B1_src_psi_i.raw_buffer(),
          b_B1_snk_psi_r.raw_buffer(),
          b_B1_snk_psi_i.raw_buffer(),
          b_src_color_weights.raw_buffer(),
          b_src_spin_weights.raw_buffer(),
          b_src_weights.raw_buffer(),
          b_src_spins.raw_buffer(),
          b_snk_color_weights.raw_buffer(),
          b_snk_spin_weights.raw_buffer(),
          b_snk_weights.raw_buffer(),
          b_sigs.raw_buffer());

      if (rank == 0)
      {
      }

      // symmetrize and such
#ifdef WITH_MPI

      for (int rp = 0; rp < B1Nrows; rp++)
         for (int m = 0; m < NsrcHex; m++)
            for (int r = 0; r < B1Nrows; r++)
               for (int n = 0; n < NsnkHex; n++)
                  for (int t = 0; t < Lt; t++)
                  {
                     double number0r;
                     double number0i;
                     double this_number0r = b_C_r(n, r, m, rp, rank, t);
                     double this_number0i = b_C_i(n, r, m, rp, rank, t);
                     MPI_Allreduce(&this_number0r, &number0r, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
                     MPI_Allreduce(&this_number0i, &number0i, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
                     C_re[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)] += number0r;
                     C_im[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)] += number0i;
                  }
#else
   for (int rp = 0; rp < B1Nrows; rp++)
      for (int m = 0; m < NsrcHex; m++)

         for (int r = 0; r < B1Nrows; r++)
            for (int n = 0; n < NsnkHex; n++)
               for (int t = 0; t < Lt; t++)
                  for (int x = 0; x < Vsnk; x++)
                  {
                     double number0r;
                     double number0i;
                     number0r = b_C_r(n, r, m, rp, x, t);
                     number0i = b_C_i(n, r, m, rp, x, t);
                     C_re[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)] += number0r;
                     C_im[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)] += number0i;
                  }
#endif
   }

   int main(int, char **)
   {

      int rank = 0;
#ifdef WITH_MPI
      rank = tiramisu_MPI_init();
#endif

      srand(0);

      std::vector<std::chrono::duration<double, std::milli>> duration_vector_1;
      std::vector<std::chrono::duration<double, std::milli>> duration_vector_2;

      int q, t, iC, iS, jC, jS, y, x, x1, x2, m, n, k, wnum, rp, r;
      int iC1, iS1, iC2, iS2, jC1, jS1, jC2, jS2, kC1, kS1, kC2, kS2;

      // Initialization
      // Props
      double *B1_prop_re = (double *)malloc(Nq * Lt * Nc * Ns * Nc * Ns * Vsnk * Vsrc * sizeof(double));
      double *B1_prop_im = (double *)malloc(Nq * Lt * Nc * Ns * Nc * Ns * Vsnk * Vsrc * sizeof(double));
      for (q = 0; q < Nq; q++)
      {
         for (t = 0; t < Lt; t++)
         {
            for (iC = 0; iC < Nc; iC++)
            {
               for (iS = 0; iS < Ns; iS++)
               {
                  for (jC = 0; jC < Nc; jC++)
                  {
                     for (jS = 0; jS < Ns; jS++)
                     {
                        for (y = 0; y < Vsrc; y++)
                        {
                           for (x = 0; x < Vsnk; x++)
                           {
                              if (randommode == 1)
                              {
                                 double v1 = rand() % 10;
                                 double v2 = rand() % 10;
                                 double v3 = rand() % 10;
                                 double v4 = rand() % 10;
                                 B1_prop_re[prop_index(q, t, jC, jS, iC, iS, y, x, Nc, Ns, Vsrc, Vsnk, Lt)] = v1;
                                 B1_prop_im[prop_index(q, t, jC, jS, iC, iS, y, x, Nc, Ns, Vsrc, Vsnk, Lt)] = v3;
                              }
                              else
                              {
                                 if ((jC == iC) && (jS == iS))
                                 {
                                    B1_prop_re[prop_index(q, t, jC, jS, iC, iS, y, x, Nc, Ns, Vsrc, Vsnk, Lt)] = 1 / mq * cos(2 * M_PI / 6);
                                    B1_prop_im[prop_index(q, t, jC, jS, iC, iS, y, x, Nc, Ns, Vsrc, Vsnk, Lt)] = 1 / mq * sin(2 * M_PI / 6);
                                 }
                                 else
                                 {
                                    B1_prop_re[prop_index(q, t, jC, jS, iC, iS, y, x, Nc, Ns, Vsrc, Vsnk, Lt)] = 0;
                                    B1_prop_im[prop_index(q, t, jC, jS, iC, iS, y, x, Nc, Ns, Vsrc, Vsnk, Lt)] = 0;
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
      // Wavefunctions
      double *src_psi_B1_re = (double *)malloc(Nsrc * Vsrc * sizeof(double));
      double *src_psi_B1_im = (double *)malloc(Nsrc * Vsrc * sizeof(double));
      for (m = 0; m < Nsrc; m++)
         for (x = 0; x < Vsrc; x++)
         {
            double v1 = 1.0;
            double v2 = 0.0;
            if (randommode == 1)
            {
               v1 = rand() % 10;
               v2 = rand() % 10;
            }
            src_psi_B1_re[index_2d(x, m, Nsrc)] = v1; // / Vsrc;
            src_psi_B1_im[index_2d(x, m, Nsrc)] = v2; // / Vsrc;
         }
      double *snk_psi_B1_re = (double *)malloc(Nsnk * Vsnk * sizeof(double));
      double *snk_psi_B1_im = (double *)malloc(Nsnk * Vsnk * sizeof(double));
      for (n = 0; n < Nsnk; n++)
      {
         for (x = 0; x < Vsnk; x++)
         {
            double v1 = 1.0;
            double v2 = 0.0;
            if (randommode == 1)
            {
               v1 = rand() % 10;
               v2 = rand() % 10;
            }
            snk_psi_B1_re[index_2d(x, n, Nsnk)] = v1; // / Vsnk;
            snk_psi_B1_im[index_2d(x, n, Nsnk)] = v2; // / Vsnk;
         }
      }
      // Weights
      /*static int src_color_weights_r1_P[Nw][Nq] = { {0,1,2}, {0,2,1}, {1,0,2} ,{0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,1,0}, {2,0,1} };
      static int src_spin_weights_r1_P[Nw][Nq] = { {0,1,0}, {0,1,0}, {0,1,0}, {1,0,0}, {1,0,0}, {1,0,0}, {1,0,0}, {1,0,0}, {1,0,0} };
      static double src_weights_r1_P[Nw] = {-2/ sqrt(2), 2/sqrt(2), 2/sqrt(2), 1/sqrt(2), -1/sqrt(2), -1/sqrt(2), 1/sqrt(2), -1/sqrt(2), 1/sqrt(2)};
      static int src_color_weights_r2_P[Nw][Nq] = { {0,1,2}, {0,2,1}, {1,0,2} ,{1,2,0}, {2,1,0}, {2,0,1}, {0,1,2}, {0,2,1}, {1,0,2} };
      static int src_spin_weights_r2_P[Nw][Nq] = { {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1}, {1,0,1}, {1,0,1}, {1,0,1} };
      static double src_weights_r2_P[Nw] = {1/ sqrt(2), -1/sqrt(2), -1/sqrt(2), 1/sqrt(2), -1/sqrt(2), 1/sqrt(2), -2/sqrt(2), 2/sqrt(2), 2/sqrt(2)}; */

      static int src_color_weights_r1_P[Nw][Nq] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 1, 0}, {2, 0, 1}, {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 1, 0}, {2, 0, 1}};
      static int src_spin_weights_r1_P[Nw][Nq] = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}};
      static double src_weights_r1_P[Nw] = {-1 / sqrt(2), 1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2), -1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2), 1 / sqrt(2)};

      static int src_color_weights_r2_P[Nw][Nq] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 1, 0}, {2, 0, 1}, {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 1, 0}, {2, 0, 1}};
      static int src_spin_weights_r2_P[Nw][Nq] = {{0, 1, 1}, {0, 1, 1}, {0, 1, 1}, {0, 1, 1}, {0, 1, 1}, {0, 1, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}, {1, 0, 1}};
      static double src_weights_r2_P[Nw] = {-1 / sqrt(2), 1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2), -1 / sqrt(2), 1 / sqrt(2), -1 / sqrt(2), 1 / sqrt(2)};

      int *src_color_weights_r1 = (int *)malloc(Nw * Nq * sizeof(int));
      int *src_color_weights_r2 = (int *)malloc(Nw * Nq * sizeof(int));
      int *src_spin_weights_r1 = (int *)malloc(Nw * Nq * sizeof(int));
      int *src_spin_weights_r2 = (int *)malloc(Nw * Nq * sizeof(int));
      double src_weights_r1[Nw];
      double src_weights_r2[Nw];
      for (wnum = 0; wnum < Nw; wnum++)
      {
         for (q = 0; q < Nq; q++)
         {
            src_color_weights_r1[index_2d(wnum, q, Nq)] = src_color_weights_r1_P[wnum][q];
            src_color_weights_r2[index_2d(wnum, q, Nq)] = src_color_weights_r2_P[wnum][q];
            src_spin_weights_r1[index_2d(wnum, q, Nq)] = src_spin_weights_r1_P[wnum][q];
            src_spin_weights_r2[index_2d(wnum, q, Nq)] = src_spin_weights_r2_P[wnum][q];
         }
         src_weights_r1[wnum] = src_weights_r1_P[wnum];
         src_weights_r2[wnum] = src_weights_r2_P[wnum];
      }
      int perms_array[2][3] = {{1, 2, 3}, {3, 2, 1}};
      int sigs_array[2] = {1, -1};
      int *perms = (int *)malloc(B1Nperms * Nq * sizeof(int));
      int sigs[B1Nperms];
      int permnum = 0;
      for (int i = 0; i < B1Nperms; i++)
      {
         for (int q = 0; q < Nq; q++)
         {
            perms[index_2d(permnum, q, Nq)] = perms_array[i][q];
         }
         sigs[permnum] = sigs_array[i];
         permnum += 1;
      }
      // Correlators
      double *C_re = (double *)malloc(B1Nrows * B1Nrows * (NsrcHex) * (NsnkHex)*Lt * sizeof(double));
      double *C_im = (double *)malloc(B1Nrows * B1Nrows * (NsrcHex) * (NsnkHex)*Lt * sizeof(double));
      double *t_C_re = (double *)malloc(B1Nrows * B1Nrows * (NsrcHex) * (NsnkHex)*Lt * sizeof(double));
      double *t_C_im = (double *)malloc(B1Nrows * B1Nrows * (NsrcHex) * (NsnkHex)*Lt * sizeof(double));
      for (rp = 0; rp < B1Nrows; rp++)
         for (m = 0; m < NsrcHex; m++)
            for (r = 0; r < B1Nrows; r++)
               for (n = 0; n < NsnkHex; n++)
                  for (t = 0; t < Lt; t++)
                  {
                     C_re[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)] = 0.0;
                     C_im[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)] = 0.0;
                     t_C_re[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)] = 0.0;
                     t_C_im[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)] = 0.0;
                  }

      for (int i = 0; i < nb_tests; i++)
      {

         auto start1 = std::chrono::high_resolution_clock::now();

         tiramisu_make_nucleon_2pt(t_C_re,
                                   t_C_im,
                                   B1_prop_re,
                                   B1_prop_im,
                                   src_color_weights_r1,
                                   src_spin_weights_r1,
                                   src_weights_r1,
                                   src_color_weights_r2,
                                   src_spin_weights_r2,
                                   src_weights_r2,
                                   perms,
                                   sigs,
                                   src_psi_B1_re,
                                   src_psi_B1_im,
                                   snk_psi_B1_re,
                                   snk_psi_B1_im); //, Nc, Ns, Vsrc, Vsnk, Lt, Nw, Nq, NsrcHex, NsnkHex);

         auto end1 = std::chrono::high_resolution_clock::now();
         std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start1).count() / (double)1000000 << " " << std::flush;
      }
      std::cout << std::endl;

      if (rank == 0)
      {

#if RUN_REFERENCE
         for (int i = 0; i < nb_tests; i++)
         {
            // {
            //    auto start1 = std::chrono::high_resolution_clock::now();
            //    make_nucleon_2pt(C_re, C_im, B1_prop_re, B1_prop_im, src_color_weights_r1, src_spin_weights_r1, src_weights_r1, src_color_weights_r2, src_spin_weights_r2, src_weights_r2, src_psi_B1_re, src_psi_B1_im, snk_psi_B1_re, snk_psi_B1_im, Nc, Ns, Vsrc, Vsnk, Lt, Nw, Nq, NsrcHex, NsnkHex);
            //    auto end1 = std::chrono::high_resolution_clock::now();
            //    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start1).count() / (double)1000000 << " " << std::flush;

            auto start2 = std::chrono::high_resolution_clock::now();

            make_nucleon_2pt(C_re, C_im, B1_prop_re, B1_prop_im, src_color_weights_r1, src_spin_weights_r1, src_weights_r1, src_color_weights_r2, src_spin_weights_r2, src_weights_r2, src_psi_B1_re, src_psi_B1_im, snk_psi_B1_re, snk_psi_B1_im, Nc, Ns, Vsrc, Vsnk, Lt, Nw, Nq, NsrcHex, NsnkHex);

            auto end2 = std::chrono::high_resolution_clock::now();
            auto ref_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - start2).count() / (double)1000000;
            float float_ref_time = (float)ref_time;

            std::ofstream fw("/data/cs7214/tiramisu/benchmarks/tensors/baryon/tiramisu_make_fused_baryon_blocks_correlator/test.txt", std::ofstream::out);
            if (fw.is_open())
            {

               fw << std::to_string(float_ref_time) << "\n";
               fw.close();
            }
         }
#endif

#if RUN_CHECK

         // for (rp=0; rp<B1Nrows; rp++) {
         //    printf("\n");
         //    for (m=0; m<NsrcHex; m++)
         //       for (r=0; r<B1Nrows; r++)
         //          for (n=0; n<NsnkHex; n++)
         //             for (t=0; t<Lt; t++) {
         //                std::cout << "\n\n\n--------------------------------";
         //                std::cout << "C_re\n";
         //                std::cout << std::to_string(C_re[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)]) << "\n";
         //                std::cout << "t_C_re\n";
         //                std::cout << std::to_string(t_C_re[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)]) << "\n";
         //                std::cout << "C_im\n";
         //                std::cout << std::to_string(C_im[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)]) << "\n";
         //                std::cout << "t_C_im\n";
         //                std::cout << std::to_string(t_C_im[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)]) << "\n";
         //                std::cout << "\n----------------------------\n";
         //                if ((std::abs(C_re[index_5d(rp,m,r,n,t, NsrcHex,B1Nrows,NsnkHex,Lt)] - t_C_re[index_5d(rp,m,r,n,t, NsrcHex,B1Nrows,NsnkHex,Lt)]) >= 0.01*Vsnk*Vsnk) ||
         //                (std::abs(C_im[index_5d(rp,m,r,n,t, NsrcHex,B1Nrows,NsnkHex,Lt)] -  t_C_im[index_5d(rp,m,r,n,t, NsrcHex,B1Nrows,NsnkHex,Lt)]) >= 0.01*Vsnk*Vsnk))
         //             {
         //                printf("rp=%d, m=%d, n=%d, t=%d: %4.1f + I (%4.1f) vs %4.1f + I (%4.1f) \n", rp, m, n, t, C_re[index_5d(rp,m,r,n,t, NsrcHex,B1Nrows,NsnkHex,Lt)], C_im[index_5d(rp,m,r,n,t, NsrcHex,B1Nrows,NsnkHex,Lt)],  t_C_re[index_5d(rp,m,r,n,t, NsrcHex,B1Nrows,NsnkHex,Lt)],  t_C_im[index_5d(rp,m,r,n,t, NsrcHex,B1Nrows,NsnkHex,Lt)]);
         // 	            std::cout << "Error: different computed values for C_r or C_i!" << std::endl;
         // 	            exit(1);
         //             }
         //          }
         // }
         std::ofstream legality_check_fs("legality_check.txt", std::ofstream::app);

         std::fstream schedule_nb_fs;
         int index = 0;
         std::string tp;
         schedule_nb_fs.open("/data/cs7214/tiramisu/benchmarks/tensors/baryon/tiramisu_make_fused_baryon_blocks_correlator/index.txt", std::ios::in);
         if (schedule_nb_fs.is_open())
         {
            while (getline(schedule_nb_fs, tp))
            {
               index = std::stoi(tp);
               index++;
            }
            schedule_nb_fs.close();
         }

         std::ofstream ofs;

         ofs.open("/data/cs7214/tiramisu/benchmarks/tensors/baryon/tiramisu_make_fused_baryon_blocks_correlator/index.txt", std::ofstream::out);

         if (ofs.is_open())
         {
            ofs << std::to_string(index) << "\n";
            ofs.close();
         }

         bool break_flag = false;
         for (rp = 0; rp < B1Nrows; rp++)
         {
            if (break_flag == true)
               break;
            for (m = 0; m < NsrcHex; m++)
            {
               if (break_flag == true)
                  break;
               for (r = 0; r < B1Nrows; r++)
               {
                  if (break_flag == true)
                     break;
                  for (n = 0; n < NsnkHex; n++)
                  {
                     if (break_flag == true)
                        break;

                     for (t = 0; t < Lt; t++)
                     {
                        double diff = std::sqrt(std::pow(std::abs(C_re[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)] - t_C_re[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)]), 2) + std::pow(std::abs(C_im[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)] - t_C_im[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)]), 2));
                        double mag = std::sqrt(std::pow(std::abs(C_re[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)]), 2) + std::pow(std::abs(C_im[index_5d(rp, m, r, n, t, NsrcHex, B1Nrows, NsnkHex, Lt)]), 2));
                        if (diff / mag >= 1 / 1e12)
                        {
                           if (legality_check_fs.is_open())
                           {
                              legality_check_fs << "\n Schedule number: ";
                              legality_check_fs << std::to_string(index);
                              legality_check_fs << " is not legal";
                              break_flag = true;
                              break;
                           }
                        }
                     }
                  }
               }
            }
         }

         legality_check_fs.close();

#endif
      }

#ifdef WITH_MPI
      tiramisu_MPI_cleanup();
#endif

      return 0;
   }

#ifdef __cplusplus
} // extern "C"
#endif
