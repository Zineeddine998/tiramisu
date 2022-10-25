#include <cudnn.h>
#include <cuda.h>
#include <stdio.h>

#define cudaErrCheck(stat) { cudaErrCheck_((stat), __FILE__, __LINE__); }
void cudaErrCheck_(cudaError_t stat, const char *file, int line) {
   if (stat != cudaSuccess) {
      fprintf(stderr, "CUDA Error: %s %s %d\n", cudaGetErrorString(stat), file, line);
   }
}

#define cudnnErrCheck(stat) { cudnnErrCheck_((stat), __FILE__, __LINE__); }
void cudnnErrCheck_(cudnnStatus_t stat, const char *file, int line) {
   if (stat != CUDNN_STATUS_SUCCESS) {
      fprintf(stderr, "cuDNN Error: %s %s %d\n", cudnnGetErrorString(stat), file, line);
   }
}

// Global variables 1st LSTMs
int seqLength;

void *x;
void *hx = NULL;
void *cx = NULL;

void *y;
void *hy = NULL;
void *cy = NULL;

void *w;

cudnnHandle_t cudnnHandle;
cudnnRNNDescriptor_t rnnDesc;
cudnnTensorDescriptor_t *xDesc, *yDesc;
cudnnTensorDescriptor_t hxDesc, cxDesc;
cudnnTensorDescriptor_t hyDesc, cyDesc;
cudnnFilterDescriptor_t wDesc;

void *workspace;
size_t workSize;

// Global variables 2nd LSTMs
int seqLength2;

void *x2;
void *hx2 = NULL;
void *cx2 = NULL;

void *y2;
void *hy2 = NULL;
void *cy2 = NULL;

void *w2;

cudnnHandle_t cudnnHandle2;
cudnnRNNDescriptor_t rnnDesc2;
cudnnTensorDescriptor_t *xDesc2, *yDesc2;
cudnnTensorDescriptor_t hxDesc2, cxDesc2;
cudnnTensorDescriptor_t hyDesc2, cyDesc2;
cudnnFilterDescriptor_t wDesc2;

void *workspace2;
size_t workSize2;

