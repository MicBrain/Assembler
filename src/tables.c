

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "tables.h"

const int SYMTBL_NON_UNIQUE = 0;
const int SYMTBL_UNIQUE_NAME = 1;

/*******************************
 * Helper Functions
 *******************************/

void allocation_failed() {
    write_to_log("Error: allocation failed\n");
    exit(1);
}

void addr_alignment_incorrect() {
    write_to_log("Error: address is not a multiple of 4.\n");
}

void name_already_exists(const char* name) {
    write_to_log("Error: name '%s' already exists in table.\n", name);
}

void write_symbol(FILE* output, uint32_t addr, const char* name) {
    fprintf(output, "%u\t%s\n", addr, name);
}

/*******************************
 * Symbol Table Functions
 *******************************/

/* Creates a new SymbolTable containg 0 elements and returns a pointer to that
   table. Multiple SymbolTables may exist at the same time. 
   If memory allocation fails, it calls allocation_failed(). 
   Mode will be either SYMTBL_NON_UNIQUE or SYMTBL_UNIQUE_NAME. You will need
   to store this value for use during add_to_table().
 */

SymbolTable* create_table(int mode) {
    SymbolTable* myTable = (SymbolTable*)malloc(sizeof(SymbolTable)); //DO WE NEED TO USE CALLOC HERE ? OR SHOULD WE?
    if(!myTable) {
      allocation_failed();
    }
    myTable -> len = 0;
    myTable -> mode = mode;
    myTable -> cap = 5;
    myTable -> tbl = malloc(100 * sizeof(Symbol));
    if(!(myTable -> tbl)) {
      allocation_failed();
    }
    return myTable;
}

/* Frees the given SymbolTable and all associated memory. */
void free_table(SymbolTable* table) {
  int size_table = table -> len;
  for(int i = 0; i < size_table; i++) {
    free( (table-> tbl)[i].name);
  }
  free(table -> tbl);
  free(table);
}

/* Adds a new symbol and its address to the SymbolTable pointed to by TABLE. 
   ADDR is given as the byte offset from the first instruction. The SymbolTable
   must be able to resize itself as more elements are added. 

   Note that NAME may point to a temporary array, so it is not safe to simply
   store the NAME pointer. You must store a copy of the given string.

   If ADDR is not word-aligned, you should call addr_alignment_incorrect() and
   return -1. If the table's mode is SYMTBL_UNIQUE_NAME and NAME already exists 
   in the table, you should call name_already_exists() and return -1. If memory
   allocation fails, you should call allocation_failed(). 

   Otherwise, you should store the symbol name and address and return 0.
 */
int add_to_table(SymbolTable* table, const char* name, uint32_t addr) {
    // char* nameHolder;
    // strcpy(nameHolder, name);
    // printf("%s%s\n","OOOOOOOOOGGGGGGG NAME IS: ", name );
    if(addr % 4 != 0) {
      addr_alignment_incorrect();
      return -1;
    }
    // Symbol* ptHead = table -> tbl;
    char newName[strlen(name) + 1];
    strcpy(newName, name);
    if ( (table -> mode) == SYMTBL_UNIQUE_NAME){
      int size_table = table -> len;
      for (int i = 0; i < size_table; i++) {
        if(strcmp(newName, ((table -> tbl)[i]).name) == 0) {
          name_already_exists(name);
          return -1;
        }
      }
    }
    if(table -> len == table -> cap) {
      table -> cap = (table -> cap) * 5;
      table -> tbl = realloc(table->tbl, (table->cap) * sizeof(Symbol));
      uint32_t new_size_needed = strlen(name) + 1; //Also need to make sure you can add one to a uint32_t
      table->tbl[(table->len)].name = malloc(new_size_needed * sizeof(char));
      strcpy(table->tbl[(table->len)].name, newName);
      table->tbl[(table->len)].addr = addr;
      table -> len = table->len + 1;      
    } else {
        uint32_t new_size_needed = strlen(newName) + 1;
        table->tbl[(table->len)].name = malloc(new_size_needed * sizeof(char));
        strcpy(table->tbl[(table->len)].name, newName);
        table->tbl[(table->len)].addr = addr;
        table ->len = table -> len + 1;
    }
    return 0;
}

/* Returns the address (byte offset) of the given symbol. If a symbol with name
   NAME is not present in TABLE, return -1.
 */
int64_t get_addr_for_symbol(SymbolTable* table, const char* name) {
  // fprintf(stdout, "%s\n", "GOT HERE");
    for(int i = 0; i< table -> len; i++) {
      if(strcmp(name, table->tbl[i].name) == 0) {
        return table->tbl[i].addr; //IS IT A PROBLEM THAT ADDR IS A uint32_t and this func expects a int64_t
      }
    }
    return -1;   
}

/* Writes the SymbolTable TABLE to OUTPUT. You should use write_symbol() to
   perform the write. Do not print any additional whitespace or characters.
 */
void write_table(SymbolTable* table, FILE* output) {
    for (int i = 0; i < table -> len; i++) {
      Symbol* ptHead = table -> tbl;
      Symbol currSymbol = ptHead[i];
      char* currName = currSymbol.name;
      int64_t currAddr = currSymbol.addr;
      write_symbol(output, currAddr, currName);
    }
}
