	.text
	.even
	.globl adler32
adler32:
	link.w %a6,#0
	movm.l #0x1c20,-(%sp)
	move.l 10(%a6),%d0
	move.l 14(%a6),%a2
	move.w 18(%a6),%d5

	moveq.l #0,%d4
	moveq.l #0,%d3

	move.w %d0,%d3
	swap %d0
	move.w %d0,%d4

	cmp.w #0,%a2
	jbne .L32
	moveq.l #1,%d0
	bra .L31

.L23:
	move.w %d5,%d0
	cmp.w #5552,%d5
	jbls .L24
	move.w #5552,%d0
.L24:
	sub.w %d0,%d5
	tst.w %d0
	jbeq .L25
	moveq.l #0,%d1
	subq.w #1,%d0

.L29:
	move.b (%a2)+,%d1
	add.l %d1,%d3
	add.l %d3,%d4
	dbra.w %d0,.L29
.L25:
	divu  #65521,%d3
	clr.w %d3
	swap  %d3

	divu  #65521,%d4
	clr.w %d4
	swap  %d4

.L32:
	tst.w %d5
	jbne .L23

	move.w %d4,%d0
	swap %d0
	move.w %d3,%d0
.L31:
	movm.l -16(%a6),#0x438
	unlk %a6
	rts