void setup_cudnn(int _seqLength, int numLayers, int batch_size, int feature_size) {
    seqLength = _seqLength;
    int hiddenSize = feature_size;
    int inputSize = hiddenSize;
    int miniBatch = batch_size;

    // -------------------------
    // Create cudnn context
    // -------------------------
    cudnnErrCheck(cudnnCreate(&cudnnHandle));

    // -------------------------
    // Set up inputs and outputs
    // -------------------------
    cudaErrCheck(cudaMalloc((void**)&x, seqLength * inputSize * miniBatch * sizeof(DATA_TYPE)));
    cudaErrCheck(cudaMalloc((void**)&y, seqLength * hiddenSize * miniBatch * sizeof(DATA_TYPE)));

    xDesc = (cudnnTensorDescriptor_t*)malloc(seqLength * sizeof(cudnnTensorDescriptor_t));
    yDesc = (cudnnTensorDescriptor_t*)malloc(seqLength * sizeof(cudnnTensorDescriptor_t));

    int dimA[3];
    int strideA[3];

    for (int i = 0; i < seqLength; i++) {
        cudnnErrCheck(cudnnCreateTensorDescriptor(&xDesc[i]));
        cudnnErrCheck(cudnnCreateTensorDescriptor(&yDesc[i]));

        dimA[0] = miniBatch;
        dimA[1] = inputSize;
        dimA[2] = 1;

        strideA[0] = dimA[2] * dimA[1];
        strideA[1] = dimA[2];
        strideA[2] = 1;

        cudnnErrCheck(cudnnSetTensorNdDescriptor(xDesc[i], DATA_TYPE_CUDNN, 3, dimA, strideA));

        dimA[0] = miniBatch;
        dimA[1] = hiddenSize;
        dimA[2] = 1;

        strideA[0] = dimA[2] * dimA[1];
        strideA[1] = dimA[2];
        strideA[2] = 1;

        cudnnErrCheck(cudnnSetTensorNdDescriptor(yDesc[i], DATA_TYPE_CUDNN, 3, dimA, strideA));
    }


    dimA[0] = numLayers;
    dimA[1] = miniBatch;
    dimA[2] = hiddenSize;

    strideA[0] = dimA[2] * dimA[1];
    strideA[1] = dimA[2];
    strideA[2] = 1;

    cudnnErrCheck(cudnnCreateTensorDescriptor(&hxDesc));
    cudnnErrCheck(cudnnCreateTensorDescriptor(&cxDesc));
    cudnnErrCheck(cudnnCreateTensorDescriptor(&hyDesc));
    cudnnErrCheck(cudnnCreateTensorDescriptor(&cyDesc));

    cudnnErrCheck(cudnnSetTensorNdDescriptor(hxDesc, DATA_TYPE_CUDNN, 3, dimA, strideA));
    cudnnErrCheck(cudnnSetTensorNdDescriptor(cxDesc, DATA_TYPE_CUDNN, 3, dimA, strideA));
    cudnnErrCheck(cudnnSetTensorNdDescriptor(hyDesc, DATA_TYPE_CUDNN, 3, dimA, strideA));
    cudnnErrCheck(cudnnSetTensorNdDescriptor(cyDesc, DATA_TYPE_CUDNN, 3, dimA, strideA));

    // -------------------------
    // Set up the dropout descriptor (needed for the RNN descriptor)
    // -------------------------

    cudnnDropoutDescriptor_t dropoutDesc;
    cudnnErrCheck(cudnnCreateDropoutDescriptor(&dropoutDesc));

    size_t stateSize;
    void *states;
    cudnnErrCheck(cudnnDropoutGetStatesSize(cudnnHandle, &stateSize));

    cudaErrCheck(cudaMalloc(&states, stateSize));

    cudnnErrCheck(cudnnSetDropoutDescriptor(dropoutDesc,
                cudnnHandle,
                0,
                states,
                stateSize,
                0));

    // -------------------------
    // Set up the RNN descriptor
    // -------------------------
    cudnnRNNMode_t RNNMode;
    cudnnRNNAlgo_t RNNAlgo;

    cudnnErrCheck(cudnnCreateRNNDescriptor(&rnnDesc));

    RNNMode = CUDNN_LSTM;

    RNNAlgo = CUDNN_RNN_ALGO_STANDARD;

    cudnnErrCheck(cudnnSetRNNDescriptor_v6(cudnnHandle,
                rnnDesc,
                hiddenSize,
                numLayers,
                dropoutDesc,
                CUDNN_LINEAR_INPUT,
                CUDNN_UNIDIRECTIONAL,
                RNNMode,
                RNNAlgo,
                DATA_TYPE_CUDNN));


    // -------------------------
    // Set up parameters
    // -------------------------
    // This needs to be done after the rnn descriptor is set as otherwise
    // we don't know how many parameters we have to allocate

    cudnnErrCheck(cudnnCreateFilterDescriptor(&wDesc));

    size_t weightsSize;
    cudnnErrCheck(cudnnGetRNNParamsSize(cudnnHandle, rnnDesc, xDesc[0], &weightsSize, DATA_TYPE_CUDNN));

    int dimW[3];
    dimW[0] = weightsSize / sizeof(DATA_TYPE);
    dimW[1] = 1;
    dimW[2] = 1;

    cudnnErrCheck(cudnnSetFilterNdDescriptor(wDesc, DATA_TYPE_CUDNN, CUDNN_TENSOR_NCHW, 3, dimW));

    cudaErrCheck(cudaMalloc((void**)&w,  weightsSize));


    // -------------------------
    // Set up work space and reserved memory
    // -------------------------

    // Need for every pass
    cudnnErrCheck(cudnnGetRNNWorkspaceSize(cudnnHandle, rnnDesc, seqLength, xDesc, &workSize));
    cudaErrCheck(cudaMalloc((void**)&workspace, workSize));

}

