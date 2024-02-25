#include "defines.h"
#include "code-conversion.h"

dataSet data = {
    .hex = { .code = NULL, .size = 1 },
    .hex_nop = { .code = NULL, .size = 1 },
    .asm_code = { .code = NULL, .size = 1, .line = 1, .jmp_num = 0, .label = NULL },
    .offset = 0,
    .cur_offset = 0
};

void strip_leadntrail_spaces(char *str) {
    int len = strlen(str);
    int i = len - 1;

    while (i >= 0 && (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')) {
        i--;
    }
    str[i + 1] = '\0';
    
    len = strlen(str);
    i = 0;
    while (i < len && (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')) {
        i++;
    }
    memmove(str, str + i, len - i + 1);
}

void strip_all_spaces(char *str) {
	char *modified = str;
	do
		while (*modified == ' ' || *modified == '\t' || *modified == '\n' || *modified == '\r')
			modified++;
	while ((*str++ = *modified++));
}

// Converts all uppercase chars to lowercase
void lowercase(char *str)
{
	do
		if (*str > 0x40 && *str < 0x5B)
			*str += 0x20;
	while (*str++);
}

// Converts all lowercase chars to uppercase
void uppercase(char *str)
{
	do
		if (*str > 0x60)
			*str -= 0x20;
	while (*str++);
}

// Replaces selected char with a null byte
void nullify_char(char *str, char target)
{
	do
		if (*str == target)
		{
			*str = 0;
			return;
		}
	while (*str++);
}

//Standardize instructions
void standardize_instruction(char* str) 
{
	lowercase(str);
	nullify_char(str, ';');
	strip_leadntrail_spaces(str);
	int len = strlen(str);
	char* search_pos;
	search_pos = strchr(str, '[');
	if(search_pos)
	{
		*search_pos = '(';
		search_pos = strchr(str, ']');
		*search_pos = ')';
	}
	search_pos = strstr(str, "ldi");
	if (search_pos) {
		memmove(search_pos + 2, search_pos + 3, strlen(search_pos + 3) + 1);
		search_pos = strstr(str, "hl");
		memmove(search_pos + 3, search_pos + 2, strlen(search_pos + 2));
		memcpy(search_pos, "hli", 3);
		return;
	}
	search_pos = strstr(str, "hl+");
	if (search_pos) {
		*(search_pos + 2) = 'i';
		return;
	}
	search_pos = strstr(str, "ldd");
	if (search_pos) {
		memmove(search_pos + 2, search_pos + 3, strlen(search_pos + 3) + 1);
		search_pos = strstr(str, "hl");
		memmove(search_pos + 3, search_pos + 2, strlen(search_pos + 2));
		memcpy(search_pos, "hld", 3);
		return;
	}
	search_pos = strstr(str, "hl-");
	if (search_pos) {
		*(search_pos + 2) = 'd';
		return;
	}
}
// Changes input values to $xx or $xxyy
char* normalize_param(char *str)
{
	char *hex = calloc(5, sizeof(char));
	do
		if (*str == '$')
		{
			hex[0] = str[1];
			hex[1] = str[2];
			str[1] = 'x';
			str[2] = 'x';
			if (str[3] > ')')
			{
				hex[2] = hex[0];
				hex[3] = hex[1];
				hex[0] = str[3];
				hex[1] = str[4];
				str[3] = 'y';
				str[4] = 'y';
			}
			break;
		}
	while (*str++);
	return hex;
}


// Reads hexadecimal written in ASCII, and replaces each character by its value.
void ascii2hex(char *str, int len) {
	for (int i = 0; i < len; i++) {
		if (str[i] >= '0' && str[i] <= '9') {
			str[i] = str[i] - '0';
		} else if (str[i] >= 'A' && str[i] <= 'F') {
			str[i] = str[i] - 'A' + 10;
		} else if (str[i] >= 'a' && str[i] <= 'f') {
			str[i] = str[i] - 'a' + 10;
		}
	}
}


// Converts an instruction into its hexadecimal equivalent.
char* op2hex(char *opcode, char *param, char *args) {
	char *hex = calloc(7, sizeof(char));
	char opcode_hex[3];
	int cb = 0; // Flags if this instruction is CB-prefixed.
	
	// Scan the list of prefixed instructions.
	for (int x = 0; x < 11; x++)
		if (!strcmp(opcode, cb_set[x]))
		{
			strcat(hex, "cb");
			cb = 1;
		}

	// Check the lookup table for matches
	for (int i = 0; i < 16; i++) {
		for (int k = 0; k < 16; k++) {
			if ((!strcmp(opcode, cb_opcode_table[i][k]) &&
				!strcmp(param, cb_param_table[i][k])) ||
				(!strcmp(opcode, opcode_table[i][k]) &&
				!strcmp(param, param_table[i][k])))
			{
				int index = (i << 4) | k;
				sprintf(opcode_hex, "%02X", index);
				strcat(hex, opcode_hex);
				strcat(hex, args);
				return hex;
			}
		}
	}

	fprintf(stderr, "Couldn't parse [%s %s] on line %d\nInvalid instruction?\n", opcode, param, data.asm_code.line);
	exit(1);
}


// Converts a jump label into a jump address
void jump2addr(char *param, int type)
{
	for (int i = 0; i < data.asm_code.jmp_num; i++)
	{
		if (!strcmp(data.asm_code.label[i].name, param))
		{
			param[0] = '$';
			if (type) {
				if (data.asm_code.label[i].address > data.cur_offset) {
					sprintf(param + 1, "%02X", (data.asm_code.label[i].address - data.cur_offset));
					printf("Param 1: %s\n", param);
				} else {
					sprintf(param + 1, "%02X", (data.asm_code.label[i].address - data.cur_offset - 2) & 0xFF);
				}
			} else {
				sprintf(param + 1, "%04X", data.asm_code.label[i].address);
			}
			param[(type ? 3 : 5)] = 0; //if type = 1 (jr) then 2 char address, if type =0 (jp, call) then full address
		}
	}
}

// Converts hex to asm and prints the results
void hex_to_asm() {
	char buffer[50];
	char buffer_offset[20];
	data.cur_offset = data.offset;
	int hex_len = strlen(data.hex.code);
	// Make modifiable copy of input string
	unsigned char bytes[hex_len + 1]; //terminator not included in len
	strcpy(bytes, data.hex.code);
	// Translate the hex string into a byte array
	ascii2hex(bytes, hex_len);
	// Print origin
	snprintf(buffer_offset, sizeof(buffer_offset), ".org $%04X\n", data.cur_offset);
        data.asm_code.code = (char*)malloc((data.asm_code.size += strlen(buffer_offset)) * sizeof(char));
	strcat(data.asm_code.code, buffer_offset);
	// Loop through each opcode
	for (int i = 0; i < hex_len;) {
		// Split high and low nybble as indices
		char h = bytes[i++];
		char l = bytes[i++];
		data.cur_offset++;
		char cb = 0;
		// Check if current byte is prefix CB
		if (h == 0xC && l == 0xB)
			cb = 1;
		// Print next byte for CB opcodes
		if (cb)
		{
			h = bytes[i++];
			l = bytes[i++];
			data.cur_offset++;
		}
		// Special case for the STOP instruction
		if (h == 1 && l == 0)
		{
			data.cur_offset++;
			i += 2;
		}

		// Mnemonic formatting variables
		char *instruction;
		int param_len = 0;
		int arg_size = 0;
		int offset = 0;

		if (cb)
		{
			instruction = cb_opcode_table[h][l];
			param_len = strlen(cb_param_table[h][l]) + 1;
		}
		else
		{
			instruction = opcode_table[h][l];
			param_len = strlen(param_table[h][l]) + 1;
			arg_size = size_table[h][l];
			offset = offset_table[h][l];
		}

		// Create modifiable copy of parameter string
		char *parameters = malloc(param_len * sizeof(char));
		memset(parameters, 0, param_len);
		if (cb)
			strcpy(parameters, cb_param_table[h][l]);
		else
			strcpy(parameters, param_table[h][l]);

		// Modify mnemonic string if required
		for (int x = 0; x < (arg_size * 2); x++)
		{
			// Swap endianness if necessary
			if (arg_size == 2)
				parameters[offset + ((x + 2) % 4)] = data.hex.code[i];
			else
				parameters[offset + x] = data.hex.code[i];
			bytes[i++];
			if (x % 2)
			{
				data.cur_offset++;
			}
		}
		snprintf(buffer, sizeof(buffer), "%s %s\n", instruction, parameters);
        	data.asm_code.code = (char*)realloc(data.asm_code.code, (data.asm_code.size += strlen(buffer)) * sizeof(char));
		strcat(data.asm_code.code, buffer);
	}
}

void smart_item_processing(char* cur_hex) {
	//If we are currently on item id, not item quantity, and its a single byte asm instruction
	if (data.cur_offset % 2 == 0 && strlen(cur_hex) / 2 == 1) {
		unsigned char hex = strtol(cur_hex, NULL, 16);
		if (!((hex >= 0x00 && hex <= 0x04) || (hex >= 0x7F && hex <= 0xB0) || (hex >= 0xE0 && hex <= 0xE3) || hex == 0xE6 || hex == 0xE7 || (hex >= 0xEF && hex <= 0xF5))) {
			cur_hex = (char*)realloc(cur_hex, (strlen(cur_hex) + 3) * sizeof(char));
			memmove(cur_hex + 2, cur_hex, strlen(cur_hex));
			memcpy(cur_hex, "00", 2);
		}
	}
}

void process_labels(char* str, int alternate) {
	int i = 0;
	char opcode[5] = {0};
	char *param, *args;

	//Cleanup line for processing
	standardize_instruction(str);
	
	// No instruction on this line
	if (!str[i]) {
		data.asm_code.line++;
		return;
	}
	int len = strlen(str);
	
	//Detect origin or labels
	if (str[i] == '.' || strchr(str, ':')) {
		strip_all_spaces(str);
		char* orgptr = strstr(str, ".org");
		if (orgptr) {
			orgptr += 4;
			if (orgptr[0] == '$')
				orgptr++;
			data.offset = strtol(orgptr, NULL, 16);
			data.cur_offset = data.offset;
			data.asm_code.line++;
			return;
		}
		nullify_char(str, ':'); //for the "MyLabel:" type labelling
		data.asm_code.label = realloc(data.asm_code.label, (data.asm_code.jmp_num + 1) * sizeof(Label));
		data.asm_code.label[data.asm_code.jmp_num].address = data.cur_offset;
		data.asm_code.label[data.asm_code.jmp_num].name = calloc(len, sizeof(char));
		strcpy(data.asm_code.label[data.asm_code.jmp_num++].name, (str[i] == '.' ? str + 1 : str));
		data.asm_code.line++;
		return;
	}
	// Fetch the instruction
	for (int k = 0; str[i] > 0x20;)
		opcode[k++] = str[i++];
	// Remove all the remaining spaces
	strip_all_spaces(str + i);
	// Detect comma location and parse accordingly
	if (opcode[0] == 'j' || (opcode[0] == 'c' && opcode[1] == 'a')) { //we can just increase offset and return because we know the bytes and smartprocessing only changes if single byte opcode
		if (opcode[1] == 'r') {
			data.cur_offset += 2;
			data.asm_code.line++;
		} else {
			data.cur_offset += 3;
			data.asm_code.line++;
		}
		return;
	}
	args = normalize_param(str);
	// The rest of the string is parameter data
	param = calloc(len, sizeof(char));
	strcpy(param, str + i);
	// Special case for the STOP instruction
	if (!strcmp(opcode, "stop")) {
		args = "01";
	}
	// Get hex equivalent of instruction
	char* cur_hex = op2hex(opcode, param, args);
	if (alternate != 0) {
		smart_item_processing(cur_hex);
	}

	// These are for jump correction and error handling
	data.cur_offset += strlen(cur_hex) / 2;
	data.asm_code.line++;
	
	// Free allocated string as it goes out of scope.
	if(strcmp(opcode, "stop"))
		free(args);
}

void process_asm(char* str, int alternate) {
	int i = 0;
	char opcode[5] = {0};
	char *param, *args;

	//Cleanup line for processing
	standardize_instruction(str);
	
	//Line is empty or contains label (no instructions)
	if (!str[i] || str[i] == '.' || strchr(str, ':')) {
		data.asm_code.line++;
		return;
	}
	
	// Fetch the instruction
	for (int k = 0; str[i] > 0x20;)
		opcode[k++] = str[i++];
	
	char *period = strchr(str, '.');
    	if (period) {
        	*period = ' ';
    	}
	// Remove all the remaining spaces
	strip_all_spaces(str + i);
	int len = strlen(str);
	
	// Detect comma location and parse accordingly
    	char *comma = strchr(str, ',');
	if (opcode[0] == 'j' || (opcode[0] == 'c' && opcode[1] == 'a')) { //we can just increase offset and return because we know the bytes and smartprocessing only changes if single byte opcode
		if (comma)
	 		jump2addr(comma + 1, (opcode[1] == 'r' ? 1 : 0));
	 	else
	 		jump2addr(str + i, (opcode[1] == 'r' ? 1 : 0));
	}
	args = normalize_param(str);
	// The rest of the string is parameter data
	param = calloc(len, sizeof(char));
	strcpy(param, str + i);
	// Special case for the STOP instruction
	if (!strcmp(opcode, "stop"))
	{
		args = "01";
	}
	// Get hex equivalent of instruction
	char* cur_hex = op2hex(opcode, param, args);
	uppercase(cur_hex);
	if (alternate != 0) {
		smart_item_processing(cur_hex);
		data.hex_nop.code = (char*)realloc(data.hex_nop.code, (data.hex_nop.size += strlen(cur_hex)) * sizeof(char));
		strcat(data.hex_nop.code, cur_hex);
	} else {
		data.hex.code = (char*)realloc(data.hex.code, (data.hex.size += strlen(cur_hex)) * sizeof(char));
		strcat(data.hex.code, cur_hex);
	}

	// These are for jump correction and error handling
	data.cur_offset += strlen(cur_hex) / 2;
	data.asm_code.line++;
	
	// Free allocated string as it goes out of scope.
	if(strcmp(opcode, "stop"))
		free(args);
}

void asm_to_hex(char alternate, char labels) {

	if (alternate == 0) {
		data.hex.code = (char *)calloc(1, sizeof(char));
	} else {
		data.hex_nop.code = (char *)calloc(1, sizeof(char));
	}
	data.cur_offset = data.offset;
	char *cur_pos = data.asm_code.code;
	while (1) {
		char *new_pos = strstr(cur_pos, "\n");
		if (new_pos) {
			// Calculate the length of the substring up to the '\n'
			size_t str_len = new_pos - cur_pos;
			// Allocate memory for the new string
			char *str = malloc(str_len + 1); // +1 for the null terminator
			// Copy characters up to the '\n' into the new string
			strncpy(str, cur_pos, str_len);
			str[str_len] = '\0'; // Null-terminate the new string
			//Process the string
			if (labels == 1)
				process_labels(str, alternate);
			else
				process_asm(str, alternate);
			// Update the original string pointer to the character following the '\n'
			cur_pos = new_pos + 1; //only +1 because '\n' is one char
			free(str);
		} else {
			// If no more '\n' is found, break out of the loop
			break;
		}
        }
	//Process remaining characters
	if (labels == 1) {
		process_labels(cur_pos, alternate);
	} else {
		process_asm(cur_pos, alternate);
		data.asm_code.jmp_num = 0;
	}
	data.asm_code.line = 1;
}

//Parse stdin data
void get_stdin_data(char* raw_data, char* format) {
	if (format == NULL || !(strcmp(format, "hex") == 0 || strcmp(format, "asm") == 0)) {
		format = "hex";
		int i = 0;
		while (raw_data[i] != '\0') {
			if (raw_data[i] == 'l' || raw_data[i] == 'L' || raw_data[i] == 'j' || raw_data[i] == 'J') {
				format = "asm";
				printf("ASM character detected: \"%c\", assuming ASM input\n", raw_data[i]);
				break;
			}
			i++;
		}
		printf("Input format detected as %s!\n", format);
	}
	if (!strcmp(format, "hex")) {
		data.hex.code = raw_data;
		strip_all_spaces(data.hex.code);
		uppercase(data.hex.code);
		if (strlen(data.hex.code) %2 != 0)
		{
			fprintf(stderr, "Error: Hex code should be even in length, but odd detected\n");
			exit(1);
		}
		hex_to_asm();
		asm_to_hex(1, 1);
		asm_to_hex(1, 0);
	}
	else if (!strcmp(format, "asm")) {
		//Set the pointer for asm_code
		for (int i = 0; raw_data[i] != '\0'; i++) {
			if (raw_data[i] == '\\' && raw_data[i + 1] == 'n') {
				raw_data[i] = '\n'; // Replace '\\' with '\n'
				memmove(&raw_data[i + 1], &raw_data[i + 2], strlen(&raw_data[i + 2]) + 1);
			}
		}
		data.asm_code.code = raw_data;
		data.asm_code.label = (Label*)calloc(1, sizeof(Label));
		asm_to_hex(0, 1);
		asm_to_hex(0, 0);
		asm_to_hex(1, 1);
		asm_to_hex(1, 0);
	}
}

// Parse file as input
void get_file_data(char* filename, char* format) {
	
	int file_size;
	size_t result;
	FILE* file = fopen(filename, "r");
	if(!file)
	{
		fprintf(stderr, "Error: specified file \"%s\" does not exist !\n", filename);
		exit(1);
	}
	// Get the file size
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	// Check if file size is less than 1 MB
	if (file_size > 1024 * 1024) {
		fprintf(stderr, "File size exceeds 1 megabyte\n");
		fclose(file);
		exit(1);
	}
	char c;
	if (format == NULL || !(strcmp(format, "hex") == 0 || strcmp(format, "asm") == 0)) {
		format = "hex";
		while ((c = fgetc(file)) != EOF) {
       			if (c == 'l' || c == 'L' || c == 'j' || c == 'J') {
				format = "asm";
				break;
			}
		}
		printf("Input format detected as %s!\n", format);
		rewind(file);
        }
	if (!strcmp(format, "hex")) {
		int length = 1;
		data.hex.code = (char *)malloc(length * sizeof(char));
		while ((c = fgetc(file)) != EOF) {
			if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
				data.hex.code = (char *)realloc(data.hex.code, length * sizeof(char));
				data.hex.code[length - 1] = c;
				length ++;
			}
		}
		data.hex.code[length - 1] = '\0';
		uppercase(data.hex.code);
		if (strlen(data.hex.code) %2 != 0)
		{
			fprintf(stderr, "Error: Hex code should be even in length, but odd detected\n");
			exit(1);
		}
		hex_to_asm();
		asm_to_hex(1, 1);
		asm_to_hex(1, 0);
	}
	else if (!strcmp(format, "asm")) {
		// Allocate memory to hold the file content
		data.asm_code.code = (char *)malloc((file_size + 1) * sizeof(char)); // +1 for null terminator
		if (data.asm_code.code == NULL) {
			fprintf(stderr, "Memory allocation failed.\n");
			fclose(file);
			exit(1);
		}
		// Read the file content into the allocated memory
		result = fread(data.asm_code.code, 1, file_size, file);
		if (result != file_size) {
			fprintf(stderr, "Error reading file.\n");
			fclose(file);
			free(data.asm_code.code);
			exit(1);
		}
		// Null-terminate the buffer
		data.asm_code.code[file_size] = '\0';
		data.asm_code.label = (Label*)calloc(1, sizeof(Label));
		asm_to_hex(0, 1);
		asm_to_hex(0, 0);
		asm_to_hex(1, 1);
		asm_to_hex(1, 0);
	}
	fclose(file);
}

//autodetect data input
dataSet generate_code(char* raw_data, int origin, char* format) {
	int len = strlen(raw_data);
	//check the 5th to last -> 2nd to last character for a "." to detect filename (.asm, .a, etc)
	if (raw_data[len - 2] == '.' || raw_data[len - 3] == '.' || raw_data[len - 4] == '.' || raw_data[len - 5] == '.')
		get_file_data(raw_data, format);
	else 
		get_stdin_data(raw_data, format);

	return data;
}
