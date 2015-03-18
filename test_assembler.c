#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <CUnit/Basic.h>

#include "src/utils.h"
#include "src/tables.h"
#include "src/translate_utils.h"
#include "src/translate.h"

const char* TMP_FILE = "test_output.txt";
const int BUF_SIZE = 1024;

/****************************************
 *  Helper functions 
 ****************************************/

int do_nothing() {
    return 0;
}

int init_log_file() {
    set_log_file(TMP_FILE);
    return 0;
}

int check_lines_equal(char **arr, int num) {
    char buf[BUF_SIZE];

    FILE *f = fopen(TMP_FILE, "r");
    if (!f) {
        CU_FAIL("Could not open temporary file");
        return 0;
    }
    for (int i = 0; i < num; i++) {
        if (!fgets(buf, BUF_SIZE, f)) {
            CU_FAIL("Reached end of file");
            return 0;
        }
        CU_ASSERT(!strncmp(buf, arr[i], strlen(arr[i])));
    }
    fclose(f);
    return 0;
}

/****************************************
 *  Test cases for translate_utils.c 
 ****************************************/

void test_translate_reg() {
    CU_ASSERT_EQUAL(translate_reg("$0"), 0);
    CU_ASSERT_EQUAL(translate_reg("$at"), 1);
    CU_ASSERT_EQUAL(translate_reg("$v0"), 2);
    CU_ASSERT_EQUAL(translate_reg("$a0"), 4);
    CU_ASSERT_EQUAL(translate_reg("$a1"), 5);
    CU_ASSERT_EQUAL(translate_reg("$a2"), 6);
    CU_ASSERT_EQUAL(translate_reg("$a3"), 7);
    CU_ASSERT_EQUAL(translate_reg("$t0"), 8);
    CU_ASSERT_EQUAL(translate_reg("$t1"), 9);
    CU_ASSERT_EQUAL(translate_reg("$t2"), 10);
    CU_ASSERT_EQUAL(translate_reg("$t3"), 11);
    CU_ASSERT_EQUAL(translate_reg("$s0"), 16);
    CU_ASSERT_EQUAL(translate_reg("$s1"), 17);
    CU_ASSERT_EQUAL(translate_reg("$3"), -1);
    CU_ASSERT_EQUAL(translate_reg("asdf"), -1);
    CU_ASSERT_EQUAL(translate_reg("hey there"), -1);
}

void test_translate_num() {
    long int output;

    CU_ASSERT_EQUAL(translate_num(&output, "35", -1000, 1000), 0);
    CU_ASSERT_EQUAL(output, 35);
    CU_ASSERT_EQUAL(translate_num(&output, "145634236", 0, 9000000000), 0);
    CU_ASSERT_EQUAL(output, 145634236);
    CU_ASSERT_EQUAL(translate_num(&output, "0xC0FFEE", -9000000000, 9000000000), 0);
    CU_ASSERT_EQUAL(output, 12648430);
    CU_ASSERT_EQUAL(translate_num(&output, "72", -16, 72), 0);
    CU_ASSERT_EQUAL(output, 72);
    CU_ASSERT_EQUAL(translate_num(&output, "72", -16, 71), -1);
    CU_ASSERT_EQUAL(translate_num(&output, "72", 72, 150), 0);
    CU_ASSERT_EQUAL(output, 72);
    CU_ASSERT_EQUAL(translate_num(&output, "72", 73, 150), -1);
    CU_ASSERT_EQUAL(translate_num(&output, "35x", -100, 100), -1);
}

/****************************************
 *  Test cases for tables.c 
 ****************************************/

void test_table_1() {
    int retval;

    SymbolTable* tbl = create_table(SYMTBL_UNIQUE_NAME);
    CU_ASSERT_PTR_NOT_NULL(tbl);

    retval = add_to_table(tbl, "abc", 8);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "efg", 12);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "q45", 16);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "q45", 24); 
    CU_ASSERT_EQUAL(retval, -1); 
    retval = add_to_table(tbl, "bob", 14); 
    CU_ASSERT_EQUAL(retval, -1); 

    retval = get_addr_for_symbol(tbl, "abc");
    CU_ASSERT_EQUAL(retval, 8); 
    retval = get_addr_for_symbol(tbl, "q45");
    CU_ASSERT_EQUAL(retval, 16); 
    retval = get_addr_for_symbol(tbl, "ef");
    CU_ASSERT_EQUAL(retval, -1);
    
    free_table(tbl);

    char* arr[] = { "Error: name 'q45' already exists in table.",
                    "Error: address is not a multiple of 4." };
    check_lines_equal(arr, 2);

    SymbolTable* tbl2 = create_table(SYMTBL_NON_UNIQUE);
    CU_ASSERT_PTR_NOT_NULL(tbl2);

    retval = add_to_table(tbl2, "q45", 16);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl2, "q45", 24); 
    CU_ASSERT_EQUAL(retval, 0);

    free_table(tbl2);
}

