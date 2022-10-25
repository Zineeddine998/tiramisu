#include <stdio.h>
#include <stdlib.h>
#include "configure.h"
#include <time.h>
#include "mkl_dnn.h"

#define CHECK_ERR(f, err)                                          \
    do                                                             \
    {                                                              \
        (err) = (f);                                               \
        if ((err) != E_SUCCESS)                                    \
        {                                                          \
            printf("[%s:%d] err (%d)\n", __FILE__, __LINE__, err); \
            goto bail_out;                                         \
        }                                                          \
    } while (0)

#define dimension (4)

static dnnError_t init_conversion(dnnPrimitive_t *cv, float **ptr_out,
                                  dnnLayout_t lt_pr, dnnLayout_t lt_us, float *ptr_us)
{
    dnnError_t err;
    *ptr_out = NULL;
    if (!dnnLayoutCompare_F32(lt_pr, lt_us))
    {
        CHECK_ERR(dnnConversionCreate_F32(cv, lt_us, lt_pr), err);
        CHECK_ERR(dnnAllocateBuffer_F32((void **)ptr_out, lt_pr), err);
    }
    else
    {
        *ptr_out = ptr_us;
    }
    return E_SUCCESS;

bail_out:
    if (*ptr_out)
        dnnReleaseBuffer_F32(*ptr_out);
    return err;
}

