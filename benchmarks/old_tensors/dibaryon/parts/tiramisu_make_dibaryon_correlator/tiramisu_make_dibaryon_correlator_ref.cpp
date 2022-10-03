#include <tiramisu/utils.h>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <complex.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

/* index functions */
int index_2d(int a, int b, int length2) {
   return b +length2*( a );
}
int index_3d(int a, int b, int c, int length2, int length3) {
   return c +length3*( b +length2*( a ));
}
int index_4d(int a, int b, int c, int d, int length2, int length3, int length4) {
   return d +length4*( c +length3*( b +length2*( a )));
}
int prop_index(int q, int t, int c1, int s1, int c2, int s2, int y, int x) {
   return x +Vsnk*( y +Vsrc*( c2 +Nc*( s2 +Ns*( c1 +Nc*( s1 +Ns*( t +Nt* q ))))));
}
int Q_index(int t, int c1, int s1, int c2, int s2, int x1, int c3, int s3, int y) {
   return y +Vsrc*( s3 +Ns*( c3 +Nc*( x1 +Vsnk*( s2 +Ns*( c2 +Nc*( s1 +Ns*( c1 +Nc*( t ))))))));
}
int Blocal_index(int t, int c1, int s1, int c2, int s2, int x, int c3, int s3, int m) {
   return m +Nsrc*( s3 +Ns*( c3 +Nc*( x +Vsnk*( s2 +Ns*( c2 +Nc*( s1 +Ns*( c1 +Nc*( t ))))))));
}
int Bdouble_index(int t, int c1, int s1, int c2, int s2, int x1, int c3, int s3, int x2, int m) {
   return m +Nsrc*( x2 +Vsnk*(  s3 +Ns*( c3 +Nc*( x1 +Vsnk*( s2 +Ns*( c2 +Nc*( s1 +Ns*( c1 +Nc*( t )))))))));
}

