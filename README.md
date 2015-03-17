# Assembler
This is an implementation of an assembler that translates a subset of the MIPS instruction set to machine code.

At a high level, the functionality of our assembler can be divided as follows:

* Pass 1: Reads the input (.s) file. Comments are stripped, pseudoinstructions are expanded, and the address of each label is recorded into the symbol table.  Input validation of the labels and
