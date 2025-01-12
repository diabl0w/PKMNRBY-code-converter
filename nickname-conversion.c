#include "defines.h"
#define min(a, b) ((a) < (b) ? (a) : (b))

// Define the structure for characters set entry
typedef struct {
    const char *key;
    int value;
    int column;
    int row;
    int screen;
} CharSetEntry;

// Define the characters set
CharSetEntry charSet[] = {
{" ", 0x7F, 8, 2, 0}, 
{"A", 0x80, 0, 0, 1}, 
{"B", 0x81, 1, 0, 1}, 
{"C", 0x82, 2, 0, 1}, 
{"D", 0x83, 3, 0, 1}, 
{"E", 0x84, 4, 0, 1}, 
{"F", 0x85, 5, 0, 1}, 
{"G", 0x86, 6, 0, 1}, 
{"H", 0x87, 7, 0, 1}, 
{"I", 0x88, 8, 0, 1}, 
{"J", 0x89, 0, 1, 1}, 
{"K", 0x8A, 1, 1, 1}, 
{"L", 0x8B, 2, 1, 1}, 
{"M", 0x8C, 3, 1, 1}, 
{"N", 0x8D, 4, 1, 1}, 
{"O", 0x8E, 5, 1, 1}, 
{"P", 0x8F, 6, 1, 1}, 
{"Q", 0x90, 7, 1, 1}, 
{"R", 0x91, 8, 1, 1}, 
{"S", 0x92, 0, 2, 1}, 
{"T", 0x93, 1, 2, 1}, 
{"U", 0x94, 2, 2, 1}, 
{"V", 0x95, 3, 2, 1}, 
{"W", 0x96, 4, 2, 1}, 
{"X", 0x97, 5, 2, 1}, 
{"Y", 0x98, 6, 2, 1}, 
{"Z", 0x99, 7, 2, 1}, 
{"(", 0x9A, 1, 3, 0}, 
{")", 0x9B, 2, 3, 0}, 
{":", 0x9C, 3, 3, 0}, 
{";", 0x9D, 4, 3, 0}, 
{"[", 0x9E, 5, 3, 0}, 
{"]", 0x9F, 6, 3, 0}, 
{"a", 0xA0, 0, 0, -1}, 
{"b", 0xA1, 1, 0, -1}, 
{"c", 0xA2, 2, 0, -1}, 
{"d", 0xA3, 3, 0, -1}, 
{"e", 0xA4, 4, 0, -1}, 
{"f", 0xA5, 5, 0, -1}, 
{"g", 0xA6, 6, 0, -1}, 
{"h", 0xA7, 7, 0, -1}, 
{"i", 0xA8, 8, 0, -1}, 
{"j", 0xA9, 0, 1, -1}, 
{"k", 0xAA, 1, 1, -1}, 
{"l", 0xAB, 2, 1, -1}, 
{"m", 0xAC, 3, 1, -1}, 
{"n", 0xAD, 4, 1, -1}, 
{"o", 0xAE, 5, 1, -1}, 
{"p", 0xAF, 6, 1, -1}, 
{"q", 0xB0, 7, 1, -1}, 
{"r", 0xB1, 8, 1, -1}, 
{"s", 0xB2, 0, 2, -1}, 
{"t", 0xB3, 1, 2, -1}, 
{"u", 0xB4, 2, 2, -1}, 
{"v", 0xB5, 3, 2, -1}, 
{"w", 0xB6, 4, 2, -1}, 
{"x", 0xB7, 5, 2, -1}, 
{"y", 0xB8, 6, 2, -1}, 
{"z", 0xB9, 7, 2, -1}, 
{"[Pk]", 0xE1, 7, 3, 0}, 
{"[Mn]", 0xE2, 8, 3, 0}, 
{"-", 0xE3, 0, 4, 0}, 
{"?", 0xE6, 1, 4, 0}, 
{"!", 0xE7, 2, 4, 0}, 
{".", 0xF2, 6, 4, 0}, 
{"♂", 0xEF, 3, 4, 0}, 
{"*", 0xF1, 0, 3, 0}, 
{"/", 0xF3, 5, 4, 0}, 
{",", 0xF4, 7, 4, 0}, 
{"♀", 0xF5, 4, 4, 0}
};

