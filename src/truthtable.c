#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { AND, OR, NAND, NOR, XOR, NOT, PASS, DECODER, MULTIPLEXER } kind_t;

char* operative[9] =
{ "AND", "OR", "NAND", "NOR", "XOR", "NOT", "PASS", "DECODER", "MULTIPLEXER" };

// Gate structure
typedef struct gate {
    kind_t kind;
    int size;       // indicates size of DECODER and MULTIPLEXER
    int *params;    // length determined by kind and size;
                    // includes inputs and outputs, indicated by variable numbers
} gate_t;

// Variable structure
typedef struct variable {
    char *name;     // variable name
    int data;       // variable value
    int flag;       // indicates whether temp output or final output
} var_t;

// Circuit structure
typedef struct Circuit {
    int num_in;     // number inputs
    int num_out;    // number outputs
    int num_vars;   // number of variables
    int num_gates;  // number of gates
    int *inputs;    // ptr for input variable positions
    int *outputs;   // ptr for output variable positions
    gate_t **gates; // lists gates
    var_t **vars;  // lists variables
} circuit_t;

// Function declarations
circuit_t *new_circuit(char *filename);
circuit_t *create_circuit(void);
var_t *new_variable(char *name, circuit_t *circuit);
int add_variable(char *name, circuit_t *circuit);
int find_variable(char *var, circuit_t *circuit);
gate_t *new_gate(kind_t kind, int x, int size, circuit_t *circuit);
void add_gate(gate_t *gate, circuit_t *circuit);
int solve_gate(gate_t *gate, circuit_t *circuit);
void solve_circuit(circuit_t *circuit);
int to_decimal(int start, int end, gate_t *gate, circuit_t *circuit);
int increment(circuit_t *circuit);
void undef(circuit_t *circuit);
int check_outputs(circuit_t *circuit);
void print_inputs(circuit_t *circuit);
void print_outputs(circuit_t *circuit);
void free_gate(gate_t *gate);
void free_var(var_t *var);
void free_circuit(circuit_t *circuit);


// Given a file name, return a circuit
circuit_t *new_circuit(char *filename)
{
    // Read text file as argument
	FILE *in_file = fopen(filename, "r");

    char str[17];
    // Number of outputs, index of outputs, x = variable holder
    int num_out, out_ind, a, x, sz;
    kind_t kind;
    circuit_t *circuit;
    gate_t *gate;
    
    // If error loading file, return error and exit
    if(in_file == NULL) {
        printf("Error loading file\n");
        exit(EXIT_FAILURE);
    }

    // Initialize new circuit
    circuit = create_circuit();

    // Read input variables to circuit struct
    fscanf(in_file, "%16s %d", str, &circuit->num_in);

    // Allocate memory for inputs
    circuit->inputs = (int*)malloc(circuit->num_in * sizeof(int));

    if (circuit->inputs == NULL) {
        printf("Error allocating input list\n");
        free_circuit(circuit);
        exit(EXIT_FAILURE);
    }

    // Read input variables
    for(int i = 0; i < circuit->num_in; i++) {
        
        // Read variable
        fscanf(in_file, "%16s", str);

        // Create variable
        x = add_variable(str, circuit);
        
        // Save input variable in order
        circuit->inputs[i] = x;
    }

    // Read output variables to circuit struct
    fscanf(in_file, "%16s %d", str, &circuit->num_out);

    // Allocate memory for outputs
    circuit->outputs = (int*)malloc(circuit->num_out * sizeof(int));

    if (circuit->outputs == NULL) {
        printf("Error allocating output list\n");
        free_circuit(circuit);
        exit(EXIT_FAILURE);
    }

    // Read output variables
    for(int i = 0; i < circuit->num_out; i++) {

        // Read variable
        fscanf(in_file, "%16s", str);

        // Create variable
        x = add_variable(str, circuit);

        // Save output variable in order
        circuit->outputs[i] = x;

        // Flag
        circuit->vars[circuit->outputs[i]]->flag = 1;
    }

    // Read remaining lines of file
    while(!feof(in_file)) {

        // Read directive
        if(fscanf(in_file, "%16s", str) == 1) {

            out_ind = 2;
            num_out = 1;

            if (!strcmp(str, "NOT")) {
                kind = NOT;
                x = 2;
                out_ind = 1;
                num_out = 1;
            }
            if(!strcmp(str, "PASS")) {
                kind = PASS;
                x = 2;
                out_ind = 1;
                num_out = 1;
            } else if(!strcmp(str, "AND")) {
                kind = AND;
                x = 3;
            } else if (!strcmp(str, "OR")) {
                kind = OR;
                x = 3;
            } else if (!strcmp(str, "NAND")) {
                kind = NAND;
                x = 3;
            } else if (!strcmp(str, "NOR")) {
                kind = NOR;
                x = 3;
            } else if (!strcmp(str, "XOR")) {
                kind = XOR;
                x = 3;
            } else if (!strcmp(str, "DECODER")) {
                kind = DECODER;
                // Read number of inputs
                fscanf(in_file, "%d", &sz);
                // x + 2^x # of parameters
                x = sz + (1 << sz);
                // Begin outputs
                out_ind = sz;
                // # of outputs
                num_out = 1 << sz;
            } else if (!strcmp(str, "MULTIPLEXER")) {
                kind = MULTIPLEXER;
                // Read number of selectors
                fscanf(in_file, "%d", &sz);
                // x + 2^x + 1 # of parameters
                x = sz + (1 << sz) + 1;
                // Begin outputs
                out_ind = sz + (1 << sz);
                // # of outputs
                num_out = 1;
            }

            // Create gate
            gate = new_gate(kind, x, sz, circuit);

            // Read the variables
            for(int i = 0; i < x; i++) {

                fscanf(in_file, "%16s", str);

                // Create variable
                a = add_variable(str, circuit);

                // Save variable in order
                gate->params[i] = a;

                // Flag 
                if(i >= out_ind && i <= (num_out + out_ind - 1)) {
                    circuit->vars[a]->flag = 1;
                } 
            }

            // Add the gate to the circuit
            add_gate(gate, circuit);
        }
    }
    fclose(in_file);
    return circuit;
}

