#set -x

source ../../../../../configure_paths.sh
MKLDNNROOT=/usr/local/

export INCLUDES="-I${MKL_PREFIX}/include/ -I${MKLDNNROOT}/include"
export LIBRARIES="${MKL_FLAGS} -lisl -lz -lpthread -ldl "
export LIBRARIES_DIR="-L${MKL_PREFIX}/lib/${MKL_LIB_PATH_SUFFIX} -L${MKLDNNROOT}/lib"

source ${MKL_PREFIX}/bin/mklvars.sh ${MKL_LIB_PATH_SUFFIX}

g++ -O3 -DMKL_ILP64 -m64 ${INCLUDES} sparse_vggblock_generator_mkl_sparse.cpp -o vggblock_mkl_sparse ${LIBRARIES_DIR} -Wl,--no-as-needed -lmkl_intel_ilp64 -lmkl_gnu_thread -lmkl_core -lgomp -lpthread -fopenmp -lm -ldl  -lmkldnn
./vggblock_mkl_sparse
