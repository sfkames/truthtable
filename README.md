# truthtable
C program that reads a file containing a description of a circuit, and prints that circuit's truth table

CS211 PA4
Samantha Ames

truthtable takes a single argument, which is the name of a file containing a circuit description.
The behavior of truthtable is unspecified if it receives no arguments or more than one argument,
but you should still check that the number of arguments is correct. (One possibility is to have
truthtable read from standard input if no file argument is given.)
Usage
$ ./truthtable my-cool-circuit.txt
0 0 | 0 0
0 1 | 0 1
1 0 | 0 1
1 1 | 1 0
Input The input to your program will be a single circuit description using the language described
in section 3. The first argument to truthtable will identify a file containing this circuit description.
You MAY assume that the input is correctly formatted and that no variable depends on its own
output.
Output The output of truthtable is a truth table showing each combination of inputs and the
corresponding output for the specified circuit. Each column in the table corresponds to a specific
input or output variable, which are given in the same order as their declaration in the INPUT and
OUTPUT directives. Columns are separated by a single space, and a vertical bar (|) occurs between
the input and output variables.
Note that no white space follows the final column.