// Create a circuit
circuit_t *create_circuit(void)
{
    int x;

    // Allocate space for circuit
    circuit_t *circuit = (circuit_t*)malloc(sizeof(circuit_t));

    if (circuit == NULL) {
        printf("Error allocating circuit\n");
        exit(EXIT_FAILURE);
    }

    // Zero-out all values
    memset(circuit, 0, sizeof(circuit_t));
    // Zero bit
    x = add_variable("0", circuit);
    circuit->vars[x]->data = 0;
    // One bit
    x = add_variable("1", circuit);
    circuit->vars[x]->data = 1;
    add_variable("_", circuit);

    return circuit;
}

// Initialize new variable
var_t *new_variable(char *name, circuit_t *circuit)
{
    // Allocate memory for new variable
    var_t *var = (var_t*)malloc(sizeof(var_t));

    if (var == NULL) {
        printf("Error allocating variable\n");
        free_circuit(circuit);
        exit(EXIT_FAILURE);
    }

    // Allocate memory for variable name
    var->name = (char*)malloc(strlen(name) + 1);

    if (var->name == NULL) {
        printf("Error allocating variable name\n");
        free(var);
        free_circuit(circuit);
        exit(EXIT_FAILURE);
    }

    // Save variable name
    strcpy(var->name, name);

    // Not yet defined
    var->data = -1;

    // Flag
    var->flag = 0;

    return var;
}

// Add variable to its circuit's list, returns its index
int add_variable(char *name, circuit_t *circuit)
{
    int x = find_variable(name, circuit);
    var_t **vars;

    // Not yet defined = new variable
    if(x == -1) {

        x = circuit->num_vars;

        // If 0, allocate memory for new variable list, otherwise reallocate more memory for list
        if (x == 0) {
            vars = (var_t**)malloc(sizeof(var_t*));
        } else {
            vars = (var_t**)realloc(circuit->vars, (x + 1) * sizeof(var_t*));
        }

        if (vars == NULL) {
            printf("Error allocating variable list\n");
            free_circuit(circuit);
            exit(EXIT_FAILURE);
        }

        circuit->vars = vars;
        circuit->vars[x] = new_variable(name, circuit);
        circuit->num_vars++;
    }
    return x;
}

