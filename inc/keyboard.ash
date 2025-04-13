_BSS	SEGMENT	PUBLIC DWORD USE32 'BSS'

extrn key_hit      : byte
extrn extended_key : byte
extrn keyboard     : byte
extrn key_edge     : byte
extrn key_string   : byte

_BSS	ENDS

_TEXT   SEGMENT PUBLIC USE32 DWORD 'CODE'

extrn	keyint_on	        : near
extrn	keyint_off	      : near
extrn	clear_key_buffer	: near

_TEXT   ENDS