void setup_cudnn2(int _seqLength, int numLayers, int batch_size, int feature_size) {
    seqLength2 = _seqLength;
    int hiddenSize = feature_size;
    int inputSize = hiddenSize;
    int miniBatch = batch_size;

    // -------------------------
    // Create cudnn context
    // -------------------------
    //cudnnErrCheck(cudnnCreate(&cudnnHandle2));

    // -------------------------
    // Set up inputs and outputs
    // -------------------------
    cudaErrCheck(cudaMalloc((void**)&x2, seqLength2 * inputSize * miniBatch * sizeof(DATA_TYPE)));
    cudaErrCheck(cudaMalloc((void**)&y2, seqLength2 * hiddenSize * miniBatch * sizeof(DATA_TYPE)));

    xDesc2 = (cudnnTensorDescriptor_t*)malloc(seqLength2 * sizeof(cudnnTensorDescriptor_t));
    yDesc2 = (cudnnTensorDescriptor_t*)malloc(seqLength2 * sizeof(cudnnTensorDescriptor_t));

    int dimA[3];
    int strideA[3];

    for (int i = 0; i < seqLength2; i++) {
        cudnnErrCheck(cudnnCreateTensorDescriptor(&xDesc2[i]));
        cudnnErrCheck(cudnnCreateTensorDescriptor(&yDesc2[i]));

        dimA[0] = miniBatch;
        dimA[1] = inputSize;
        dimA[2] = 1;

        strideA[0] = dimA[2] * dimA[1];
        strideA[1] = dimA[2];
        strideA[2] = 1;

        cudnnErrCheck(cudnnSetTensorNdDescriptor(xDesc2[i], DATA_TYPE_CUDNN, 3, dimA, strideA));

        dimA[0] = miniBatch;
        dimA[1] = hiddenSize;
        dimA[2] = 1;

        strideA[0] = dimA[2] * dimA[1];
        strideA[1] = dimA[2];
        strideA[2] = 1;

        cudnnErrCheck(cudnnSetTensorNdDescriptor(yDesc2[i], DATA_TYPE_CUDNN, 3, dimA, strideA));
    }


    dimA[0] = numLayers;
    dimA[1] = miniBatch;
    dimA[2] = hiddenSize;

    strideA[0] = dimA[2] * dimA[1];
    strideA[1] = dimA[2];
    strideA[2] = 1;

    cudnnErrCheck(cudnnCreateTensorDescriptor(&hxDesc2));
    cudnnErrCheck(cudnnCreateTensorDescriptor(&cxDesc2));
    cudnnErrCheck(cudnnCreateTensorDescriptor(&hyDesc2));
    cudnnErrCheck(cudnnCreateTensorDescriptor(&cyDesc2));

    cudnnErrCheck(cudnnSetTensorNdDescriptor(hxDesc2, DATA_TYPE_CUDNN, 3, dimA, strideA));
    cudnnErrCheck(cudnnSetTensorNdDescriptor(cxDesc2, DATA_TYPE_CUDNN, 3, dimA, strideA));
    cudnnErrCheck(cudnnSetTensorNdDescriptor(hyDesc2, DATA_TYPE_CUDNN, 3, dimA, strideA));
    cudnnErrCheck(cudnnSetTensorNdDescriptor(cyDesc2, DATA_TYPE_CUDNN, 3, dimA, strideA));

    // -------------------------
    // Set up the dropout descriptor (needed for the RNN descriptor)
    // -------------------------

    cudnnDropoutDescriptor_t dropoutDesc;
    cudnnErrCheck(cudnnCreateDropoutDescriptor(&dropoutDesc));

    size_t stateSize;
    void *states;
    cudnnErrCheck(cudnnDropoutGetStatesSize(cudnnHandle, &stateSize));

    cudaErrCheck(cudaMalloc(&states, stateSize));

    cudnnErrCheck(cudnnSetDropoutDescriptor(dropoutDesc,
                cudnnHandle,
                0,
                states,
                stateSize,
                0));

    // -------------------------
    // Set up the RNN descriptor
    // -------------------------
    cudnnRNNMode_t RNNMode;
    cudnnRNNAlgo_t RNNAlgo;

    cudnnErrCheck(cudnnCreateRNNDescriptor(&rnnDesc2));

    RNNMode = CUDNN_LSTM;

    RNNAlgo = CUDNN_RNN_ALGO_STANDARD;

    cudnnErrCheck(cudnnSetRNNDescriptor_v6(cudnnHandle,
                rnnDesc2,
                hiddenSize,
                numLayers,
                dropoutDesc,
                CUDNN_LINEAR_INPUT,
                CUDNN_UNIDIRECTIONAL,
                RNNMode,
                RNNAlgo,
                DATA_TYPE_CUDNN));


    // -------------------------
    // Set up parameters
    // -------------------------
    // This needs to be done after the rnn descriptor is set as otherwise
    // we don't know how many parameters we have to allocate

    cudnnErrCheck(cudnnCreateFilterDescriptor(&wDesc2));

    size_t weightsSize;
    cudnnErrCheck(cudnnGetRNNParamsSize(cudnnHandle, rnnDesc2, xDesc2[0], &weightsSize, DATA_TYPE_CUDNN));

    int dimW[3];
    dimW[0] = weightsSize / sizeof(DATA_TYPE);
    dimW[1] = 1;
    dimW[2] = 1;

    cudnnErrCheck(cudnnSetFilterNdDescriptor(wDesc2, DATA_TYPE_CUDNN, CUDNN_TENSOR_NCHW, 3, dimW));

    cudaErrCheck(cudaMalloc((void**)&w2,  weightsSize));


    // -------------------------
    // Set up work space and reserved memory
    // -------------------------

    // Need for every pass
    cudnnErrCheck(cudnnGetRNNWorkspaceSize(cudnnHandle, rnnDesc2, seqLength2, xDesc2, &workSize2));
    cudaErrCheck(cudaMalloc((void**)&workspace2, workSize2));

}

