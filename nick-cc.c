#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define min(a, b) ((a) < (b) ? (a) : (b))

// Define the structure for character set entry
typedef struct {
    const char *key;
    int value;
    int column;
    int row;
    int screen;
} CharSetEntry;

// Define the character set
CharSetEntry charSet[] = {
{"_", 0x7F, 8, 2, 0}, {"A", 0x80, 0, 0, 1}, {"B", 0x81, 1, 0, 1}, {"C", 0x82, 2, 0, 1}, {"D", 0x83, 3, 0, 1}, {"E", 0x84, 4, 0, 1}, {"F", 0x85, 5, 0, 1}, {"G", 0x86, 6, 0, 1}, {"H", 0x87, 7, 0, 1}, {"I", 0x88, 8, 0, 1}, {"J", 0x89, 0, 1, 1}, {"K", 0x8A, 1, 1, 1}, {"L", 0x8B, 2, 1, 1}, {"M", 0x8C, 3, 1, 1}, {"N", 0x8D, 4, 1, 1}, {"O", 0x8E, 5, 1, 1}, {"P", 0x8F, 6, 1, 1}, {"Q", 0x90, 7, 1, 1}, {"R", 0x91, 8, 1, 1}, {"S", 0x92, 0, 2, 1}, {"T", 0x93, 1, 2, 1}, {"U", 0x94, 2, 2, 1}, {"V", 0x95, 3, 2, 1}, {"W", 0x96, 4, 2, 1}, {"X", 0x97, 5, 2, 1}, {"Y", 0x98, 6, 2, 1}, {"Z", 0x99, 7, 2, 1}, {"(", 0x9A, 1, 3, 0}, {")", 0x9B, 2, 3, 0}, {":", 0x9C, 3, 3, 0}, {";", 0x9D, 4, 3, 0}, {"[", 0x9E, 5, 3, 0}, {"]", 0x9F, 6, 3, 0}, {"a", 0xA0, 0, 0, -1}, {"b", 0xA1, 1, 0, -1}, {"c", 0xA2, 2, 0, -1}, {"d", 0xA3, 3, 0, -1}, {"e", 0xA4, 4, 0, -1}, {"f", 0xA5, 5, 0, -1}, {"g", 0xA6, 6, 0, -1}, {"h", 0xA7, 7, 0, -1}, {"i", 0xA8, 8, 0, -1}, {"j", 0xA9, 0, 1, -1}, {"k", 0xAA, 1, 1, -1}, {"l", 0xAB, 2, 1, -1}, {"m", 0xAC, 3, 1, -1}, {"n", 0xAD, 4, 1, -1}, {"o", 0xAE, 5, 1, -1}, {"p", 0xAF, 6, 1, -1}, {"q", 0xB0, 7, 1, -1}, {"r", 0xB1, 8, 1, -1}, {"s", 0xB2, 0, 2, -1}, {"t", 0xB3, 1, 2, -1}, {"u", 0xB4, 2, 2, -1}, {"v", 0xB5, 3, 2, -1}, {"w", 0xB6, 4, 2, -1}, {"x", 0xB7, 5, 2, -1}, {"y", 0xB8, 6, 2, -1}, {"z", 0xB9, 7, 2, -1}, {"3", 0xE1, 7, 3, 0}, {"2", 0xE2, 8, 3, 0}, {"-", 0xE3, 0, 4, 0}, {"?", 0xE6, 1, 4, 0}, {"!", 0xE7, 2, 4, 0}, {".", 0xF2, 6, 4, 0}, {"1", 0xEF, 3, 4, 0}, {"*", 0xF1, 0, 3, 0}, {"/", 0xF3, 5, 4, 0}, {",", 0xF4, 7, 4, 0}, {"0", 0xF5, 4, 4, 0}
};

int charSetSize = sizeof(charSet) / sizeof(charSet[0]);
int **combinedDict;
int iCode_index = 0;

typedef struct {
    int hex;
    int distance;
} encodedSet;

typedef struct {
    char value[11];
    int keypress;
    int screen;
    int lowbit_decimal;
    char newvalue[11];
    int newkeypress;
    int newscreen;
    int newlowbit_decimal;
} nameSet;

