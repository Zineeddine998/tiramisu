export BEAM_SIZE=3
export DYNAMIC_RUNS=1
export MAX_RUNS=1
export NB_EXEC=1
export AS_VERBOSE=1
export EXPLORE_BY_EXECUTION=1
export SAVE_BEST_SCHED_IN_FILE=0
export EXECUTE_BEST_AND_INITIAL_SCHED=1
export SET_DEFAULT_EVALUATION=0
export PRUNE_SLOW_SCHEDULES=0
export MAX_DEPTH=4
export MIN_RUNS=1
export TIRAMISU_ROOT=/data/kb4083/tiramisu/

# Compile the autoscheduler
cd ${TIRAMISU_ROOT}/build
make -j tiramisu_auto_scheduler

cd ${TIRAMISU_ROOT}/tutorials/tutorial_autoscheduler
# Cmpile and run a program
mkdir build ; cd build
cmake .. -DBUILD_TARGET="generator"  -DFUNCTION_TO_BUILD=$1
make
../generator
g++ -shared -o function.o.so $1.o
# Generate a wrapper
g++ -std=c++11 -fno-rtti -I${TIRAMISU_ROOT}/include -DTIRAMISU_MINI -I${TIRAMISU_ROOT}/3rdParty/Halide/include -I${TIRAMISU_ROOT}/3rdParty/isl/include/ -I${TIRAMISU_ROOT}/benchmarks -L${TIRAMISU_ROOT}/build -L${TIRAMISU_ROOT}/3rdParty/Halide/lib/ -L${TIRAMISU_ROOT}/3rdParty/isl/build/lib -o $1_wrapper -ltiramisu -lHalide -ldl -lpthread -lz -lm -Wl,-rpath,${TIRAMISU_ROOT}/build ../$1_wrapper.cpp ./function.o.so -ltiramisu -lHalide -ldl -lpthread -lz -lm

cmake .. -DBUILD_TARGET="autoscheduler"  -DFUNCTION_TO_BUILD=$1
make
../generator