float run_cudnn(DATA_TYPE *raw_Weights, DATA_TYPE *raw_biases, DATA_TYPE *raw_x, DATA_TYPE *raw_y) {
    cudaErrCheck(cudaDeviceSynchronize());

    // Initialise inputs
    cudaMemcpy(x, raw_x, FEATURE_SIZE * BATCH_SIZE * SEQ_LENGTH * sizeof(DATA_TYPE),
               cudaMemcpyKind::cudaMemcpyHostToDevice);

    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int linLayerID = 0; linLayerID < 8; linLayerID++) {
            cudnnFilterDescriptor_t linLayerMatDesc;
            cudnnErrCheck(cudnnCreateFilterDescriptor(&linLayerMatDesc));
            DATA_TYPE *linLayerMat;

            cudnnErrCheck(cudnnGetRNNLinLayerMatrixParams(cudnnHandle,
                        rnnDesc,
                        layer,
                        xDesc[0],
                        wDesc,
                        w,
                        linLayerID,
                        linLayerMatDesc,
                        (void**)&linLayerMat));

            cudnnDataType_t dataType;
            cudnnTensorFormat_t format;
            int nbDims;
            int filterDimA[3];
            cudnnErrCheck(cudnnGetFilterNdDescriptor(linLayerMatDesc,
                        3,
                        &dataType,
                        &format,
                        &nbDims,
                        filterDimA));

            cudaMemcpy(linLayerMat, raw_Weights + FEATURE_SIZE * FEATURE_SIZE * (linLayerID + 8 * layer),
                       FEATURE_SIZE * FEATURE_SIZE * sizeof(DATA_TYPE), cudaMemcpyKind::cudaMemcpyHostToDevice);

            cudnnErrCheck(cudnnDestroyFilterDescriptor(linLayerMatDesc));

            cudnnFilterDescriptor_t linLayerBiasDesc;
            cudnnErrCheck(cudnnCreateFilterDescriptor(&linLayerBiasDesc));
            DATA_TYPE *linLayerBias;

            cudnnErrCheck(cudnnGetRNNLinLayerBiasParams(cudnnHandle,
                        rnnDesc,
                        layer,
                        xDesc[0],
                        wDesc,
                        w,
                        linLayerID,
                        linLayerBiasDesc,
                        (void**)&linLayerBias));

            cudnnErrCheck(cudnnGetFilterNdDescriptor(linLayerBiasDesc,
                        3,
                        &dataType,
                        &format,
                        &nbDims,
                        filterDimA));

            // We use merged bias
            if (linLayerID < 4) {
                cudaMemcpy(linLayerBias,
                           raw_biases + FEATURE_SIZE * (linLayerID + 4 * layer),
                           FEATURE_SIZE * sizeof(DATA_TYPE),
                           cudaMemcpyKind::cudaMemcpyHostToDevice);
            } else {
                cudaMemset(linLayerBias, 0, FEATURE_SIZE * sizeof(DATA_TYPE));
            }

            cudnnErrCheck(cudnnDestroyFilterDescriptor(linLayerBiasDesc));
        }
    }

    cudaErrCheck(cudaDeviceSynchronize());

    cudaEvent_t start, stop;
    float timeForward;
    cudaErrCheck(cudaEventCreate(&start));
    cudaErrCheck(cudaEventCreate(&stop));

    cudaErrCheck(cudaEventRecord(start));

    cudnnErrCheck(cudnnRNNForwardInference(cudnnHandle,
                rnnDesc,
                seqLength,
                xDesc,
                x,
                hxDesc,
                hx,
                cxDesc,
                cx,
                wDesc,
                w,
                yDesc,
                y,
                hyDesc,
                hy,
                cyDesc,
                cy,
                workspace,
                workSize));

    cudaErrCheck(cudaEventRecord(stop));
    cudaErrCheck(cudaEventSynchronize(stop));
    cudaErrCheck(cudaEventElapsedTime(&timeForward, start, stop));

    // Make double-sure everything is finished before we copy for result checking.
    cudaDeviceSynchronize();

    cudaMemcpy(raw_y, y, FEATURE_SIZE * BATCH_SIZE * SEQ_LENGTH * sizeof(DATA_TYPE),
               cudaMemcpyKind::cudaMemcpyDeviceToHost);

    cudaDeviceSynchronize();

    return timeForward;
}

