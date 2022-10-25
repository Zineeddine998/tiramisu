#set -x

MKLDNNROOT=/usr/local/

g++ -std=c++11 -I${MKLDNNROOT}/include -L${MKLDNNROOT}/lib resize_conv_relu_maxpool_generator_mkldnn.cpp -lmkldnn -o resize_conv_relu_maxpool_mkldnn $(pkg-config --cflags --libs opencv)
./resize_conv_relu_maxpool_mkldnn