int charSetSize = sizeof(charSet) / sizeof(charSet[0]);
int iCode_index = 0;

typedef struct {
    int hex;
    int dist;
} encodedSet;
encodedSet **encodedSetDict;

typedef struct {
	char nibU;
	char nibL;
} screenState;

typedef struct {
	unsigned char nibU;
	unsigned char nibL;
	int dist;
	int hex;
	screenState screen;
} matchSet
;
typedef struct {
	int count;
	matchSet* match;
} matchSetBundle;

typedef struct {
	char name[81]; // max possible = 10 * "[female]" + '\0' term
	unsigned char nibU;
	unsigned char nibL;
	int keypress;
	int checksum;
	char screen;
} nameSet;

typedef struct {
	int count;
	nameSet* namedata;
} nameSetBundle;

encodedSet** createEncodedSetDict() {
	// Declare 2D array of structs and Allocate memory for the rows
	encodedSetDict = (encodedSet **)malloc(charSetSize * sizeof(encodedSet *));
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
			// Compute dist
			int dists[4] = {
				abs(charSet[i].column - charSet[j].column),
				abs(charSet[i].column - 9 - charSet[j].column),
				abs(charSet[i].column - (charSet[j].column - 9)),
				abs(charSet[i].column - 9 - (charSet[j].column - 9))
			};
		       	int min_dist = dists[0];
		       	for (int k = 1; k < 4; k++) {
			       	if (dists[k] < min_dist) {
				       	min_dist = dists[k];
			       	}
		       	}
		       	encodedSetDict[i][j].dist = min_dist + abs(charSet[i].row - charSet[j].row);
		       	encodedSetDict[i][j].hex = (2 * charSet[i].value + charSet[j].value) % 256;
	       	}
       	}
	return encodedSetDict;
}

nameSetBundle initializeNameBuilder(matchSetBundle matches) {
	int count = 0;
	nameSetBundle encodedNick;
	encodedNick.namedata = (nameSet *)malloc(matches.count * sizeof(nameSet));
	for (int m = 0; m < matches.count; m++) { //for each match
		int screen_penalty;
		int screen_state;
		int keypress = 0;
		int checksum = 0;
		//Calculate screen switch keypress required
		if (matches.match[m].screen.nibU == 0 && matches.match[m].screen.nibL == 0) {
			screen_penalty = 0;
		} else if (matches.match[m].screen.nibL == 0 || matches.match[m].screen.nibU == matches.match[m].screen.nibL) {
			screen_penalty = abs(matches.match[m].screen.nibU - 1) / 2;
		} else if (matches.match[m].screen.nibU == 0) {
			screen_penalty = abs(matches.match[m].screen.nibL - 1) / 2;
		} else {
			screen_penalty = 1 + (abs(matches.match[m].screen.nibU - 1) / 2);
		}
		//Calculate total input keypresses required
		keypress = (
			encodedSetDict[1][matches.match[m].nibU - 1].dist
			+ matches.match[m].dist
			+ screen_penalty
			+ 2
			);
		//Keep track of screen state
		if (matches.match[m].screen.nibL == 0  && matches.match[m].screen.nibU != 0) {
			screen_state = matches.match[m].screen.nibU;
		} else if (matches.match[m].screen.nibL == 0  && matches.match[m].screen.nibU == 0) {
			screen_state = 1;
		} else {
			screen_state = matches.match[m].screen.nibL;
		}
		//Calculate checksum
		checksum = (0x80 + matches.match[m].hex) % 256;
		//Write data
		snprintf(encodedNick.namedata[m].name, sizeof(encodedNick.namedata[m]), "%s%s", charSet[matches.match[m].nibU - 1].key, charSet[matches.match[m].nibL - 1].key);
		encodedNick.namedata[m].nibU = matches.match[m].nibU;
		encodedNick.namedata[m].nibL = matches.match[m].nibL;
		encodedNick.namedata[m].screen = screen_state;
		encodedNick.namedata[m].keypress = keypress;
		encodedNick.namedata[m].checksum = checksum;
		count ++;
		encodedNick.count = count;
	}
	return encodedNick;
}

