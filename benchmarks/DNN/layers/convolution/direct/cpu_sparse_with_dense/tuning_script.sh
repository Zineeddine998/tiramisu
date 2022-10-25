#!/bin/bash

for x in 2 3 4 8 14 16; do
	for fo in 8 14 16 4 2; do
	    for fin in 8 14 16 4 2; do
	            echo "#define FOut 32" > tuning_parameters.h
	            echo "#define FIn 32" >> tuning_parameters.h
	            echo "#define FOUT_BLOCKING $fo" >> tuning_parameters.h
	            echo "#define FIN_BLOCKING $fin" >> tuning_parameters.h
	            echo "#define X_BLOCKING $x" >> tuning_parameters.h
		    echo "#define FOUT_NB_BLOCKS FOut/FOUT_BLOCKING" >> tuning_parameters.h
		    echo "#define FIN_NB_BLOCKS FIn/FIN_BLOCKING" >> tuning_parameters.h
		    echo "#define X_NB_BLOCKS N/X_BLOCKING" >> tuning_parameters.h
		    echo "#define X_BOUND X_NB_BLOCKS*X_BLOCKING" >> tuning_parameters.h
		    echo "" >> tuning_parameters.h
		    cat tuning_parameters.h
		    cat tuning_parameters.h >> log.txt;
		    cd /Users/b/Documents/src/MIT/tiramisu/build/benchmarks/DNN/layers/convolution/direct/cpu_sparse_with_dense;
		    make &> log.txt;
		    cd -
		    ./wrapper_conv_sparse_with_dense_layer
	done
done