static dnnError_t simple_net()
{
    dnnError_t err;

    size_t inputSize[dimension] = {N + 2, N + 2, FIn, BATCH_SIZE};
    size_t inputStrides[dimension] = {1, N + 2, (N + 2) * (N + 2), (N + 2) * (N + 2) * FIn};

    size_t output1Size[dimension] = {N, N, FOut, BATCH_SIZE};
    size_t output2Size[dimension] = {N, N, FOut, BATCH_SIZE};
    
    size_t filter1Size[dimension] = {K, K, FIn, FOut};
    size_t filter1Strides[dimension] = {1, K, K * K, K * K * FIn};

    size_t filter2Size[dimension] = {K, K, FOut, FOut};
    size_t filter2Strides[dimension] = {1, K, K * K, K * K * FOut};

    size_t convolutionStride[dimension - 2] = {1, 1};
    int input1Offset[dimension - 2] = {0, 0};
    int input2Offset[dimension - 2] = {-1, -1};

    size_t biasSize[1] = {FOut};
    size_t biasStrides[1] = {1};

    size_t pool_outputSize[dimension] = {N/2, N/2, FOut, BATCH_SIZE};
    size_t pool_outputStrides[dimension] = {1, N/2, N/2 * N/2, N/2 * N/2 * FOut};
    
    size_t pool_kernelSize[2] = {2, 2};
    size_t pool_kernelStride[2] = {2, 2};
    int pool_offset[2] = {0, 0};

    dnnLayout_t lt_user_input = NULL,
                lt_user_filt1 = NULL,
                lt_user_output = NULL,
                lt_user_filt2 = NULL;

    dnnLayout_t lt_conv1_input = NULL,
                lt_conv1_filt = NULL,
                lt_conv1_output = NULL;

    dnnPrimitive_t conv1 = NULL,
                   cv_user_to_conv1_input = NULL,
                   cv_user_to_conv1_filt = NULL;

    dnnLayout_t lt_conv2_filt = NULL,
                lt_conv2_output = NULL;

    dnnPrimitive_t conv2 = NULL,
                   cv_user_to_conv2_filt = NULL;

    dnnPrimitive_t relu1 = NULL,
                   relu2 = NULL;

    dnnLayout_t lt_pool1_input = NULL;
    dnnPrimitive_t pool1 = NULL,
                   cv_pool1_to_user_output = NULL;
    
    dnnLayout_t lt_pool1_output = NULL,
                lt_pool1_workspace = NULL;

    dnnPrimitiveAttributes_t attributes = NULL;

    float *resConv1[dnnResourceNumber] = {0};
    float *resConv2[dnnResourceNumber] = {0};
    float *resRelu1[dnnResourceNumber] = {0};
    float *resRelu2[dnnResourceNumber] = {0};
    void *resPool1[dnnResourceNumber] = {0};

    float *user_i = NULL,
          *user_c1_f = NULL,
          *user_c1_b = NULL,
          *user_c2_f = NULL,
          *user_c2_b = NULL,
          *user_o = NULL;

    /*** Data allocation ***/
    user_i = (float *)malloc(sizeof(float) * (inputSize[0] * inputSize[1] * inputSize[2] * inputSize[3]));
    user_c1_f = (float *)malloc(sizeof(float) * (filter1Size[0] * filter1Size[1] * filter1Size[2] * filter1Size[3]));
    user_c2_f = (float *)malloc(sizeof(float) * (filter2Size[0] * filter2Size[1] * filter2Size[2] * filter2Size[3]));
    user_c1_b = (float *)malloc(sizeof(float) * (output1Size[2]));
    user_c2_b = (float *)malloc(sizeof(float) * (output2Size[2]));

    if (user_i == NULL || user_c1_f == NULL || user_c2_f == NULL || user_c1_b == NULL) {
        err = E_MEMORY_ERROR;
        goto bail_out;
    }

    /*** User's data description ***/
    CHECK_ERR(dnnLayoutCreate_F32(&lt_user_input, dimension, inputSize, inputStrides), err);
    CHECK_ERR(dnnLayoutCreate_F32(&lt_user_filt1, dimension, filter1Size, filter1Strides), err);
    CHECK_ERR(dnnLayoutCreate_F32(&lt_user_output, dimension, pool_outputSize, pool_outputStrides), err);
    CHECK_ERR(dnnLayoutCreate_F32(&lt_user_filt2, dimension, filter2Size, filter2Strides), err);

    /* Initialize attributes */
    CHECK_ERR(dnnPrimitiveAttributesCreate_F32(&attributes), err);

    /*** Convolution section ***/
    CHECK_ERR(dnnConvolutionCreateForwardBias_F32(&conv1, attributes,
                                                  dnnAlgorithmConvolutionDirect, dimension, inputSize,
                                                  output1Size, filter1Size, convolutionStride, input1Offset,
                                                  dnnBorderZeros),
              err);

    // Convolution describes what layout it expects
    CHECK_ERR(dnnLayoutCreateFromPrimitive_F32(&lt_conv1_input, conv1, dnnResourceSrc), err);
    CHECK_ERR(dnnLayoutCreateFromPrimitive_F32(&lt_conv1_filt, conv1, dnnResourceFilter), err);
    CHECK_ERR(dnnLayoutCreateFromPrimitive_F32(&lt_conv1_output, conv1, dnnResourceDst), err);

    CHECK_ERR(init_conversion(&cv_user_to_conv1_input, &resConv1[dnnResourceSrc], lt_conv1_input, lt_user_input, user_i), err);
    CHECK_ERR(init_conversion(&cv_user_to_conv1_filt, &resConv1[dnnResourceFilter], lt_conv1_filt, lt_user_filt1, user_c1_f), err);
    CHECK_ERR(dnnAllocateBuffer_F32((void **)&resConv1[dnnResourceDst], lt_conv1_output), err);

    resConv1[dnnResourceBias] = user_c1_b;

    /*** ReLU section ***/
    CHECK_ERR(dnnReLUCreateForward_F32(&relu1, attributes, lt_conv1_output, 0.0f), err);
    resRelu1[dnnResourceSrc] = resConv1[dnnResourceDst];
    resRelu1[dnnResourceDst] = resRelu1[dnnResourceSrc];

    /*** Convolution 2 section ***/
    CHECK_ERR(dnnConvolutionCreateForwardBias_F32(&conv2, attributes,
                                                  dnnAlgorithmConvolutionDirect, dimension, output1Size,
                                                  output2Size, filter2Size, convolutionStride, input2Offset,
                                                  dnnBorderZeros),
              err);

    resConv2[dnnResourceSrc] = resRelu1[dnnResourceDst];
    resConv2[dnnResourceBias] = user_c2_b;

    // Convolution describes what layout it expects
    CHECK_ERR(dnnLayoutCreateFromPrimitive_F32(&lt_conv2_filt, conv2, dnnResourceFilter), err);
    CHECK_ERR(dnnLayoutCreateFromPrimitive_F32(&lt_conv2_output, conv2, dnnResourceDst), err);

    CHECK_ERR(init_conversion(&cv_user_to_conv2_filt, &resConv2[dnnResourceFilter], lt_conv2_filt, lt_user_filt2, user_c2_f), err);
    CHECK_ERR(dnnAllocateBuffer_F32((void **)&resConv2[dnnResourceDst], lt_conv2_output), err);

    /*** ReLU 2 section ***/
    CHECK_ERR(dnnReLUCreateForward_F32(&relu2, attributes, lt_conv2_output, 0.0f), err);
    resRelu2[dnnResourceSrc] = resConv2[dnnResourceDst];
    resRelu2[dnnResourceDst] = resRelu2[dnnResourceSrc];

    /*** Pooling section ***/
    lt_pool1_input = lt_conv2_output;
    CHECK_ERR(dnnPoolingCreateForward_F32(&pool1, attributes, dnnAlgorithmPoolingMax, lt_pool1_input, pool_kernelSize, pool_kernelStride, pool_offset, dnnBorderZeros), err);
    resPool1[dnnResourceSrc] = resRelu2[dnnResourceDst];

    CHECK_ERR(dnnLayoutCreateFromPrimitive_F32(&lt_pool1_output, pool1, dnnResourceDst), err);
    CHECK_ERR(dnnLayoutCreateFromPrimitive_F32(&lt_pool1_workspace, pool1, dnnResourceWorkspace), err);
    CHECK_ERR(dnnAllocateBuffer_F32(&resPool1[dnnResourceDst], lt_pool1_output), err);
    CHECK_ERR(dnnAllocateBuffer_F32(&resPool1[dnnResourceWorkspace], lt_pool1_workspace), err);

    CHECK_ERR(init_conversion(&cv_pool1_to_user_output, &user_o, lt_user_output, lt_pool1_output, resPool1[dnnResourceDst]), err);

    srand(1);

    /*** Init user buffers ***/
    for (int i = 0; i < inputSize[0] * inputSize[1] * inputSize[2] * inputSize[3]; i++)
        user_i[i] = (rand() % 200 - 100) / 100.;
        
    for (int i = 0; i < output1Size[2]; i++)
        user_c1_b[i] = (rand() % 200 - 100) / 100.;
    for (int i = 0; i < filter1Size[0] * filter1Size[1] * filter1Size[2] * filter1Size[3]; i++)
        user_c1_f[i] = (rand() % 200 - 100) / 100.;

    for (int i = 0; i < output2Size[2]; i++)
        user_c2_b[i] = (rand() % 200 - 100) / 100.;
    for (int i = 0; i < filter2Size[0] * filter2Size[1] * filter2Size[2] * filter2Size[3]; i++)
        user_c2_f[i] = (rand() % 200 - 100) / 100.;

    /*** Execution ***/
    if (cv_user_to_conv1_filt)
        CHECK_ERR(dnnConversionExecute_F32(cv_user_to_conv1_filt, user_c1_f, resConv1[dnnResourceFilter]), err);
    if (cv_user_to_conv2_filt)
        CHECK_ERR(dnnConversionExecute_F32(cv_user_to_conv2_filt, user_c2_f, resConv2[dnnResourceFilter]), err);
    if (cv_user_to_conv1_input)
        CHECK_ERR(dnnConversionExecute_F32(cv_user_to_conv1_input, user_i, resConv1[dnnResourceSrc]), err);
    
    double times[NB_TESTS];
    double start, end;

    for (int i = 0; i < NB_TESTS; i++) {
        start = rtclock();

        CHECK_ERR(dnnExecute_F32(conv1, (void *)resConv1), err);
        CHECK_ERR(dnnExecute_F32(relu1, (void *)resRelu1), err);
        CHECK_ERR(dnnExecute_F32(conv2, (void *)resConv2), err);
        CHECK_ERR(dnnExecute_F32(relu2, (void *)resRelu2), err);
        CHECK_ERR(dnnExecute_F32(pool1, (void *)resPool1), err);

        end = rtclock();
        times[i] = (end - start) * 1000;
    }
    
    printf("\n\n\tVGG block time: %f ms.\n", median(NB_TESTS, times));

    if (cv_pool1_to_user_output)
        CHECK_ERR(dnnConversionExecute_F32(cv_pool1_to_user_output, resPool1[dnnResourceDst], user_o), err);

    FILE *f = fopen("mkl_result.txt", "w");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    for (int n = 0; n < BATCH_SIZE; ++n)
        for (int z = 0; z < FOut; ++z)
            for (int y = 0; y < N/2; ++y)
                for (int x = 0; x < N/2; ++x)
                    fprintf(f, "%.10g\n", user_o[x + y * N/2 + z * N/2 * N/2 + n * N/2 * N/2 * FOut]);

    fclose(f);

bail_out:

    dnnDelete_F32(conv1);
    dnnDelete_F32(relu1);
    dnnDelete_F32(conv2);
    dnnDelete_F32(pool1);
    dnnDelete_F32(cv_user_to_conv1_input);
    dnnDelete_F32(cv_user_to_conv1_filt);
    dnnDelete_F32(cv_user_to_conv2_filt);
    dnnDelete_F32(cv_pool1_to_user_output);

    dnnLayoutDelete_F32(lt_user_input);
    dnnLayoutDelete_F32(lt_user_filt1);
    dnnLayoutDelete_F32(lt_user_filt2);
    dnnLayoutDelete_F32(lt_user_output);
    dnnLayoutDelete_F32(lt_conv1_input);
    dnnLayoutDelete_F32(lt_conv1_filt);
    dnnLayoutDelete_F32(lt_conv1_output);
    dnnLayoutDelete_F32(lt_conv2_filt);
    dnnLayoutDelete_F32(lt_conv2_output);
    dnnLayoutDelete_F32(lt_pool1_output);
    dnnLayoutDelete_F32(lt_pool1_workspace);

    dnnPrimitiveAttributesDestroy_F32(attributes);

    if (resConv1[dnnResourceSrc] != (void *)user_i)
        dnnReleaseBuffer_F32(resConv1[dnnResourceSrc]);

    if (resConv1[dnnResourceFilter] != (void *)user_c1_f)
        dnnReleaseBuffer_F32(resConv1[dnnResourceFilter]);

    if (resConv1[dnnResourceBias] != (void *)user_c1_b)
        dnnReleaseBuffer_F32(resConv1[dnnResourceBias]);

    dnnReleaseBuffer_F32(resConv1[dnnResourceDst]);

    if (resConv2[dnnResourceFilter] != (void *)user_c2_f)
        dnnReleaseBuffer_F32(resConv2[dnnResourceFilter]);

    dnnReleaseBuffer_F32(resConv2[dnnResourceDst]);
    dnnReleaseBuffer_F32(resPool1[dnnResourceDst]);
    dnnReleaseBuffer_F32(resPool1[dnnResourceWorkspace]);

    if ((void *)user_o != resPool1[dnnResourceDst])
        dnnReleaseBuffer_F32((void *)user_o);

    free(user_i);
    free(user_c1_f);
    free(user_c1_b);

    return err;
}

int main(int argc, char **argv)
{
    dnnError_t err;
    err = simple_net();
    if (err != E_SUCCESS)
    {
        printf("FAILED\n");
        return err;
    }

    printf("PASSED\n");
    return 0;
}
