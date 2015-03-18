#ifndef TABLES_H
#define TABLES_H

#include <stdint.h>

extern const int SYMTBL_NON_UNIQUE;
extern const int SYMTBL_UNIQUE_NAME;

/* Signature of the SymbolTable data structure. */

typedef struct {
    char *name;
    uint32_t addr;
} Symbol;

typedef struct {
    Symbol* tbl;
    uint32_t len;
    uint32_t cap;
    int mode;
} SymbolTable;

/* Helper functions: */

void allocation_failed();

void addr_alignment_incorrect();

void name_already_exists(const char* name);

void write_symbol(FILE* output, uint32_t addr, const char* name);

SymbolTable* create_table();

void free_table(SymbolTable* table);

int add_to_table(SymbolTable* table, const char* name, uint32_t addr);

int64_t get_addr_for_symbol(SymbolTable* table, const char* name);

void write_table(SymbolTable* table, FILE* output);

#endif
