PKMNRBY-code-converter
======================
Original framework this is derived from: [8F Helper](https://github.com/KernelEquinox/8F-Helper/releases/) (KernelEquinox)
Logic for Nickwriter function [Nickname Writer Code Generator](https://scotteh.me/ace/nick/) (Scott Maxwell & TimoVM)

All-in-one conversion tool application written in C which is able to convert ASM & HEX machine code for the Sharp SM83 (gbz80) found in the Nintendo Gameboy, as well as translate these codes into various setups used for arbitrary code execution (ACE) in Gen 1 - Pokemon R/B/Y.

### Supports the following formats:
  - asm - GB-Z80 assembly language
  - hex - Hexadecimal machine code format
  - joy - Joypad values
  - item - R/B/Y item codes for use with ACE
  - nickname - for use with TimoVM's Nickwriter

### Smart Item Conversion
Many modern ACE setups start off with an expanded inventory where `j. x00` items are plentiful. By utilizing these free items, it will reduce the number of unique items that need to be obtained. This is accomplished by adding `nop` instructions to the input code, in places where it will have no detrimental effect, in order to offset the current address to a "quantity" slot. This is avoided where a conversion will be a "friendly" item like Ultra Balls/Master Balls which are easy to obtain

### Usage
```
Usage: pkmnrby-cc [options] [hex]

Options:
  -o format    Display output in specificied format (default: hex)
  -org offset  Specify memory offset for start of code
  -h           Print this help message and exit
  -v           Print version information and exit

Formats:
  asm          GB-Z80 assembly language
  hex          Hexadecimal machine code format
  joy          Joypad values
  item        R/B/Y item codes for use with ACE
  nickname         for used with TimoVM's Nickwriter

Modes:
This is specific to the item output format.
Mode 0 will do normal code -> item conversion
Mode 1 will replace as many instances of the code as possible
with `j. xQQ` since in expanded inventory you will have plenty
of those available. It does so by adding `nop` instructions to your code.
This will reduce the number of unique items needed, and save time
hunting them down.

Examples:
  pkmnrby-cc EA14D7C9
  pkmnrby-cc -o asm bgb_mem.dump
  pkmnrby-cc zzazz.asm
  pkmnrby-cc -o item 0E1626642EBB4140CDD635C9
  pkmnrby-cc -o nickname coin_case.asm
```

### Examples
test_code.asm:
```
.org $D322

di               ; Kill all interrupts

.testJump        ; Just to show the label/jump processing
dec a
inc a
inc b
jr c, testJump   ; Relative jump to label

ld h, $FF        ; Set hl to $FF00
inc b
xor l

ld a, $20        ; Enter STOP mode until D-pad is pressed
inc c
ld (hl), a
stop

ret              ; Continue normal execution 
```
Program outputs:
```
root@gbdev:~# pkmnrby-cc test_code.asm

**************************************************************
 ____  _  __ _      _      ____  ____ ___  _
/  __\/ |/ // \__/|/ \  /|/  __\/  __\\  \//
|  \/||   / | |\/||| |\ |||  \/|| | // \  /
|  __/|   \ | |  ||| | \|||    /| |_\\ / /
\_/   \_|\_\\_/  \|\_/  \|\_/\_\\____//_/
 ____  ____  ____  _____   
/   _\/  _ \/  _ \/  __/
|  /  | / \|| | \||  \ _____
|  \_ | \_/|| |_/||  /_\____\
\____/\____/\____/\____\
____  ____  _      _     _____ ____  _____  _____ ____
/   _\/  _ \/ \  /|/ \ |\/  __//  __\/__ __\/  __//  __\ 
|  /  | / \|| |\ ||| | //|  \  |  \/|  / \  |  \  |  \/|
|  \_ | \_/|| | \||| \// |  /_ |    /  | |  |  /_ |    /
\____/\____/\_/  \|\__/  \____\\_/\_\  \_/  \____\\_/\_\ 

**************************************************************
Input format detected as asm!

*********************************************************
Assembly Code:
.org D322

di               ; Kill all interrupts

.testJump        ; Just to show the label/jump processing
dec a
inc a
inc b
jr c, testJump   ; Relative jump to label

ld h, $FF        ; Set hl to $FF00
inc b
xor l

ld a, $20        ; Enter STOP mode until D-pad is pressed
inc c
ld (hl), a
stop

ret              ; Continue normal execution 

*********************************************************
Full Joypad Writer
==================

Joypad Values:
A
DOWN UP LEFT RIGHT B A A
LEFT RIGHT START SELECT A A
LEFT RIGHT START SELECT SELECT
DOWN SELECT DOWN DOWN
LEFT RIGHT START START
DOWN UP LEFT RIGHT START B A A
LEFT SELECT B B
DOWN UP LEFT RIGHT START SELECT B A A
SELECT SELECT
DOWN LEFT START SELECT A A
LEFT RIGHT START SELECT B B
LEFT LEFT
START SELECT SELECT
UP LEFT RIGHT SELECT B A A
RIGHT RIGHT
A A
DOWN UP START A A
START + SELECT

Total number of button presses: 85


*********************************************************
Nicknames for TimOS' Nickname Writer
====================================

Nickname 1 (36 keypresses, checksum: 0x28):	"jbl!l?saj?"
Nickname 2 (24 keypresses, checksum: 0x51):	"jjywvVmmMV"
Nickname 3 (33 keypresses, checksum: 0x71):	"g.yqz([Mn]ttk"
Nickname 4 (11 keypresses, checksum: 0x4A):	"irRh"

Total Keypresses: 108


*********************************************************
Smart Item List:

Item            Quantity
========================
TM43            x61
[item 0x00]     x60
Poke Ball       x56
TM50            x38
CANCEL (TM55)   x4
[item 0xAD]     x62
Fire Stone      x12
[item 0x00]     x119
Full Restore    x1
[item 0x00]     x201


-- WARNING! --
 * Duplicate item stacks detected!
 * Glitch items detected!

*********************************************************
Standard Item List:

Item            Quantity
========================
TM43            x61
Fresh Water     x4
Super Repel     x251
Carbos          x255
Poke Ball       x173
Lemonade        x32
Burn Heal       x119
Full Restore    x1
TM01            xAny

*********************************************************
Machine code: F33D3C0438FB26FF04AD3E200C771001C9
```

### Notes
I've opted to use `10 01` as the `STOP` opcode instead of the correct `10 00`. This is because it's much easier to get 1 of an item rather than 0 of an item. In all tests, the `STOP` instruction executes normally even with a non-zero argument.

