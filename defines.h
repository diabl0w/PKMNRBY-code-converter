#ifndef DEFINES_H
#define DEFINES_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	unsigned int address;
	char* name;
} Label;

typedef struct {
	char* code;
	int size;
	int line;
	int jmp_num;
	Label* label;
} ASM_code;

typedef struct {
	char* code;
	int size;
} HEX_code;

typedef struct {
	HEX_code hex;
	HEX_code hex_nop;
	ASM_code asm_code;
	int offset;
	int cur_offset;
} dataSet;

dataSet generate_code(char*, int, char*);
void ascii2hex(char *str, int len);
void hex_to_joy(char*);
void hex_to_nick(char*);
void hex_to_item(char*);

// ANSI color codes
#define RED     "\x1b[1;31m"
#define GREEN   "\x1b[1;32m"
#define YELLOW  "\x1b[1;33m"
#define BLUE    "\x1b[1;34m"
#define MAGENTA "\x1b[1;35m"
#define CYAN    "\x1b[1;36m"
#define BLACK "\x1b[0m"
#endif