void free_cudnn() {
    cudaFree(x);
    cudaFree(y);
    cudaFree(workspace);
    cudaFree(w);

    cudnnDestroy(cudnnHandle);
}

float run_cudnn2(DATA_TYPE *raw_Weights, DATA_TYPE *raw_biases, DATA_TYPE *raw_x, DATA_TYPE *raw_y) {
    cudaErrCheck(cudaDeviceSynchronize());

    // Initialise inputs

    cudaMemcpy(raw_x, x2, FEATURE_SIZE * BATCH_SIZE * (SEQ_LENGTH / 2) * sizeof(DATA_TYPE),
               cudaMemcpyKind::cudaMemcpyDeviceToHost);

    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int linLayerID = 0; linLayerID < 8; linLayerID++) {
            cudnnFilterDescriptor_t linLayerMatDesc;
            cudnnErrCheck(cudnnCreateFilterDescriptor(&linLayerMatDesc));
            DATA_TYPE *linLayerMat;

            cudnnErrCheck(cudnnGetRNNLinLayerMatrixParams(cudnnHandle,
                        rnnDesc2,
                        layer,
                        xDesc2[0],
                        wDesc2,
                        w2,
                        linLayerID,
                        linLayerMatDesc,
                        (void**)&linLayerMat));

            cudnnDataType_t dataType;
            cudnnTensorFormat_t format;
            int nbDims;
            int filterDimA[3];
            cudnnErrCheck(cudnnGetFilterNdDescriptor(linLayerMatDesc,
                        3,
                        &dataType,
                        &format,
                        &nbDims,
                        filterDimA));

            cudaMemcpy(linLayerMat, raw_Weights + FEATURE_SIZE * FEATURE_SIZE * (linLayerID + 8 * layer),
                       FEATURE_SIZE * FEATURE_SIZE * sizeof(DATA_TYPE), cudaMemcpyKind::cudaMemcpyHostToDevice);

            cudnnErrCheck(cudnnDestroyFilterDescriptor(linLayerMatDesc));

            cudnnFilterDescriptor_t linLayerBiasDesc;
            cudnnErrCheck(cudnnCreateFilterDescriptor(&linLayerBiasDesc));
            DATA_TYPE *linLayerBias;

            cudnnErrCheck(cudnnGetRNNLinLayerBiasParams(cudnnHandle,
                        rnnDesc2,
                        layer,
                        xDesc2[0],
                        wDesc2,
                        w2,
                        linLayerID,
                        linLayerBiasDesc,
                        (void**)&linLayerBias));

            cudnnErrCheck(cudnnGetFilterNdDescriptor(linLayerBiasDesc,
                        3,
                        &dataType,
                        &format,
                        &nbDims,
                        filterDimA));

            // We use merged bias
            if (linLayerID < 4) {
                cudaMemcpy(linLayerBias,
                           raw_biases + FEATURE_SIZE * (linLayerID + 4 * layer),
                           FEATURE_SIZE * sizeof(DATA_TYPE),
                           cudaMemcpyKind::cudaMemcpyHostToDevice);
            } else {
                cudaMemset(linLayerBias, 0, FEATURE_SIZE * sizeof(DATA_TYPE));
            }

            cudnnErrCheck(cudnnDestroyFilterDescriptor(linLayerBiasDesc));
        }
    }

    cudaErrCheck(cudaDeviceSynchronize());

    cudaEvent_t start, stop;
    float timeForward;
    cudaErrCheck(cudaEventCreate(&start));
    cudaErrCheck(cudaEventCreate(&stop));

    cudaErrCheck(cudaEventRecord(start));

    cudnnErrCheck(cudnnRNNForwardInference(cudnnHandle,
                rnnDesc2,
                seqLength2,
                xDesc2,
                x2,
                hxDesc2,
                hx2,
                cxDesc2,
                cx2,
                wDesc2,
                w2,
                yDesc2,
                y2,
                hyDesc2,
                hy2,
                cyDesc2,
                cy2,
                workspace2,
                workSize2));

    cudaErrCheck(cudaEventRecord(stop));
    cudaErrCheck(cudaEventSynchronize(stop));
    cudaErrCheck(cudaEventElapsedTime(&timeForward, start, stop));

    // Make double-sure everything is finished before we copy for result checking.
    cudaDeviceSynchronize();

    cudaMemcpy(raw_y, y2, FEATURE_SIZE * BATCH_SIZE * (SEQ_LENGTH/2) * sizeof(DATA_TYPE),
               cudaMemcpyKind::cudaMemcpyDeviceToHost);

    cudaDeviceSynchronize();

    return timeForward;
}

void free_cudnn2() {
    cudaFree(x2);
    cudaFree(y2);
    cudaFree(workspace2);
    cudaFree(w2);

    //cudnnDestroy(cudnnHandle2);
}

static __global__ void _kernel_maxpool(DATA_TYPE *src, DATA_TYPE *dst)
{
  int batch = blockIdx.x;
  int feature = threadIdx.x;
  for (int s = 0; s< SEQ_LENGTH/2; s++){
    dst[s * BATCH_SIZE * FEATURE_SIZE + batch * FEATURE_SIZE + feature] = max(
                                                                                src[(2 * s) * BATCH_SIZE * FEATURE_SIZE + batch * FEATURE_SIZE + feature],
                                                                                src[(2 * s + 1) * BATCH_SIZE * FEATURE_SIZE + batch * FEATURE_SIZE + feature]
                                                                              );
  }
}

void downsampling_maxpool_wrapper(){
  _kernel_maxpool<<<BATCH_SIZE, FEATURE_SIZE>>>((float *) y, (float *)x2);
  // SYNCHRONIZE
  cudaDeviceSynchronize();
}
