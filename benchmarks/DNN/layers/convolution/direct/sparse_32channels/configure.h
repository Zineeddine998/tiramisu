#ifndef __SPCONV_CONF_HEADER_
#define __SPCONV_CONF_HEADER_

#include <sys/time.h>

#define IMPORT_CSR_FROM_FILE 1
#define SHOW_OUTPUT 0
#define WRITE_RESULT_TO_FILE 1
#define CHECK_CORRECTNESS 1

#define LARGE_DATA_SET	0
#define MEDIUM_DATA_SET	1
#define SMALL_DATA_SET	0

#define LARGE_N 1
#define MEDIUM_N 0
#define SMALL_N 0

#if LARGE_DATA_SET
	#define BATCH_SIZE 100
#elif MEDIUM_DATA_SET
	#define BATCH_SIZE 32
#elif SMALL_DATA_SET
	#define BATCH_SIZE 8
#endif

// Size of one data dimension
#if LARGE_N
	#define N 224
#elif MEDIUM_N
	#define N 112
#elif SMALL_N
	#define N 56
#endif

// Number of features in the input
#define FIn 32
#define FIN_BL 16 // Used only when IMPORT_CSR_FROM_FILE is at 0, otherwise, the blocked traversal is generated via the python script that exports the layer in CSR format

// Number of features in the output
#define FOut 32
#define FOUT_BL 16
#define FOUT_NB_BL (FOut/ FOUT_BL)

// Size of convolution filter (KxK)
#define K 3

#define WEIGHTS_DENSITY 0.15

#define NB_TESTS 201

// Parameters to tune
#define X_BL 32
#define X_NB_BL (N/X_BL)

#define Y_BL 2
#define Y_NB_BL (N/Y_BL)

#ifdef __cplusplus
double median(std::vector<double> scores)
{
    double median;
    size_t size = scores.size();

    sort(scores.begin(), scores.end());

    if (size % 2 == 0)
    {
        median = (scores[size / 2 - 1] + scores[size / 2]) / 2;
    }
    else
    {
        median = scores[size / 2];
    }

    return median;
}
#else
double median(int n, double x[])
{
    double temp;
    int i, j;

    // the following two loops sort the array x in ascending order
    for(i=0; i<n-1; i++) {
        for(j=i+1; j<n; j++) {
            if(x[j] < x[i]) {
                // swap elements
                temp = x[i];
                x[i] = x[j];
                x[j] = temp;
            }
        }
    }

    if(n%2==0) {
        // if there is an even number of elements, return mean of the two elements in the middle
        return((x[n/2] + x[n/2 - 1]) / 2.0);
    } else {
        // else return the element in the middle
        return x[n/2];
    }
}
#endif

double rtclock()
{
    struct timeval Tp;
    gettimeofday(&Tp, NULL);

    return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}

#endif