encodedSet** createEncodedSetDict() {
	// Declare 2D array of structs and Allocate memory for the rows
	encodedSet **encodedSetDict = (encodedSet **)malloc(charSetSize * sizeof(encodedSet *));
	if (encodedSetDict == NULL) {
		fprintf(stderr, "Memory allocation failed.\n");
		exit(1);
	}
	// Allocate memory for the columns of each row and assign values
	for (int i = 0; i < charSetSize; i++) {
	       	encodedSetDict[i] = (encodedSet *)malloc(charSetSize * sizeof(encodedSet));
		if (encodedSetDict[i] == NULL) {
			fprintf(stderr, "Memory allocation failed.\n");
			exit(1);
		}
		for (int j = 0; j < charSetSize; j++) {
			// Compute distance
			int distances[4] = {
				abs(charSet[i].column - charSet[j].column),
				abs(charSet[i].column - 9 - charSet[j].column),
				abs(charSet[i].column - (charSet[j].column - 9)),
				abs(charSet[i].column - 9 - (charSet[j].column - 9))
			};
		       	int min_distance = distances[0];
		       	for (int k = 1; k < 4; k++) {
			       	if (distances[k] < min_distance) {
				       	min_distance = distances[k];
			       	}
		       	}
		       	encodedSetDict[i][j].distance = min_distance + abs(charSet[i].row - charSet[j].row);
		       	encodedSetDict[i][j].hex = (2 * charSet[i].value + charSet[j].value) % 256;
	       	}
       	}
	return encodedSetDict;
}

//Calculate penalites of combinations that require too much keyboard navigation, starting with cursor at the "A" position with the keyboard is open, iterate through all nicknames, keeping the best combinations
nameSet** getBestEncoding(encodedSet **encodedSetDict, char ***aAllOptions, char iNameCount) {
	nameSet **encodedNicks = (nameSet **)malloc(iNameCount * sizeof(nameSet *));
	for (int n = 0; aAllOptions[n] != NULL; n++) {
		encodedNicks[n] = (nameSet *)malloc(charSetSize * sizeof(nameSet ));
		encodedNicks[n][0].value[0] = '\0';
		for (int c = 0; aAllOptions[n][c] != NULL; c+=2) {
			int iCost = 99;
			if (c == 0) { //If new nickname, keyboard cursors at "A"
				for (int m = 0; aAllOptions[n][c][m] != 0; m++) {
					char new_screen_status;
					int highbit = aAllOptions[n][c][m] - 1;
					int lowbit = aAllOptions[n][c + 1][m] - 1;
					int screenh = charSet[highbit].screen;
					int screenl = charSet[lowbit].screen;
					int screen_penalty;
					if (screenl == 0 && screenh == 0) {
						screen_penalty = 0;
					} else if (screenl == 0 || screenh == screenl) {
						screen_penalty = abs(screenh - 1) / 2;
						int screen_penalty2 = abs(screenh - 1);
					} else if (screenh == 0) {
						screen_penalty = abs(screenl - 1) / 2;
					} else {
						screen_penalty = 1 + (abs(screenh - 1) / 2);
					}

					int iCost_tmp = (
						encodedSetDict[1][highbit].distance
						+ encodedSetDict[highbit][lowbit].distance
						+ screen_penalty
						+ 2
						);
					if (screenl != 0) {
						new_screen_status = screenl;
					} else if (screenh != 0) {
						new_screen_status = screenh;
					} else {
						new_screen_status = 1;
					}
					encodedNicks[n][m].value[c] = charSet[highbit].key[0];
					encodedNicks[n][m].value[c + 1] = charSet[lowbit].key[0];
					encodedNicks[n][m].keypress = iCost_tmp;
					encodedNicks[n][m].lowbit_decimal = lowbit;
					encodedNicks[n][m].screen= new_screen_status;
				}
			} else {
				for (int p = 0; encodedNicks[n][p].value[0] != '\0'; p++) {
					int prevbit =  encodedNicks[n][p].lowbit_decimal;
					int prev_penalty = encodedNicks[n][p].keypress;
					int screenp =  encodedNicks[n][p].screen;
					for (int m = 0; aAllOptions[n][c][m] != 0; m++) {
						int highbit = aAllOptions[n][c][m] - 1;
						int lowbit = aAllOptions[n][c + 1][m] - 1;
						int screenh = charSet[highbit].screen;
						int screenl = charSet[lowbit].screen;
						char new_screen_status;
						int screen_penalty;
						char found = 0;
						char replaced = 0;
						if (screenl == 0 && screenh == 0) {
							screen_penalty = 0;
						} else if (screenl == 0 || screenh == screenl) {
							screen_penalty = abs(screenh - screenp) / 2;
						} else if (screenh == 0) {
							screen_penalty = abs(screenl - screenp) / 2;
						} else {
							screen_penalty = 1 + (abs(screenh - screenp) / 2);
						}
						int iCost_tmp = (
							encodedSetDict[prevbit][highbit].distance
							+ encodedSetDict[highbit][lowbit].distance
							+ screen_penalty
							+ prev_penalty
							+ 2
							);
						if (screenl != 0) {
							new_screen_status = screenl;
						} else if (screenh != 0) {
							new_screen_status = screenh;
						} else {
							new_screen_status = screenp;
						}

						if (iCost_tmp < encodedNicks[n][p].keypress || encodedNicks[n][p].value[c] == '\0') {
							encodedNicks[n][p].value[c] = charSet[highbit].key[0];
							encodedNicks[n][p].value[c + 1] = charSet[lowbit].key[0];
							encodedNicks[n][p].keypress = iCost_tmp;
							encodedNicks[n][p].lowbit_decimal = lowbit;
							encodedNicks[n][p].screen=new_screen_status;
						}
					}
				}
			}
			
		}
	}
	encodedNicks[iNameCount] = NULL;
	return encodedNicks;
}