nameSetBundle nameBuilder(nameSetBundle nick_draft, matchSetBundle matches) {
	int c = 0;
	nameSetBundle encodedNick;
	encodedNick.namedata = (nameSet *)malloc(matches.count * nick_draft.count * sizeof(nameSet)); //make the array max size, but shorten as we find duplicates
	for (int o = 0; o < nick_draft.count; o++) {
		for (int m = 0; m < matches.count; m++) { //for each match
			int found = 0;
			int screen_penalty;
			int screen_state;
			int keypress = 0;
			int checksum = 0;
			//Calculate screen switch keypress required
			if (matches.match[m].screen.nibU == 0 && matches.match[m].screen.nibL == 0) {
				screen_penalty = 0;
			} else if (matches.match[m].screen.nibL == 0 || matches.match[m].screen.nibU == matches.match[m].screen.nibL) {
				screen_penalty = abs(matches.match[m].screen.nibU - nick_draft.namedata[o].screen) / 2;
			} else if (matches.match[m].screen.nibU == 0) {
				screen_penalty = abs(matches.match[m].screen.nibL - nick_draft.namedata[o].screen) / 2;
			} else {
				screen_penalty = 1 + (abs(matches.match[m].screen.nibU - nick_draft.namedata[o].screen) / 2);
			}

			//Calculate total input keypresses required
			keypress = (
				encodedSetDict[nick_draft.namedata[o].nibL - 1][matches.match[m].nibU - 1].dist
				+ matches.match[m].dist
				+ nick_draft.namedata[o].keypress
				+ screen_penalty
				+ 2
				);
			//Keep track of screen state
			if (matches.match[m].screen.nibL == 0  && matches.match[m].screen.nibU != 0) {
				screen_state = matches.match[m].screen.nibU;
			} else if (matches.match[m].screen.nibL == 0  && matches.match[m].screen.nibU == 0) {
				screen_state = nick_draft.namedata[o].screen;
			} else {
				screen_state = matches.match[m].screen.nibL;
			}
			//Calculate checksum
			checksum = (nick_draft.namedata[o].checksum + matches.match[m].hex) % 256;
			//If we already have the the last character as a candidate
			for (int i = 0; i < c; i++) {
				if (encodedNick.namedata[i].nibL == matches.match[m].nibL) {
					found = 1;
					//If the new candidate requires less keypresses, overwrite the old one
				       	if (keypress <= encodedNick.namedata[i].keypress) {

						snprintf(encodedNick.namedata[i].name, sizeof(encodedNick.namedata[i]), "%s%s%s", nick_draft.namedata[o].name, charSet[matches.match[m].nibU - 1].key, charSet[matches.match[m].nibL - 1].key);
						encodedNick.namedata[i].nibU = matches.match[m].nibU;
						encodedNick.namedata[i].nibL = matches.match[m].nibL;
						encodedNick.namedata[i].screen = screen_state;
						encodedNick.namedata[i].keypress = keypress;
						encodedNick.namedata[i].checksum = checksum;
					}
				} 
			}
			//If this is a unique candidate, write it
			if (found != 1) {
				snprintf(encodedNick.namedata[c].name, sizeof(encodedNick.namedata[c]), "%s%s%s", nick_draft.namedata[o].name, charSet[matches.match[m].nibU - 1].key, charSet[matches.match[m].nibL - 1].key);
				encodedNick.namedata[c].nibU = matches.match[m].nibU;
				encodedNick.namedata[c].nibL = matches.match[m].nibL;
				encodedNick.namedata[c].screen = screen_state;
				encodedNick.namedata[c].keypress = keypress;
				encodedNick.namedata[c].checksum = checksum;
				c++;
			}
		}
	}
	encodedNick.namedata = (nameSet *)realloc(encodedNick.namedata, c * sizeof(nameSet));
	encodedNick.count = c;
	return encodedNick;
}

