#ifndef QBLOCKS_2PT_PARAMETERS_H
#define QBLOCKS_2PT_PARAMETERS_H

#define DATA_SET_1 0
#define DATA_SET_2 0
#define DATA_SET_3 0
#define DATA_SET_4 1

#if DATA_SET_1

#define P_Vsrc 512
#define P_Vsnk 512
#define P_Nsrc 44
#define P_Nsnk 44
#define P_NEntangled 3
#define P_NsrcHex 2
#define P_NsnkHex 2
#define P_Nperms 36
#define P_B1Nperms 2
#define P_Nw 12
#define P_Nw2 288
#define P_Nw2Hex 32
#define P_Nt 2
#define P_Nc 3
#define P_Ns 2
#define P_Nq 3
#define P_B2Nrows 4
#define P_B1Nrows 2
#define P_Nb 2
#define P_mq 1.0
#define P_Mq 2
#define P_B0Nrows 2
#define P_Mw 12
#define P_NsFull 4
#define P_sites_per_rank 4
#define P_src_sites_per_rank 4

#elif DATA_SET_2

#define P_Vsrc 8
#define P_Vsnk 8
#define P_Nsrc 44
#define P_Nsnk 44
#define P_NEntangled 3
#define P_NsrcHex 2
#define P_NsnkHex 2
#define P_Nperms 36
#define P_B1Nperms 2
#define P_Nw 12
#define P_Nw2 288
#define P_Nw2Hex 32
#define P_Nt 2
#define P_Nc 3
#define P_Ns 2
#define P_Nq 3
#define P_B2Nrows 4
#define P_B1Nrows 2
#define P_Nb 2
#define P_mq 1.0
#define P_Mq 2
#define P_B0Nrows
#define P_Mw 12
#define P_NsFull 4
#define P_sites_per_rank 4
#define P_src_sites_per_rank 4

#elif DATA_SET_3

#define P_Vsrc 2
#define P_Vsnk 2
#define P_Nsrc 22
#define P_Nsnk 22
#define P_NEntangled 3
#define P_NsrcHex 2
#define P_NsnkHex 2
#define P_Nperms 12
#define P_B1Nperms 2
#define P_Nw 12
#define P_Nw2 32
#define P_Nw2Hex 32
#define P_Nt 2
#define P_Nc 3
#define P_Ns 2
#define P_Nq 3
#define P_B2Nrows 2
#define P_B1Nrows 2
#define P_Nb 2
#define P_mq 1.0
#define P_Mq 2
#define P_B0Nrows
#define P_Mw 2
#define P_NsFull 2
#define P_sites_per_rank 2
#define P_src_sites_per_rank 2

#elif DATA_SET_4

#define P_Vsrc 8
#define P_Vsnk 8
#define P_Nsrc 44
#define P_Nsnk 44
#define P_NEntangled 3
#define P_NsrcHex 1
#define P_NsnkHex 1
#define P_Nperms 36
#define P_B1Nperms 2
#define P_Nw 12
#define P_Nw2 288
#define P_Nw2Hex 32
#define P_Nt 1
#define P_Nc 3
#define P_Ns 2
#define P_Nq 3
#define P_B2Nrows 4
#define P_B1Nrows 2
#define P_Nb 2
#define P_mq 1.0
#define P_Mq 2
#define P_B0Nrows 1
#define P_Mw 12
#define P_NsFull 4
#define P_sites_per_rank 4
#define P_src_sites_per_rank 4

#endif

#endif