//Process input into its corresponding hex, then lookup all possible encoding options for each nibble and store in a 3-dimensional array [NameIndex][Char][Combinations]
char *** processInput(encodedSet** encodedSetDict, const char *sInput, int sInput_len, char iNameCount) {
	int sInput_index;
	char sHex[3];
	unsigned char iHex;
	//Create a 3 Dimensional array - A 2d matrix of Nicknamesx10 characters, each cell containing an array of candidates that provide valid encoding for that hex
	char ***aAllOptions = (char ***)malloc((iNameCount + 1) * sizeof(char **));
    	for (int n = 0; n < iNameCount; n++) {
        	aAllOptions[n] = (char **)malloc(11 * sizeof(char *));
		//Iterate through the hex input every 2 characters
		for (int c = 0; c < 10 && (n * 10) + c < sInput_len; c+=2) {
        		aAllOptions[n][c] = (char *)malloc(50 * sizeof(char ));
        		aAllOptions[n][c + 1] = (char *)malloc(50 * sizeof(char ));
			int match = 0;
			//Extract Both Nibbles and convert from string literal to the Hex values they represent
			int sInput_index = n * 10 + c;
			sHex[0] = sInput[sInput_index];
			sHex[1] = sInput[sInput_index+1];
			sHex[2] = '\0';
			iHex = strtol(sHex, NULL, 16);
			//Look in encodedSetDict for all combinations that gives us that hex value
			for (int k = 0; k < charSetSize; k++) {
				for (int l = 0; l < charSetSize; l++) {
					//If a match is found, store the i,j indexes for where it was found (add 1 because otherwise 0 is treated as null, we will subtractit out later
					if(encodedSetDict[k][l].hex == iHex) {
						aAllOptions[n][c][match] = k + 1;
            					aAllOptions[n][c + 1][match] = l + 1;
						match++;
					}
				}
			} //None of this should be necessary, i think i was paranoid when first encountering bugs
			//Add padding and print #matches
			aAllOptions[n][c][match] = 0;
			aAllOptions[n][c + 1][match] = 0;
			//printf("Match: %d\n", match);
		}
		//Add padding
		aAllOptions[n][10] = NULL;//null padding in 11th slot
	}
	//Add padding
	aAllOptions[iNameCount] = NULL;
	unsigned char extra_padding = sInput_len - ((iNameCount - 1) * 10);
	if (extra_padding != 10) {
		aAllOptions[iNameCount - 1][extra_padding] = NULL;
	}

	return aAllOptions;
}

int hex_to_nick(char *sHex) {
	int sHex_len = strlen(sHex);
	char iNameCount = (sHex_len + 9) / 10;
	encodedSet **encodedSetDict = createEncodedSetDict();
	char ***aAllOptions = processInput(encodedSetDict, sHex, sHex_len, iNameCount);
	nameSet **encodedNicks = getBestEncoding(encodedSetDict, aAllOptions, iNameCount);
	for (int n = 0; encodedNicks[n] != NULL; n++) {
		int r = 0;
		int keys = 99;
		int tempkeys;
		for (int p = 0; encodedNicks[n][p].value[0] != '\0'; p++) {
			tempkeys = encodedNicks[n][p].keypress;
			if (tempkeys < keys) {
				keys = tempkeys;
				r = p;
			}
		}
		printf("Nickname: %d (%d keypresses)\n", n + 1, encodedNicks[n][r].keypress);
		printf("%s\n\n", encodedNicks[n][r].value, encodedNicks[n][r].keypress);
	}
	return 0;
}
