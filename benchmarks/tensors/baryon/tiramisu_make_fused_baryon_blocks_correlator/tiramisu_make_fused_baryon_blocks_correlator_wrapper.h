#ifndef TIRAMISU_test_h
#define TIRAMISU_test_h

#include "../../utils/qblocks_2pt_parameters.h"

#define SMALL_BARYON_DATA_SET 0
#define LARGE_BARYON_DATA_SET 0
#define USE_GLOBAL_PARAMS 1

#if SMALL_BARYON_DATA_SET

#define Nq 3
#define Nc 3
#define Ns 2
#define Nw 9
#define Nw2 Nw *Nw
#define twoNw 81
#define Nperms 36
#define Lt 2
#define Nt 2
#define Vsrc 4
#define Vsnk 8
#define Nsrc 4
#define Nsnk 4
#define mq 1.0
#define NsnkHex 4
#define Nb 4

#elif USE_GLOBAL_PARAMS

#define Nq P_Nq
#define Nc P_Nc
#define Ns P_Ns
#define Nw P_Nw
#define Nw2 Nw*Nw
#define Nw2Hex P_Nw2Hex
#define Nperms P_Nperms
#define B1Nperms P_B1Nperms
#define Lt P_Nt
#define Vsrc P_Vsrc
#define Vsnk P_Vsnk
#define Nsrc P_Nsrc
#define Nsnk P_Nsnk
#define NsrcHex P_NsrcHex
#define NsnkHex P_NsnkHex
#define mq P_mq
#define Nb P_Nb
#define B2Nrows P_B2Nrows
#define B1Nrows P_B1Nrows
#define NEntangled P_NEntangled
#define sites_per_rank P_sites_per_rank

#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Define these values for each new test
#define TEST_NAME_STR       "tiramisu_make_fused_baryon_blocks_correaltor"

#include <tiramisu/utils.h>

#ifdef __cplusplus
extern "C" {
#endif

int tiramisu_make_fused_baryon_blocks_correlator(
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

extern const struct halide_filter_metadata_t halide_pipeline_aot_metadata;
#ifdef __cplusplus
}  // extern "C"
#endif
#endif
