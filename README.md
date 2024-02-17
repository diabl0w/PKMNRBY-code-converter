PKMNRBY-code-converter
======================
All credit goes to the original author of [8F Helper](https://github.com/KernelEquinox/8F-Helper/releases/) (KernelEquinox)
and new features will be implemented soon based off code from [Nickname Writer Code Generator](https://scotteh.me/ace/nick/) (Scott Maxwell & TimoVM)
I've merely added some tweaks and combined tools, most of the work was already done.

A quick and dirty (but fast!) application in C that uses lookup tables to assist in the conversion between ASM & HEX machine code for the Sharp SM83 (gbz80) found in the Nintendo Gameboy, as well as utilities for translating these codes into various types of setups used for arbitrary code execution (ACE) in Gen 1 - Pokemon R/B/Y.

## Usage
```
Usage: pkmnrby-cc [options] [hex]

Options:
  -i format    Specify Input format of your code (hex or asm) (required)
  -o format    Display output in specificied format (default: hex)
  -f file      File mode (read input from file) (default: sys.arg)
  -ofs offset  Specify memory offset to display in asm format.
                 (Ignored in other formats)
  -m mode      See notes below (valid values 0 or 1)
  -w           Disable item warning messages
  -h           Print this help message and exit
  -v           Print version information and exit

Formats:
  asm          GB-Z80 assembly language
  bgb          BGB-style assembly language
  hex          Hexadecimal machine code format
  joy          Joypad values
  items        R/B/Y item codes for use with ACE
*TODO:* add TimoVM's Nickwriter

Modes:
This is specific to the item output format.
Mode 0 will do normal code -> item conversion
Mode 1 will replace as many instances of the code as possible
with `j. xQQ` since in expanded inventory you will have plenty
of those available. It does so by adding `nop` instructions to your code.
This will reduce the number of unique items needed, and save time
hunting them down.

Examples:
  pkmnrby-cc -o asm EA14D7C9
  pkmnrby-cc -i hex -o asm -f bgb_mem.dump
  pkmnrby-cc -i asm -o hex -f zzazz.asm
  pkmnrby-cc -i hex -o items 0E1626642EBB4140CDD635C9
  pkmnrby-cc -i asm -o items -f coin_case.asm
```

## Examples
test_code.asm:
```asm
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
### Program outputs
```
root@gbdev:~# pkmnrby-cc -i asm -o hex -f test_code.asm

Machine code: F33D3C0438FB26FF04AD3E200C771001C9
```
```
root@gbdev:~# pkmnrby-cc -o asm F33D3C0438FB26FF04AD3E200C771001C9

gbz80 Assembly:

   0  F3               di
   1  3D               dec  a
   2  3C               inc  a
   3  04               inc  b
   4  38 FB            jr   c,$FB
   6  26 FF            ld   h,$FF
   8  04               inc  b
   9  AD               xor  l
   A  3E 20            ld   a,$20
   C  0C               inc  c
   D  77               ld   (hl),a
   E  10 01            stop
  10  C9               ret
```
```
root@gbdev:~# pkmnrby-cc -o gen1 -f test_code.asm

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
```

## Notes
I've opted to use `10 01` as the `STOP` opcode instead of the correct `10 00`. This is because it's much easier to get 1 of an item rather than 0 of an item. In all tests, the `STOP` instruction executes normally even with a non-zero argument.