void make_dibaryon_correlator(double* C_re,
    double* C_im,
    const double* B1_Blocal_re, 
    const double* B1_Blocal_im, 
    const double* B1_Bsingle_re, 
    const double* B1_Bsingle_im, 
    const double* B1_Bdouble_re, 
    const double* B1_Bdouble_im, 
    const double* B2_Blocal_re, 
    const double* B2_Blocal_im, 
    const double* B2_Bsingle_re, 
    const double* B2_Bsingle_im, 
    const double* B2_Bdouble_re, 
    const double* B2_Bdouble_im, 
    const int* perms, 
    const int* sigs, 
    const double overall_weight,
    const int* snk_color_weights, 
    const int* snk_spin_weights, 
    const double* snk_weights, 
    const double* snk_psi_re,
    const double* snk_psi_im) {
   /* indices */
   int iC1,iS1,jC1,jS1,kC1,kS1,iC2,iS2,jC2,jS2,kC2,kS2,x1,x2,t,wnum,nperm,b,n,m;
   double term_re, term_im, new_term_re, new_term_im;
   /* build dibaryon */
   int snk_1_nq[Nb];
   int snk_2_nq[Nb];
   int snk_3_nq[Nb];
   int snk_1_b[Nb];
   int snk_2_b[Nb];
   int snk_3_b[Nb];
   int snk_1[Nb];
   int snk_2[Nb];
   int snk_3[Nb];
   for (nperm=0; nperm<Nperms; nperm++) {
      for (b=0; b<Nb; b++) {
         snk_1[b] = perms[index_2d(nperm,Nq*b+0 ,2*Nq)] - 1;
         snk_2[b] = perms[index_2d(nperm,Nq*b+1 ,2*Nq)] - 1;
         snk_3[b] = perms[index_2d(nperm,Nq*b+2 ,2*Nq)] - 1;
         snk_1_b[b] = (snk_1[b] - snk_1[b] % Nq) / Nq;
         snk_2_b[b] = (snk_2[b] - snk_2[b] % Nq) / Nq;
         snk_3_b[b] = (snk_3[b] - snk_3[b] % Nq) / Nq;
         snk_1_nq[b] = snk_1[b] % Nq;
         snk_2_nq[b] = snk_2[b] % Nq;
         snk_3_nq[b] = snk_3[b] % Nq;
      }
      for (wnum=0; wnum< Nw2; wnum++) {
         iC1 = snk_color_weights[index_3d(snk_1_b[0],wnum,snk_1_nq[0] ,Nw2,Nq)];
         iS1 = snk_spin_weights[index_3d(snk_1_b[0],wnum,snk_1_nq[0] ,Nw2,Nq)];
         jC1 = snk_color_weights[index_3d(snk_2_b[0],wnum,snk_2_nq[0] ,Nw2,Nq)];
         jS1 = snk_spin_weights[index_3d(snk_2_b[0],wnum,snk_2_nq[0] ,Nw2,Nq)];
         kC1 = snk_color_weights[index_3d(snk_3_b[0],wnum,snk_3_nq[0] ,Nw2,Nq)];
         kS1 = snk_spin_weights[index_3d(snk_3_b[0],wnum,snk_3_nq[0] ,Nw2,Nq)];
         iC2 = snk_color_weights[index_3d(snk_1_b[1],wnum,snk_1_nq[1] ,Nw2,Nq)];
         iS2 = snk_spin_weights[index_3d(snk_1_b[1],wnum,snk_1_nq[1] ,Nw2,Nq)];
         jC2 = snk_color_weights[index_3d(snk_2_b[1],wnum,snk_2_nq[1] ,Nw2,Nq)];
         jS2 = snk_spin_weights[index_3d(snk_2_b[1],wnum,snk_2_nq[1] ,Nw2,Nq)];
         kC2 = snk_color_weights[index_3d(snk_3_b[1],wnum,snk_3_nq[1] ,Nw2,Nq)];
         kS2 = snk_spin_weights[index_3d(snk_3_b[1],wnum,snk_3_nq[1] ,Nw2,Nq)]; 
         for (t=0; t<Nt; t++) {
            for (x1=0; x1<Vsnk; x1++) {
               for (x2=0; x2<Vsnk; x2++) {
                  for (m=0; m<Nsrc; m++) {
                     term_re = sigs[nperm] * overall_weight * snk_weights[wnum];
                     term_im = 0.0;
                     for (b=0; b<Nb; b++) {
                        if ((snk_1_b[b] == 0) && (snk_2_b[b] == 0) && (snk_3_b[b] == 0)) {
                           new_term_re = term_re * B1_Blocal_re[Blocal_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,m)] - term_im * B1_Blocal_im[Blocal_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,m)];
                           new_term_im = term_re * B1_Blocal_im[Blocal_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,m)] + term_im * B1_Blocal_re[Blocal_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,m)];
                        }
                        else if ((snk_1_b[b] == 1) && (snk_2_b[b] == 1) && (snk_3_b[b] == 1)) {
                           new_term_re = term_re * B2_Blocal_re[Blocal_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,m)] - term_im * B2_Blocal_im[Blocal_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,m)];
                           new_term_im = term_re * B2_Blocal_im[Blocal_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,m)] + term_im * B2_Blocal_re[Blocal_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,m)];
                        }
                        else if ((snk_1_b[b] == 0) && (snk_3_b[b] == 0)) {
                           new_term_re = term_re * B1_Bsingle_re[Bdouble_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,x2,m)] - term_im * B1_Bsingle_im[Bdouble_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,x2,m)];
                           new_term_im = term_re * B1_Bsingle_im[Bdouble_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,x2,m)] + term_im * B1_Bsingle_re[Bdouble_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,x2,m)];
                        }
                        else if ((snk_1_b[b] == 1) && (snk_3_b[b] == 1)) {
                           new_term_re = term_re * B2_Bsingle_re[Bdouble_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,x1,m)] - term_im * B2_Bsingle_im[Bdouble_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,x1,m)];
                           new_term_im = term_re * B2_Bsingle_im[Bdouble_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,x1,m)] + term_im * B2_Bsingle_re[Bdouble_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,x1,m)];
                        }
                        else if (((snk_1_b[b] == 0) && (snk_2_b[b] == 0)) || ((snk_2_b[b] == 0) && (snk_3_b[b] == 0))) {
                           new_term_re = term_re * B1_Bdouble_re[Bdouble_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,x2,m)] - term_im * B1_Bdouble_im[Bdouble_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,x2,m)];
                           new_term_im = term_re * B1_Bdouble_im[Bdouble_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,x2,m)] + term_im * B1_Bdouble_re[Bdouble_index(t,iC1,iS1,kC1,kS1,x1,jC1,jS1,x2,m)];
                        }
                        else if (((snk_1_b[b] == 1) && (snk_2_b[b] == 1)) || ((snk_2_b[b] == 1) && (snk_3_b[b] == 1))) {
                           new_term_re = term_re * B2_Bdouble_re[Bdouble_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,x1,m)] - term_im * B2_Bdouble_im[Bdouble_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,x1,m)];
                           new_term_im = term_re * B2_Bdouble_im[Bdouble_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,x1,m)] + term_im * B2_Bdouble_re[Bdouble_index(t,iC2,iS2,kC2,kS2,x2,jC2,jS2,x1,m)];
                        }
                        term_re = new_term_re;
                        term_im = new_term_im;
                     }
                     for (n=0; n<Nsnk; n++) {
                        C_re[index_3d(m,n,t,Nsnk,Nt)] += snk_psi_re[index_3d(x1,x2,n ,Vsnk,Nsnk)] * term_re - snk_psi_im[index_3d(x1,x2,n ,Vsnk,Nsnk)] * term_im;
                        C_im[index_3d(m,n,t,Nsnk,Nt)] += snk_psi_re[index_3d(x1,x2,n ,Vsnk,Nsnk)] * term_im + snk_psi_im[index_3d(x1,x2,n ,Vsnk,Nsnk)] * term_re;
                     }
                  }
               }
            }
         }
      }
   } 
}
