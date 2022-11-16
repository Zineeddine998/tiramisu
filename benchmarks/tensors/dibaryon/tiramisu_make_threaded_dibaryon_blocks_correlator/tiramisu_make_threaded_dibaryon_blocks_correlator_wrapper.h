#ifndef TIRAMISU_test_h
#define TIRAMISU_test_h

#include "../../utils/qblocks_2pt_parameters.h"

#define SMALL_BARYON_DATA_SET 0
#define LARGE_BARYON_DATA_SET 0
#define USE_GLOBAL_PARAMS 1

#define R1 0

#if SMALL_BARYON_DATA_SET

#define Nq 3
#define Nc 3
#define Ns 2
#define Nw 9
#define Nw2 81
#define Nperms 9
#define Lt 10
#define Vsrc 8
#define Vsnk 8
#define Nsrc 4
#define Nsnk 4
#define mq 1.0
#define Nb 2

#elif LARGE_BARYON_DATA_SET

#define Nq 3
#define Nc 3
#define Ns 2
#define Nw 9
#define Nw2 81
#define Nperms 9 //36
#define Lt 4 // 1..32
#define Vsrc 16 //64 //8, 64, 512
#define Vsnk 16 //64 //8, 64, 512
#define Nsrc 6
#define Nsnk 6
#define mq 1.0
#define Nb 2

#elif USE_GLOBAL_PARAMS

#define Nq P_Nq
#define Nc P_Nc
#define Ns P_Ns
#define Nw P_Nw
#define Nw2 Nw*Nw
#define Nw2Hex 32
#define Nperms P_Nperms
#define Lt P_Nt
#define Vsrc P_Vsrc
#define Vsnk P_Vsnk
#define Nsrc P_Nsrc
#define Nsnk P_Nsnk
#define NsrcHex P_NsrcHex
#define NsnkHex P_NsnkHex
#define mq P_mq
#define Nb 2

#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Define these values for each new test
#define TEST_NAME_STR       "tiramisu_make_threaded_dibaryon_blocks_correaltor"

#include <tiramisu/utils.h>

static int src_color_weights_r1_P[Nw][Nq] = { {0,1,2}, {0,2,1}, {1,0,2} ,{0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,1,0}, {2,0,1} };
static int src_spin_weights_r1_P[Nw][Nq] = { {0,1,0}, {0,1,0}, {0,1,0}, {1,0,0}, {1,0,0}, {1,0,0}, {1,0,0}, {1,0,0}, {1,0,0} };
static double src_weights_r1_P[Nw] = {-2/ sqrt(2), 2/sqrt(2), 2/sqrt(2), 1/sqrt(2), -1/sqrt(2), -1/sqrt(2), 1/sqrt(2), -1/sqrt(2), 1/sqrt(2)};

static int src_color_weights_r2_P[Nw][Nq] = { {0,1,2}, {0,2,1}, {1,0,2} ,{1,2,0}, {2,1,0}, {2,0,1}, {0,1,2}, {0,2,1}, {1,0,2} };
static int src_spin_weights_r2_P[Nw][Nq] = { {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1}, {1,0,1}, {1,0,1}, {1,0,1} };
static double src_weights_r2_P[Nw] = {1/ sqrt(2), -1/sqrt(2), -1/sqrt(2), 1/sqrt(2), -1/sqrt(2), 1/sqrt(2), -2/sqrt(2), 2/sqrt(2), 2/sqrt(2)};

#ifdef __cplusplus
extern "C" {
#endif

int tiramisu_make_threaded_dibaryon_blocks_correlator(
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *,
			    halide_buffer_t *);


int tiramisu_generated_code_argv(void **args);

void declare_memory_usage(){
    setenv("MEM_SIZE", std::to_string((double)18*18*18*2*8/1024).c_str(), true); // This value was set by the Code Generator
}

extern const struct halide_filter_metadata_t halide_pipeline_aot_metadata;
#ifdef __cplusplus
}  // extern "C"
#endif
#endif