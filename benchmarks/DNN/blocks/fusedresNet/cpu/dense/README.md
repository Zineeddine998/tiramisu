The files in this folder are organized as follows:

    General
        clean.sh : remove some useless files.
        compile_and_run_mkldnn.sh : compile mkldnn code and run it. 
        compile_and_run_mkl.sh : compile MKL code and run it. 
        configure.h: define size of input matrices.

    Tiramisu
        fused_resnet_block_generator_tiramisu.cpp: Tiramisu code generator.

    Wrapper
        wrapper_nn_block.cpp: wrapped file that calls the code generated by Tiramisu.

    Intel MKLDNN
        fused_resnet_block_generator_mkldnn.cpp: code that calls Intel MKL DNN ResNet block. 

    Intel MKL
        fused_resnet_block_generator_mkl.c: code that calls Intel MKL ResNet block.

To run this benchmark:

    At the directory build/benchmarks/DNN/blocks/fusedresNet execute 
	    make 

    wrapper_nn_block_fused_resnet executable will be created in the current directory. 
    
    To compare the result of tiramisu with MKL DNN execute :
        ./compile_and_run_mkldnn.sh
    then 
        ./wrapper_nn_block_fused_resnet

    To compare the result of tiramisu with MKL execute :
        ./compile_and_run_mkl.sh
    then 
        ./wrapper_nn_block_fused_resnet
    
    execution results could be found in the text files : 
        mkl_result.txt (same for Intel MKL and Intel MKL-DNN)
        tiramisu_result.txt
        