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