void test_table_2() {
    int retval, max = 100;

    SymbolTable* tbl = create_table(SYMTBL_UNIQUE_NAME);
    CU_ASSERT_PTR_NOT_NULL(tbl);

    char buf[10];
    for (int i = 0; i < max; i++) {
        sprintf(buf, "%d", i);
        retval = add_to_table(tbl, buf, 4 * i);
        CU_ASSERT_EQUAL(retval, 0);
    }

    for (int i = 0; i < max; i++) {
        sprintf(buf, "%d", i);
        retval = get_addr_for_symbol(tbl, buf);
        CU_ASSERT_EQUAL(retval, 4 * i);
    }

    free_table(tbl);
}


void test_addu() {
    uint8_t funct = 0x21;
    int val;
    FILE* put1 = fopen("file.txt", "w");
    char* args0[3];
    args0[0] = "$t0";
    args0[1] = "$t1";
    args0[2] = "$x3";
    size_t num_args0 = 3;
    val = write_rtype(funct, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval;
    FILE* output = fopen("file.txt", "w");
    char* args[3];
    args[0] = "$t0";
    args[1] = "$t1";
    args[2] = "$t2";
    size_t num_args = 2;
    retval = write_rtype(funct, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("file.txt", "w");
    char* args1[3];
    args1[0] = "$t0";
    args1[1] = "$t1";
    args1[2] = "$t2";
    size_t num_args1 = 4;
    retval1 = write_rtype(funct, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("file.txt", "w");
    char* args2[3];
    args2[0] = "$t0";
    args2[1] = "$t1";
    args2[2] = "$t2";
    size_t num_args2 = 3;
    retval2 = write_rtype(funct, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
}

void test_or() {
    int retval;
    uint8_t funct = 0x25;
    int val;
    FILE* put1 = fopen("boo.txt", "w");
    char* args0[3];
    args0[0] = "$t0";
    args0[1] = "$t1";
    args0[2] = "$x2";
    size_t num_args0 = 3;
    val = write_rtype(funct, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    FILE* output = fopen("boo.txt", "w");
    char* args[3];
    args[0] = "$t0";
    args[1] = "$t1";
    args[2] = "$t2";
    size_t num_args = 2;
    retval = write_rtype(funct, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "$s2";
    size_t num_args1 = 4;
    retval1 = write_rtype(funct, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[3];
    args2[0] = "$s0";
    args2[1] = "$s1";
    args2[2] = "$s2";
    size_t num_args2 = 3;
    retval2 = write_rtype(funct, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
}

void test_slt() {
    int retval;
    uint8_t funct = 0x2a;
    FILE* output = fopen("boo.txt", "w");
    char* args[3];
    args[0] = "$t1";
    args[1] = "$t2";
    args[2] = "$t3";
    size_t num_args = 2;
    retval = write_rtype(funct, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int val;
    FILE* put1 = fopen("file.txt", "w");
    char* args0[3];
    args0[0] = "$t0";
    args0[1] = "$t1";
    args0[2] = "$x2";
    size_t num_args0 = 3;
    val = write_rtype(funct, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "$s2";
    size_t num_args1 = 4;
    retval1 = write_rtype(funct, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[3];
    args2[0] = "$s0";
    args2[1] = "$s1";
    args2[2] = "$s2";
    size_t num_args2 = 3;
    retval2 = write_rtype(funct, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
}

void test_sltu() {
    uint8_t funct = 0x2b;
    int val;
    FILE* put1 = fopen("boo.txt", "w");
    char* args0[3];
    args0[0] = "$t0";
    args0[1] = "$t1";
    args0[2] = "$x2";
    size_t num_args0 = 3;
    val = write_rtype(funct, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval;
    FILE* output = fopen("boo.txt", "w");
    char* args[3];
    args[0] = "$t1";
    args[1] = "$t2";
    args[2] = "$t3";
    size_t num_args = 2;
    retval = write_rtype(funct, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "$s2";
    size_t num_args1 = 4;
    retval1 = write_rtype(funct, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[3];
    args2[0] = "$s0";
    args2[1] = "$s1";
    args2[2] = "$s2";
    size_t num_args2 = 3;
    retval2 = write_rtype(funct, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
}

void test_sll() {
    uint8_t funct = 0x00;
    int val;
    FILE* put1 = fopen("boo.txt", "w");
    char* args0[3];
    args0[0] = "$x0";
    args0[1] = "$t1";
    args0[2] = "0x2";
    size_t num_args0 = 3;
    val = write_shift(funct, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval;
    FILE* output = fopen("boo.txt", "w");
    char* args[3];
    args[0] = "$t1";
    args[1] = "$t2";
    args[2] = "0x2";
    size_t num_args = 1;
    retval = write_shift(funct, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[3];
    args1[0] = "$s0";
    args1[1] = "$s1";
    args1[2] = "0x2";
    size_t num_args1 = 7;
    retval1 = write_shift(funct, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[3];
    args2[0] = "$s0";
    args2[1] = "$s1";
    args2[2] = "0x15";
    size_t num_args2 = 3;
    retval2 = write_shift(funct, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
}


void test_jr() {
    uint8_t funct = 0x08;
    int val;
    FILE* put1 = fopen("boo.txt", "w");
    char* args0[1];
    args0[0] = "$x0";
    size_t num_args0 = 1;
    val = write_jr(funct, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval;
    FILE* output = fopen("boo.txt", "w");
    char* args[1];
    args[0] = "$t1";
    size_t num_args = -1;
    retval = write_jr(funct, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[1];
    args1[0] = "$s0";
    size_t num_args1 = 2;
    retval1 = write_jr(funct, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[1];
    args2[0] = "$ra";
    size_t num_args2 = 1;
    retval2 = write_jr(funct, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
}

void test_addiu() {
    uint8_t opcode = 0x9; 
    int val;
    FILE* put1 = fopen("boo.txt", "w");
    char* args0[3];
    args0[0] = "$t0";
    args0[1] = "$b1";
    args0[2] = "3";
    size_t num_args0 = 3;
    val = write_addiu(opcode, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval;
    FILE* output = fopen("boo.txt", "w");
    char* args[3];
    args[0] = "$t0";
    args[1] = "$t1";
    args[2] = "3";
    size_t num_args = 2;
    retval = write_addiu(opcode, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[3];
    args1[0] = "$t0";
    args1[1] = "$t1";
    args1[2] = "3";
    size_t num_args1 = 4;
    retval1 = write_addiu(opcode, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[3];
    args2[0] = "$t0";
    args2[1] = "$t1";
    args2[2] = "2";
    size_t num_args2 = 3;
    retval2 = write_addiu(opcode, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
}

void test_ori() {
    uint8_t opcode = 0xd; 
    int val;
    FILE* put1 = fopen("boo.txt", "w");
    char* args0[3];
    args0[0] = "$t0";
    args0[1] = "$b1";
    args0[2] = "3";
    size_t num_args0 = 3;
    val = write_ori(opcode, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval;
    FILE* output = fopen("boo.txt", "w");
    char* args[3];
    args[0] = "$t0";
    args[1] = "$t1";
    args[2] = "3";
    size_t num_args = 2;
    retval = write_ori(opcode, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[3];
    args1[0] = "$t0";
    args1[1] = "$t1";
    args1[2] = "3";
    size_t num_args1 = 4;
    retval1 = write_ori(opcode, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[3];
    args2[0] = "$t0";
    args2[1] = "$t1";
    args2[2] = "2";
    size_t num_args2 = 3;
    retval2 = write_ori(opcode, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
}

void test_lui() {
    uint8_t opcode = 0xf; 
    int val;
    FILE* put1 = fopen("boo.txt", "w");
    char* args0[2];
    args0[0] = "$d0";
    args0[1] = "3";
    size_t num_args0 = 2;
    val = write_lui(opcode, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval;
    FILE* output = fopen("boo.txt", "w");
    char* args[2];
    args[0] = "$t0";
    args[1] = "3";
    size_t num_args = 1;
    retval = write_lui(opcode, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[2];
    args1[0] = "$t0";
    args1[1] = "3";
    size_t num_args1 = 7;
    retval1 = write_lui(opcode, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[2];
    args2[0] = "$t0";
    args2[1] = "2";
    size_t num_args2 = 2;
    retval2 = write_lui(opcode, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
}

void test_lb() {
    uint8_t opcode = 0x20; 
    int val;
    FILE* put1 = fopen("boo.txt", "w");
    char* args0[3];
    args0[0] = "$d0";
    args0[1] = "$t0";
    args0[2] = "3";
    size_t num_args0 = 3;
    val = write_mem(opcode, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval;
    FILE* output = fopen("boo.txt", "w");
    char* args[3];
    args0[0] = "$t0";
    args0[1] = "$t0";
    args0[2] = "3";
    size_t num_args = 1;
    retval = write_mem(opcode, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[3];
    args0[0] = "$t0";
    args0[1] = "$t1";
    args0[2] = "3";
    size_t num_args1 = 7;
    retval1 = write_mem(opcode, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[3];
    args2[0] = "$t0";
    args2[1] = "3";
    args2[2] = "$t1";
    size_t num_args2 = 3;
    retval2 = write_mem(opcode, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
 }

  void test_lw() {
    uint8_t opcode = 0x23;
    int val;
    FILE* put1 = fopen("boo.txt", "w");
    char* args0[3];
    args0[0] = "$d0";
    args0[1] = "$t0";
    args0[2] = "4";
    size_t num_args0 = 3;
    val = write_mem(opcode, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval;
    FILE* output = fopen("boo.txt", "w");
    char* args[3];
    args0[0] = "$t0";
    args0[1] = "$t0";
    args0[2] = "4";
    size_t num_args = 1;
    retval = write_mem(opcode, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[3];
    args0[0] = "$t0";
    args0[1] = "$t1";
    args0[2] = "4";
    size_t num_args1 = 7;
    retval1 = write_mem(opcode, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[3];
    args2[0] = "$t0";
    args2[1] = "4";
    args2[2] = "$t1";
    size_t num_args2 = 3;
    retval2 = write_mem(opcode, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
 }


  void test_sb() {
    uint8_t opcode = 0x28;
    int val;
    FILE* put1 = fopen("boo.txt", "w");
    char* args0[3];
    args0[0] = "$d0";
    args0[1] = "$t0";
    args0[2] = "4";
    size_t num_args0 = 3;
    val = write_mem(opcode, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval;
    FILE* output = fopen("boo.txt", "w");
    char* args[3];
    args0[0] = "$t0";
    args0[1] = "$t0";
    args0[2] = "4";
    size_t num_args = 1;
    retval = write_mem(opcode, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[3];
    args0[0] = "$t0";
    args0[1] = "$t1";
    args0[2] = "4";
    size_t num_args1 = 7;
    retval1 = write_mem(opcode, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[3];
    args2[0] = "$t0";
    args2[1] = "4";
    args2[2] = "$t1";
    size_t num_args2 = 3;
    retval2 = write_mem(opcode, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
 }


 void test_sw() {
    uint8_t opcode = 0x2b;
    int val;
    FILE* put1 = fopen("boo.txt", "w");
    char* args0[3];
    args0[0] = "$d0";
    args0[1] = "$t0";
    args0[2] = "4";
    size_t num_args0 = 3;
    val = write_mem(opcode, put1, args0, num_args0);
    CU_ASSERT_EQUAL(val, -1);
    fclose(put1);
    int retval;
    FILE* output = fopen("boo.txt", "w");
    char* args[3];
    args0[0] = "$t0";
    args0[1] = "$t0";
    args0[2] = "4";
    size_t num_args = 1;
    retval = write_mem(opcode, output, args, num_args);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(output);
    int retval1;
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[3];
    args0[0] = "$t0";
    args0[1] = "$t1";
    args0[2] = "4";
    size_t num_args1 = 7;
    retval1 = write_mem(opcode, output1, args1, num_args1);
    CU_ASSERT_EQUAL(retval1, -1);
    fclose(output1);
    int retval2;
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[3];
    args2[0] = "$t0";
    args2[1] = "4";
    args2[2] = "$t1";
    size_t num_args2 = 3;
    retval2 = write_mem(opcode, output2, args2, num_args2);
    CU_ASSERT_EQUAL(retval2, 0);
    fclose(output2);
 }

  void test_li() {
    FILE* output = fopen("boo.txt", "w");
    char* name = "li";
    char* args[2];
    args[0] = "$t0";
    args[1] = "5";
    int num_args = 1;
    unsigned answer =  write_pass_one(output, name, args, num_args);
    CU_ASSERT_EQUAL(answer, 0);
    fclose(output);
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[2];
    args1[0] = "$s0";
    args1[1] = "5";
    int num_args1 = 5;
    unsigned answer1 =  write_pass_one(output1, name, args1, num_args1);
    CU_ASSERT_EQUAL(answer1, 0);
    fclose(output1);
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[2];
    args2[0] = "$t0";
    args2[1] = "546456456546745745";
    int num_args2 = 5;
    unsigned answer2 =  write_pass_one(output2, name, args2, num_args2);
    CU_ASSERT_EQUAL(answer2, 0);
    fclose(output2);
    FILE* output3 = fopen("boo.txt", "w");
    char* args3[2];
    args3[0] = "$v0";
    args3[1] = "100";
    int num_args3 = 2;
    unsigned answer3 =  write_pass_one(output3, name, args3, num_args3);
    CU_ASSERT_EQUAL(answer3, 2);
    fclose(output3);
    FILE* output4 = fopen("boo.txt", "w");
    char* args4[2];
    args4[0] = "$a0";
    args4[1] = "3453639";
    int num_args4 = 2;
    unsigned answer4 =  write_pass_one(output4, name, args4, num_args4);
    CU_ASSERT_EQUAL(answer4, 2);
    fclose(output4);
 }

   void test_blt() {
    FILE* output = fopen("boo.txt", "w");
    char* name = "blt";
    char* args[3];
    args[0] = "$t0";
    args[1] = "$t1";
    args[2] = "523";
    int num_args = 1;
    unsigned answer =  write_pass_one(output, name, args, num_args);
    CU_ASSERT_EQUAL(answer, 0);
    fclose(output);
    FILE* output1 = fopen("boo.txt", "w");
    char* args1[3];
    args1[0] = "$t0";
    args1[1] = "$t0";
    args1[2] = "523";
    int num_args1 = 5;
    unsigned answer1 =  write_pass_one(output1, name, args1, num_args1);
    CU_ASSERT_EQUAL(answer1, 0);
    fclose(output1);
    FILE* output2 = fopen("boo.txt", "w");
    char* args2[3];
    args2[0] = "$t0";
    args2[1] = "$t0";
    args2[2] = "523";
    int num_args2 = -1;
    unsigned answer2 =  write_pass_one(output2, name, args2, num_args2);
    CU_ASSERT_EQUAL(answer2, 0);
    fclose(output2);
    FILE* output3 = fopen("file.txt", "w");
    char* args3[3];
    args3[0] = "$rs";
    args3[1] = "$rt";
    args3[2] = "Label";
    int num_args3 = 3;
    unsigned answer3 =  write_pass_one(output3, name, args3, num_args3);
    CU_ASSERT_EQUAL(answer3, 2);
    fclose(output3);
 }


/****************************************
 *  Add your test cases here
 ****************************************/

int main(int argc, char** argv) {
    CU_pSuite pSuite1 = NULL, pSuite2 = NULL;
    CU_pSuite pSuite3 = NULL, pSuite4 = NULL;



    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    /* Suite 1 */
    pSuite1 = CU_add_suite("Testing translate_utils.c", NULL, NULL);
    if (!pSuite1) {
        goto exit;
    }
    if (!CU_add_test(pSuite1, "test_translate_reg", test_translate_reg)) {
        goto exit;
    }
    if (!CU_add_test(pSuite1, "test_translate_num", test_translate_num)) {
        goto exit;
    }

    /* Suite 2 */
    pSuite2 = CU_add_suite("Testing tables.c", init_log_file, NULL);
    if (!pSuite2) {
        goto exit;
    }
    if (!CU_add_test(pSuite2, "test_table_1", test_table_1)) {
        goto exit;
    }
    if (!CU_add_test(pSuite2, "test_table_2", test_table_2)) {
        goto exit;
    }

   /* Suite 3 */
    pSuite3 = CU_add_suite("Testing translate.c: PART N1", NULL, NULL);
    if (!pSuite3) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "addu instruction", test_addu)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "or instruction", test_or)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "slt instruction", test_slt)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "sltu instruction", test_sltu)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "sll instruction", test_sll)) {
        goto exit;
    }
     if (!CU_add_test(pSuite3, "jr instruction", test_jr)) {
        goto exit;
    }
     if (!CU_add_test(pSuite3, "addiu instruction", test_addiu)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "ori instruction", test_ori)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "lui instruction", test_lui)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "lb instruction", test_lb)) {
        goto exit;
    }
     if (!CU_add_test(pSuite3, "lw instruction", test_lw)) {
        goto exit;
    }
     if (!CU_add_test(pSuite3, "sb instruction", test_sb)) {
        goto exit;
    }
    if (!CU_add_test(pSuite3, "sw instruction", test_sw)) {
        goto exit;
    }

    /* Suite 4 */
    pSuite4 = CU_add_suite("Testing translate.c: PART N2", NULL, NULL);
    if (!pSuite4) {
        goto exit;
    }
    if (!CU_add_test(pSuite4, "li instruction", test_li)) {
        goto exit;
    }
    if (!CU_add_test(pSuite4, "blt instruction", test_blt)) {
        goto exit;
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

exit:
    CU_cleanup_registry();
    return CU_get_error();;
}