	.text
	move.l	%a0,%d0		| do I need to preserve A0?
	move.l	6(%a7),%a0
	pea	table(%pc)	| if not I could exg a lot, but this works.
	move.l	(%a7)+,(%a0)+
	clr.l	(%a0)
	move.l	%d0,%a0
	moveq	#0,%d0
	rts

| dispatch table - note the lack of jump to jump to jumps.  Note this
| can be moved to keep this and the entry points within 0..32K

table:
	dc.w	name-table
| 1
	dc.w	zlib_open-table	
	dc.w	zlib_close-table
	dc.w	zlib_null-table
	dc.w	zlib_null-table
| 5
	dc.w	deflateInit2_-table
	dc.w	deflate-table
	dc.w	deflateEnd-table
| 8
	dc.w	inflateInit2_-table
	dc.w	inflate-table
	dc.w	inflateEnd-table
| 11
	dc.w	crc32-table
	dc.w	adler32-table
	dc.w	compress2-table
	dc.w	uncompress-table

name:
	.asciz	"Z.lib"
	.asciz  "Palm ZLib adaptation Copyright 2000, Tom Zerucha, tz@execpc.com"
	.even

