#include "defines.h"
#include "item-conversion.h"
// Struct for holding error detections
struct Error {
	unsigned char key_quantity;
	unsigned char duplicates;
	unsigned char glitches;
};

// Converts hex string into Gen I items
void hex_to_item(char *str)
{
	// Error handler for weird item setups
	struct Error errors = {0};
	
	// Remove spaces and calculate length
	int len = strlen(str);

	// Make modifiable copy of input string
	char bytes[len + 1]; //terminator not included in len
	strcpy(bytes, str);

	unsigned char seen_items[16][16] = {0};

	// Warning placeholders
	unsigned char glitch_items = 0;
	unsigned char multi_key_items = 0;
	unsigned char duplicate_items = 0;

	printf(BLUE "\nItem            Quantity\n");
	printf("========================\n" BLACK);

	for (int i = 0; i < len;)
	{
		// Prep bytes for lookup
		ascii2hex(bytes + i, 2);

		// Place horizontal cursor at line start
		int h_cursor = 0;
		char h = bytes[i++];
		char l = bytes[i++];
		char *item = gen1_items[h][l];
		char *quantity = calloc(4, sizeof(char));
		unsigned char conversion = 0;

		h_cursor += strlen(item);

		// Grab next byte as quantity
		if (str[i])
		{
			ascii2hex(bytes + i, 2);
			conversion = bytes[i++] << 4;
			conversion |= bytes[i++];
			sprintf(quantity, "%d", conversion);
		}
		// Set as "Any" if any quantity will do
		else
			quantity = "Any";

		// Print item/quantity pairs
		printf("%s", item);
		for (; h_cursor < 16; h_cursor++)
			printf(" ");
		printf("x%s\n", quantity);

		// [Error] Key items with 2+ quantity
		if (gen1_key_items[h][l])
			if (conversion && conversion != 1)
				errors.key_quantity = 1;
		// [Error] Glitch items
		if (gen1_glitch_items[h][l])
			errors.glitches = 1;
		// [Error] Duplicate item stacks
		if (seen_items[h][l] == 1)
			errors.duplicates = 1;
		else
			seen_items[h][l] = 1;
	}

	// Print errors
	if (errors.key_quantity || errors.glitches || errors.duplicates)
	{
		printf("\n\n-- WARNING! --\n");
		if (errors.duplicates)
			printf(" * Duplicate item stacks detected!\n");
		if (errors.key_quantity)
			printf(" * Key (non-tossable) items detected!\n");
		if (errors.glitches)
			printf(" * Glitch items detected!\n");
	}
}
