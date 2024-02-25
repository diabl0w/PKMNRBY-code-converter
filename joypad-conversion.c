#include "defines.h"
// Converts hex string into joypad values for use with Full Control method
// http://forums.glitchcity.info/index.php?topic=7744.0
unsigned char joy_vals[4] = {8, 4, 2, 1};
char *joy_high[4] = {"DOWN", "UP", "LEFT", "RIGHT"};
char *joy_low[4] = {"START", "SELECT", "B", "A"};

void hex_to_joy(char* str)
{
	int len = strlen(str);

	// Make modifiable copy of input string
	char bytes[len + 1]; //terminator not included in len
	strcpy(bytes, str);
	
	ascii2hex(bytes, len);

	// Placeholder for the last button value
	char *last;

	printf(BLUE"Full Joypad Writer\n");
	printf("==================\n\n"BLACK);
	printf("Joypad Values:\n");

	// Print an initial A to skip the junk byte
	printf("A\n");

	// Total number of button presses, just for funsies
	// 3 = The initial A and the ending START + SELECT
	int presses = 3;

	for (int i = 0; i < len;)
	{
		char p14 = bytes[i++];
		char p15 = bytes[i++];
		char *buttons[12] = {0};
		int index = 0;

		// Collect all D-Pad values
		for (int x = 0; x < 4; x++)
			if (joy_vals[x] <= p14)
			{
				buttons[index++] = joy_high[x];
				p14 -= joy_vals[x];
				presses++;
			}

		// Collect all other values
		for (int x = 0; x < 4; x++)
			if (joy_vals[x] <= p15)
			{
				buttons[index++] = joy_low[x];
				p15 -= joy_vals[x];
				presses++;
			}

		// Check if 1st button = NEXT code
		if (buttons[0] == last)
			// Swap buttons if more than 1 exists
			if (buttons[1] != 0)
			{
				char *tmp = buttons[0];
				buttons[0] = buttons[1];
				buttons[1] = tmp;
			}
			// Overflow the byte for correction if only 1 exists
			else
				if (last == "DOWN")
				{
					char *tmp = buttons[0];
					buttons[0] = "UP";
					buttons[1] = "DOWN";
					buttons[2] = "UP";
					buttons[3] = tmp;
					index += 3;
					presses += 3;
				}
				else
				{
					char *tmp = buttons[0];
					buttons[0] = "DOWN";
					buttons[1] = tmp;
					buttons[2] = "DOWN";
					presses += 2;
					index += 2;
				}

		// Keep track of the last button for correction
		if (index)
			last = buttons[index - 1];

		// Print out the button combination for this byte
		for (int x = 0; x < index; x++)
			printf("%s ", buttons[x]);
		printf("%s\n", last);
		presses++;
	}

	// Print the EXIT code and number of button presses
	printf("START + SELECT\n\n");
	printf("Total number of button presses: %d\n\n", presses);
}
