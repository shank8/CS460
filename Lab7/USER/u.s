.globl begtext, begdata, begbss                      ! needed by linker
.globl _getc,_putc,_getcs, _getds
.globl _main,_prints, _int80
.globl _diskr,_get_byte,_put_byte

.text                                                ! these tell as:	
begtext:                                             ! text,data,bss segments
.data                                                ! are all the same.
begdata:
.bss
begbss:
.text               

        call _main

        mov   ax,#mes
        push  ax
        call  _prints
        pop   ax

dead:   jmp   dead

mes:    .asciz "BACK TO ASSEMBLY AGAIN\n\r"

_diskr:
_get_byte:
_put_byte:


	
_getcs:
        mov   ax, cs
        ret
_getds: mov   ax, ds
        ret
        !---------------------------------------------
        !  char getc[]   function: returns a char
        !---------------------------------------------
_getc:
        xorb   ah,ah           ! clear ah
        int    0x16            ! call BIOS to get a char in AX
        ret 

        !----------------------------------------------
        ! void putc[char c]  function: print a char
        !----------------------------------------------
_putc:           
        push   bp
        mov    bp,sp

        movb   al,4[bp]        ! get the char into aL
        movb   ah,#14          ! aH = 14
        movb   bl,#12
        int    0x10            ! call BIOS to display the char

        pop    bp
        ret

_int80:
        int    80
        ret