nameSetBundle deepCopy(nameSetBundle source) {
    nameSetBundle copy;
    copy.count = source.count;
    copy.namedata = malloc(source.count * sizeof(nameSet));
    if (copy.namedata == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    for (int i = 0; i < source.count; i++) {
        strcpy(copy.namedata[i].name, source.namedata[i].name);
        copy.namedata[i].nibU = source.namedata[i].nibU;
        copy.namedata[i].nibL = source.namedata[i].nibL;
        copy.namedata[i].keypress = source.namedata[i].keypress;
        copy.namedata[i].screen = source.namedata[i].screen;
        copy.namedata[i].checksum = source.namedata[i].checksum;
    }
    return copy;
}

matchSetBundle getCandidates(int iHex) {
	char m = 0;
    	for (int i = 0; i < charSetSize; ++i) {
    		for (int j = 0; j < charSetSize; ++j) {
			if (encodedSetDict[i][j].hex == iHex) {
				m++;
			}
        	}
	}
	matchSetBundle matches;
	matches.match = (matchSet *)malloc((m + 1) * sizeof(matchSet));
	m=0;
    	for (int i = 0; i < charSetSize; ++i) {
    		for (int j = 0; j < charSetSize; ++j) {
			if (encodedSetDict[i][j].hex == iHex) {
				matches.match[m].hex = iHex;
				matches.match[m].nibU = i + 1;
				matches.match[m].nibL = j + 1;
				matches.match[m].dist = encodedSetDict[i][j].dist;
				matches.match[m].screen.nibU = charSet[i].screen;
				matches.match[m].screen.nibL = charSet[j].screen;
				m++;	
			}
		}
	}
	matches.count = m;
	return matches;
}

void hex_to_nick(char *sCode) {
	encodedSet** encodedSetDict = createEncodedSetDict();
	int sCode_len = strlen(sCode);
	char iName_count = (sCode_len + 9) / 10;
	char bytes[iName_count];
	int total_keypress = 0;
	printf(BLUE"Nicknames for TimOS' Nickname Writer\n");
	printf("====================================\n\n"BLACK);
	for (int n=0; n < iName_count; n++) {
		if (sCode_len - ((n + 1) * 10) > -1) {
			bytes[n] = 5;
		} else {
			bytes[n] = (sCode_len - (n * 10)) / 2;
		}
	}
	for (int n=0; n < iName_count; n++) {
		nameSetBundle temp_set;
		nameSetBundle name_draft;
		matchSetBundle matches;
		for (char b=0; b < bytes[n]; b++) {
			int sCode_index = (n * 10) + (b * 2);
			char sByte[3];
			sByte[0] = sCode[sCode_index];
			sByte[1] = sCode[sCode_index + 1];
			sByte[2] = '\0';
			int iHex = strtol(sByte, NULL, 16);
			matches = getCandidates(iHex);
			if (b == 0) {
				name_draft = initializeNameBuilder(matches);
			} else {
				temp_set = nameBuilder(name_draft, matches);
				name_draft = deepCopy(temp_set);
			}
		}
		int lowest_keypress = 99;
		int index;
		for (int x = 0; x < name_draft.count; x++) {
			if (name_draft.namedata[x].keypress < lowest_keypress) {
				lowest_keypress = name_draft.namedata[x].keypress;
				index = x;
			}
		}
		total_keypress = total_keypress + name_draft.namedata[index].keypress + 1; // +1 for enter on each nickname
		printf("Nickname %d (%d keypresses, checksum: 0x%02hhX):\t\"%s\"\n", n + 1, name_draft.namedata[index].keypress, name_draft.namedata[index].checksum, name_draft.namedata[index].name); 
	}
	printf("\nTotal Keypresses: %d\n\n", total_keypress);
}
