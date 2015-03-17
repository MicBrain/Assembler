# Assembler
This is an implementation of an assembler that translates a subset of the MIPS instruction set to machine code.

At a high level, the functionality of our assembler can be divided as follows:

* Pass 1: Reads the input (.s) file. Comments are stripped, pseudoinstructions are expanded, and the address of each label is recorded into the symbol table.  Input validation of the labels and  pseudoinstructions is performed here.  The output is written to an intermediate (.int) file. 
* Pass 2: Reads the intermediate file and translates each instruction to machine code. Instruction syntax and arguments are validated at this step.