// Iterate through circuit's variable list, if found return index, otherwise return -1
int find_variable(char *var, circuit_t *circuit)
{
    int index = -1;

    for(int i = 0; i < circuit->num_vars && index == -1; i++) {
        if(!strcmp(var, circuit->vars[i]->name)) {
            index = i;
        }
    }
    return index;
}

// Initialize new gate with x # parameters
gate_t *new_gate(kind_t kind, int x, int size, circuit_t *circuit)
{
    gate_t *gate = (gate_t*)malloc(sizeof(gate_t));

    if (gate == NULL) {
        printf("Error allocating gate\n");
        free_circuit(circuit);
        exit(EXIT_FAILURE);
    }

    gate->kind = kind;
    gate->params = (int*)malloc(x * sizeof(int*));

    if (gate->params == NULL) {
        printf("Error allocating parameter\n");
        free(gate);
        free_circuit(circuit);
        exit(EXIT_FAILURE);
    }

    gate->size = size;

    return gate;
}

// Add gate to its circuit's gate list
void add_gate(gate_t *gate, circuit_t *circuit)
{
    int x = circuit->num_gates;
    gate_t **gates;

    // If no gates, allocate memory for gate list, otherwise reallocate more memory for list
    if (x == 0) {
        gates = (gate_t**)malloc(sizeof(gate_t*));
    } else {
        gates = (gate_t**)realloc(circuit->gates, (x + 1) * sizeof(gate_t*));
    }

    if (gates == NULL) {
        printf("Error allocating gate list\n");
        free_circuit(circuit);
        exit(EXIT_FAILURE);
    }

    circuit->gates = gates;
    circuit->gates[x] = gate;
    circuit->num_gates++;
}

// Determine gate output
int solve_gate(gate_t *gate, circuit_t *circuit)
{
    int a, b, index;
    int output = -1;

    // Input gate values
    a = circuit->vars[gate->params[0]]->data;
    b = circuit->vars[gate->params[1]]->data;

    switch (gate->kind) {
        case AND:
            if (a != -1 && b != -1) {
                output = (a & b);
            }
            circuit->vars[gate->params[2]]->data = output;
            break;

        case OR:
            if (a != -1 && b != -1) {
                output = (a | b);
            }
            circuit->vars[gate->params[2]]->data = output;
            break;

        case NAND:
            if (a != -1 && b != -1) {
                output = (~(a & b)) & 1;
            }
            circuit->vars[gate->params[2]]->data = output;
            break;

        case NOR:
            if (a != -1 && b != -1) {
                output = (~(a | b)) & 1;
            }
            circuit->vars[gate->params[2]]->data = output;
            break;

        case XOR:
            if (a != -1 && b != -1) {
                output = (a ^ b);
            }
            circuit->vars[gate->params[2]]->data = output;
            break;

        case NOT:
            if (a != -1) {
                output = (~a) & 1;
            }
            circuit->vars[gate->params[1]]->data = output;
            break;
        
        case PASS:
            output = a;
            circuit->vars[gate->params[1]]->data = output;
            break;

        case DECODER:
            index = to_decimal(0, gate->size - 1, gate, circuit);

            if(index == -1) {
                for (int i = 0; i < (1 << gate->size); i++) {
                    circuit->vars[gate->params[gate->size + i]]->data = -1;
                }
            } else {
                int i = 0;
                for (i = 0; i < (1 << gate->size); i++) {
                    circuit->vars[gate->params[gate->size + i]]->data = 0;
                }
                circuit->vars[gate->params[gate->size + index]]->data = 1;
                output = 1;
            }
            break;

        case MULTIPLEXER:
            index = to_decimal(1 << gate->size, (1 << gate->size) + gate->size - 1, gate, circuit);

            if (index == -1) {
                circuit->vars[gate->params[(1 << gate->size) + gate->size]]->data = -1;
            } else {
                output = circuit->vars[gate->params[index]]->data;
                circuit->vars[gate->params[(1 << gate->size) + gate->size]]->data = output;
            }
            break;
    }
    return output;
}

