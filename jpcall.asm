.org $0000
	.test1
	call test1
	jp test2
.org $4000
	.test2
	jp test1
	call test2
.org $8000
	call test1
	call test2
