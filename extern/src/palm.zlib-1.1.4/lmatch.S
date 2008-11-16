	.file	"lmatch.c"
gcc2_compiled.:
__gnu_compiled_c:
.text
	.even
.globl longest_match
longest_match:
	link.w %a6,#-12
	movm.l #0x1f38,-(%sp)
	move.l 8(%a6),%a3
	move.w 12(%a6),%d3
	move.w 76(%a3),%d5
	clr.l %d0
	move.w 68(%a3),%d0
	move.l 32(%a3),%a2
	add.l %d0,%a2 /*JANE2*/
	move.w 74(%a3),%d4
	move.w 86(%a3),%a4
	move.w 26(%a3),%d2
	move.w %d2,%d0
	add.w #-262,%d0
	move.w 68(%a3),%d1
	cmp.w %d1,%d0
	jbcc .L20
	move.w %d1,%d6
	add.w #262,%d6
	sub.w %d2,%d6
	bra .L21

.L20:
	clr.w %d6
.L21:
	move.l 40(%a3),-4(%a6)
	move.w 30(%a3),-6(%a6)
	clr.l %d0
	move.w 68(%a3),%d0
	add.l 32(%a3),%d0 /*JANE2*/
	move.l %d0,%d1
	add.l #258,%d1 /*JANE2*/
	move.w %d4,%a0
	move.b -1(%a2,%a0.l),%d7
	move.b (%a0,%a2.l),%d2
	move.w 84(%a3),%d0
	cmp.w 74(%a3),%d0
	jbhi .L22
	lsr.w #2,%d5
.L22:
	move.w 72(%a3),%d0
	cmp.w %a4,%d0
	jbcc .L23
	move.w %d0,%a4
.L23:
	clr.l -10(%a6)



.L37:
	move.w %d3,-8(%a6)

	move.l -10(%a6),%a0
	add.l 32(%a3),%a0

	cmp.b (%a0,%d4.w),%d2
	jbne .L26
	cmp.b -1(%a0,%d4.w),%d7
	jbne .L26

	move.b (%a0)+,%d0
	cmp.b (%a2),%d0
	jbne .L26

	move.b (%a0),%d0
	cmp.b 1(%a2),%d0
	jbne .L26

	addq.l #3,%a2
	addq.l #2,%a0

	move.l %d1,%a1
	lea (-258,%a1),%a1

.L31:
	cmpm.b (%a0)+,(%a2)+
	jbne .L30
	cmpm.b (%a0)+,(%a2)+
	jbne .L30
	cmpm.b (%a0)+,(%a2)+
	jbne .L30
	cmpm.b (%a0)+,(%a2)+
	jbne .L30
	cmpm.b (%a0)+,(%a2)+
	jbne .L30
	cmpm.b (%a0)+,(%a2)+
	jbne .L30
	cmpm.b (%a0)+,(%a2)+
	jbne .L30
	cmpm.b (%a0)+,(%a2)+
	jbne .L30

	cmp.l %a2,%d1
	jbcc .L31


.L30:
	add.w	#257,%a2
	sub.l	%d1,%a2
	move.w	%a2,%d0

	move.l %a1,%a2

	cmp.w %d0,%d4
	jbge .L26

	move.w %d3,70(%a3)
	move.w %d0,%d4
	cmp.w %a4,%d4
	jbge .L25

	move.b -1(%a1,%d4.w),%d7
	move.b (%a1,%d4.w),%d2
.L26:
	and.w -6(%a6),%d3
	clr.l %d0
	move.w %d3,%d0
	add.l %d0,%d0
	move.l -4(%a6),%a0
	move.w (%a0,%d0.l),%d3
	cmp.w %d3,%d6
	jbcc .L25
	subq.w #1,%d5
	jbne .L37



.L25:
	move.w 72(%a3),%d1
	move.w %d4,%d0
	cmp.w %d0,%d1
	jbcc .L39
	move.w %d1,%d0
.L39:
	movm.l (%sp)+,#0x1cf8
	unlk %a6
	rts
