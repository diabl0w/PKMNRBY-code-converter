#include "defines.h"
char *banner =
" ____  _  __ _      _      ____  ____ ___  _\n"
"/  __\\/ |/ // \\__/|/ \\  /|/  __\\/  __\\\\  \\//\n"
"|  \\/||   / | |\\/||| |\\ |||  \\/|| | // \\  /\n"
"|  __/|   \\ | |  ||| | \\|||    /| |_\\\\ / /\n"
"\\_/   \\_|\\_\\\\_/  \\|\\_/  \\|\\_/\\_\\\\____//_/\n"
" ____  ____  ____  _____   \n"  
"/   _\\/  _ \\/  _ \\/  __/\n"     
"|  /  | / \\|| | \\||  \\ _____\n" 
"|  \\_ | \\_/|| |_/||  /_\\____\\\n"
"\\____/\\____/\\____/\\____\\\n"
"____  ____  _      _     _____ ____  _____  _____ ____\n"
"/   _\\/  _ \\/ \\  /|/ \\ |\\/  __//  __\\/__ __\\/  __//  __\\ \n"
"|  /  | / \\|| |\\ ||| | //|  \\  |  \\/|  / \\  |  \\  |  \\/|\n"
"|  \\_ | \\_/|| | \\||| \\// |  /_ |    /  | |  |  /_ |    /\n"
"\\____/\\____/\\_/  \\|\\__/  \\____\\\\_/\\_\\  \\_/  \\____\\\\_/\\_\\ \n";

void usage(char *str)
{
	printf("Usage: %s [options] [hex]\n\n", str);
	printf("Options:\n");
	printf("  -o format    Display output in a specific format\n");
	printf("  -org origin  Specify memory offset to display in asm format.\n");
	printf("                  (Ignored in other formats)\n");
	printf("  -h           Print this help message and exit\n");
	printf("  -v           Print version information and exit\n\n");
	printf("Output Formats:\n");
	printf("  asm          GB-Z80 assembly language\n");
	printf("  hex          Hexadecimal machine code format\n");
	printf("  nickname     Generate Nickname writer codes\n");
	printf("  joypad       Joypad values (http://forums.glitchcity.info/index.php?topic=7744.0)\n");
	printf("  item         R/B/Y item codes for use with ACE\n");
	printf("Examples:\n");
	printf("  %s EA14D7C9\n", str);
	printf("  %s -i hex -o asm bgb_mem.dump\n", str);
	printf("  %s -o hex zzazz.asm\n", str);
	printf("  %s -o item 0E1626642EBB4140CDD635C9\n", str);
	printf("  %s -o nickname coin_case.asm\n", str);
	exit(0);
}

int main(int argc, char* argv[])
{
	printf(RED"\n**************************************************************\n");
	printf(BLUE"%s\n", banner);
	printf(RED"**************************************************************\n"BLACK);
	int origin = 0;
	char* raw_data = NULL;
	char* output = NULL;
	char* input = NULL;

	// Show help
	if (argc == 1)
		usage(argv[0]);

	// Parse arguments
	for (int i = 0; i < argc; i++)
	{
		if (!strcmp(argv[i], "-v"))
		{
			printf(RED "PKMNRBY-code-converter\n" BLACK);
			printf("Version 2.0\n\n");
			printf("by diabl0w\n");
			printf("Original Core Logic by KernelEquinox\n");
			printf("Nickname Writer Code Generator by TimOS\n\n");
			printf(BLUE "Homepage : https://github.com/diabl0w/PKMNRBY-code-converter/\n\n" BLACK);
			exit(0);
		}
		else if (!strcmp(argv[i], "-h")) 
			usage(argv[0]);
		else if (!strcmp(argv[i], "-i"))
		{
			input = argv[++i];
			if(i == argc || !(strcmp(input, "hex") == 0 || strcmp(input, "asm") == 0)) 
			{
				printf(RED "Error: \"-i\" expects \"asm\" or \"asm\" format specifiers\n" BLACK);
				exit(1);
			}
		}
		else if (!strcmp(argv[i], "-o"))
		{
			output = argv[++i];
			if(strcmp(input, "all") != 0 && strcmp(input, "asm") != 0 && strcmp(input, "hex") != 0 && strcmp(input, "item") != 0 && strcmp(input, "joypad") != 0 && strcmp(input, "nickname") != 0)
			{
				fprintf(stderr, "Error: no valid argument found for option\"-o\"\n");
				exit(1);
			}
		}
		else if (!strcmp(argv[i], "-org"))
		{
			if (!sscanf(argv[i], "%4X", &origin))
			{
				fprintf(stderr, "Error: \'-org\" expects a hexadecimal origin offset (ie. D322 or 1f49)\n");
				exit(1);
			}
		}
		else
			raw_data = argv[i];
	}

	dataSet data = generate_code(raw_data, origin, input);
	// Print results
	if (output) 
	{
		if (!strcmp(output, "item")) {
			printf(BLUE"Smart Item List:\n"BLACK);	
			hex_to_item(data.hex_nop.code);
			printf(RED"\n*********************************************************\n"BLACK);
			printf(BLUE"Standard Item List:\n"BLACK);	
			hex_to_item(data.hex.code);
		}
		else if (!strcmp(output, "nickname"))
			hex_to_nick(data.hex.code);
		else if (!strcmp(output, "joypad"))
			hex_to_joy(data.hex.code);
		else if (!strcmp(output, "asm"))
			printf(BLUE "\nAssembly Code:\n" BLACK "%s", data.asm_code.code);	
		else if (!strcmp(output, "hex"))
			printf(BLUE "\nMachine code: " BLACK "%s\n", data.hex.code);
	}
	else 
	{
		printf(RED"\n*********************************************************\n"BLACK);
		printf(BLUE "Assembly Code:\n" BLACK "%s", data.asm_code.code);	
		printf(RED"\n*********************************************************\n"BLACK);
		hex_to_joy(data.hex.code);
		printf(RED"\n*********************************************************\n"BLACK);
		hex_to_nick(data.hex.code);
		printf(RED"\n*********************************************************\n"BLACK);
		printf(BLUE"Smart Item List:\n"BLACK);	
		hex_to_item(data.hex_nop.code);
		printf(RED"\n*********************************************************\n"BLACK);
		printf(BLUE"Standard Item List:\n"BLACK);	
		hex_to_item(data.hex.code);
		printf(RED"\n*********************************************************\n"BLACK);
		printf(BLUE "Machine code: " BLACK "%s\n", data.hex.code);	
	}
	printf("\n");
	return 0;
}