// Determine the circuit outputs
void solve_circuit(circuit_t *circuit)
{
    int x;

    // Set all input values to zero bit
    for (int i = 0; i < circuit->num_in; i++) {
        circuit->vars[circuit->inputs[i]]->data = 0;
    }

    while (!x) {
        print_inputs(circuit);
        undef(circuit);

        while (!check_outputs(circuit)) {
            for (int i = 0; i < circuit->num_gates; i++) {
                solve_gate(circuit->gates[i], circuit);
            }
        }
        printf("|");
        print_outputs(circuit);
        printf("\n");
        x = increment(circuit);
    }
}

// Convert gate variables to decimal value
int to_decimal(int start, int end, gate_t *gate, circuit_t *circuit)
{
    int out = 0;

    for (int i = end, p = 0; i >= start && out != -1; i--, p++) {
        if (circuit->vars[gate->params[i]]->data == -1) {
            out = -1;
        } else {
            out += circuit->vars[gate->params[i]]->data << p;
        }
    }
    return out;
}

// Return 0 on success, otherwise 1 if all inputs = 1
int increment(circuit_t *circuit)
{
    int x = 1;

    for (int i = circuit->num_in - 1; i >= 0 && x == 1; i--) {
        if (circuit->vars[circuit->inputs[i]]->data == 0) {
            circuit->vars[circuit->inputs[i]]->data = 1;
            x = 0;
        } else {
            circuit->vars[circuit->inputs[i]]->data = 0;
            x = 1;
        }
    }
    return x;
}

// Set all outputs to undefined = -1
void undef(circuit_t *circuit)
{
    // Ignore 0, 1, _ wires
    for (int i = 3; i < circuit->num_vars; i++) {
        if(circuit->vars[i]->flag) {
            circuit->vars[i]->data = -1;
        }
    }
}

// Return 1 if outputs are valid, otherwise return 0
int check_outputs(circuit_t *circuit)
{
    for (int i = 0; i < circuit->num_out; i++) {
        if (circuit->vars[circuit->outputs[i]]->data == -1) {
            return 0;
        }
    }
    return 1;
}

// Print inputs
void print_inputs(circuit_t *circuit)
{
    for (int i = 0; i < circuit->num_in; i++) {
        printf("%d ", circuit->vars[circuit->inputs[i]]->data);
    }
}

// Print outputs
void print_outputs(circuit_t *circuit)
{
    for (int i = 0; i < circuit->num_out; i++) {
        printf(" %d", circuit->vars[circuit->outputs[i]]->data);
    }
}

// Free allocated gate memory
void free_gate(gate_t *gate)
{
    if (gate != NULL) {
        if (gate->params != NULL) {
            free(gate->params);
        }
        free(gate);
    }
}

// Free allocated variable memory
void free_var(var_t *var)
{
    if (var != NULL) {
        if(var->name != NULL) {
            free(var->name);
        }
        free(var);
    }
}

// Free allocated circuit memory
void free_circuit(circuit_t *circuit)
{
    if (circuit != NULL) {
        if (circuit->num_in && circuit->inputs != NULL) {
            free(circuit->inputs);
        }
        if (circuit->num_out && circuit->outputs != NULL) {
            free(circuit->outputs);
        }
        if (circuit->gates != NULL) {
            for (int i = 0; i < circuit->num_gates; i++) {
                free_gate(circuit->gates[i]);
            }
            free(circuit->gates);
        }
        if (circuit->vars != NULL) {
            for (int i = 0; i < circuit->num_vars; i++) {
                free_var(circuit->vars[i]);
            }
            free(circuit->vars);
        }
        free(circuit);
    }
}

// Main driver
int main(int argc, char **argv)
{
    circuit_t *circuit;

    // If not given single argument, error
    if(argc != 2) {
		printf("Error\n");
		return EXIT_FAILURE;
	}

    // Create circuit given file
    circuit = new_circuit(argv[1]);
    solve_circuit(circuit);
    free_circuit(circuit);
    return EXIT_SUCCESS;
}