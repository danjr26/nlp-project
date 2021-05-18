# NLP Semester Project
Daniel Riehm (driehm1)

## Prerequisites
The Python code (finished transformer model) was written with `Python 3.7.3`, and requires `tensorflow==2.4.0` to run.

The C++ code (unfinished custom model) can be built using the included CMake file and `g++` or another compiler. 

## Run Code
The transformer can generate text using the following command:
```shell
python src/transformer.py --load transformer_model --generate <N>
```
Where `<N>` is the maximum number of characters to generate.

The transformer can be tested using the following command:
```shell
python src/transformer.py --load transformer_model --test <test_file>
```
Where `<test_file>` contains poems delimited by `\n#SEP#\n`. You probably want to use `data/test.txt` for this. Note that it will output the `log2` of the perplexity, not the perplexity itself.

The custom model, when run, will train on the training data, write sample output to `out.txt`, test on the test data, then output the model perplexity.