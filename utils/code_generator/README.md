# Automatic code generator for Tiramisu

Generates random Tiramisu codes. The program currently generates two types of codes :
* Sequence of computations which can be : simple assignments, assignments with other computations or stencils.
* Sequence of convolution layers. With two types of padding : same padding (adding 0 padding so that the input and the output layers have the same height and width), and valid padding (no padding).


## Running the generator
### Code parameters
In this first version of the generator, we'll specify the input parameters in the input.txt file. Please leave the parameters' order as it is, and do not add any comments.
This method is to be changed.

```
nb_codes : "5"    //The number of codes to be generated.
nb_stages : "5"   //The number of stages of each generated code.
default_type_tiramisu : "p_int32"   //The default type of the Tiramisu data.
default_type_warpper : "int32_t"    //The default type of the Halide data in the wrapper file.
assignment_prob : "0.2"             //in case of sequence of computations, the represents the proportion of simple assignment computations.
assignment_input_prob : "0.3"       //in case of sequence of computations, the represents the proportion of assignment computations with other computations.
//The rest represents the proportion of stencil computations.
cnn_prob : "0.5"                    //Proportion of codes having convolutions, the rest of the generated codes will be of the first type (sequence of computations).
computation_dimensions : "10 10 10" //Computations' dimensions in case of the first type of codes. Note that, for now, all computations have the same dimensions. Please specify them as in the example (dimension sizes are separated with a whitespace).
nb_inputs : "3"                   //in case of assignment computations using other computations, this is the number of input computations.
var_nums : "0 1 2"                //in case of stencil computations, these are the variables concerned by the stencil operation.
offset : "2"                      //in case of stencil computations. ex, with offset = 2 and variable i only, we'll have i - 2, i - 1, i, i + 1 and i + 2.
same_padding prob : "0.1"         //in case of convolutions, this is the proportion of layers with "same" padding. The rest of the layers will have a "valid" padding.
```

### Generating the codes
Run the compile_and_run.sh script.

```
./compile_and_run.sh
```
each generated code will be stored in a separate folder with the associated wrapper file.

## Running the tests
See the folder `time_measurement` for more information.
