                BOOTSEG = 0x1000
                SSP     = 0
.globl begtext, begdata, begbss                      ! needed by linker

!               IMPORTS and EXPORTS
.globl _resetVideo,_getc,_putc,_setes,_inces 
.globl _main,_prints                                 
.globl _tswitch,_running,_scheduler
.globl _int80h,_kcinth
.globl _diskr,_diskw
.globl _getcs, _goUmode, _procSize
.globl _get_byte,_put_byte
.globl _get_word,_put_word

.globl _lock, _unlock, _int_off, _int_on
.globl _in_byte,_out_byte

!.globl _tinth, _thandler
.globl _s0inth, _s0handler
.globl _s1inth, _s1handler

.text                                                ! these tell as:	
begtext:                                             ! text,data,bss segments
.data                                                ! are all the same.
begdata:
.bss
begbss:
.text                                                

start:
        mov     ax,cs                   ! establish segments 
        mov     ds,ax                   ! we know ES,CS=0x1000. Let DS=CS  
        mov     ss,ax                   ! SS = CS ===> all point to 0x1000
        mov     es,ax
        mov     sp,#SSP                 ! SP = 32KB

        call _main                      ! call main[] in C

! if ever return, just hang     
        mov   ax, #msg
        push  ax
        call  _prints
!_exit:
dead:   jmp   dead
msg:    .asciz "BACK TO ASSEMBLY AND HANG\n\r"    

_kreboot:
        jmpi 0,0xFFFF	
!*************************************************************
!     KCW  added functions for MT system
!************************************************************
_tswitch:
          cli
          push   ax
          push   bx
          push   cx
          push   dx
          push   bp
          push   si
          push   di
	  pushf
	  mov	 bx, _running
 	  mov	 2[bx], sp

find:     call	 _scheduler

resume:	  mov	 bx, _running
	  mov	 sp, 2[bx]
	  popf
	  pop    di
          pop    si
          pop    bp
          pop    dx
          pop    cx
          pop    bx
          pop    ax
          sti
          ret

_getcs: mov   ax, cs
        ret

USS =   4
USP =   6
INK =   8

! as86 macro: parameters are ?1 ?2, etc
         MACRO INTH
          push ax
          push bx
          push cx
          push dx
          push bp
          push si
          push di
          push es
          push ds

          push cs              ! we know CS=0x1000 
          pop  ds              ! let     DS = CS  to access Kernel data 
          
 	  mov bx,_running   	! ready to access proc
          inc INK[bx]
          cmp INK[bx],#1
          jg   ?1

          ! was in Umode: save interrupted (SS,SP) into proc
	  mov si,_running   	! ready to access proc
          mov USS[si],ss        ! save SS  in proc.USS
          mov USP[si],sp        ! save SP  in proc.USP

          ! Set kernel sp to proc[i].ksp 
          mov  di,ds            ! stupid !!        
          mov  es,di            ! CS=DS=SS=ES in Kmode
          mov  ss,di
          mov  sp,_running      ! sp -> running's kstack[] high end
          add  sp,_procSize

?1:
          call _?1              ! call handler in C

          br   _ireturn         ! upon return from C, to _iretutn
         MEND

_int80h: INTH kcinth

!_tinth:  INTH thandler
!_kbinth: INTH kbhandler

_s0inth: INTH s0handler
_s1inth: INTH s1handler

!*===========================================================================*
!*		_ireturn  and  goUmode()       				     *
!*===========================================================================*
! ustack contains    flag,ucs,upc, ax,bx,cx,dx,bp,si,di,es,ds
! uSS and uSP are in proc
_ireturn:                       ! return from INTERRUPTs
_goUmode:                       ! goYmode(): same as return from INTERRUPT
        cli
	mov bx,_running 	! bx -> proc
        dec INK[bx]
        cmp INK[bx],#0
        jg  xkmode

! return to Umode, restore ustack (uSS, uSP)m then pop ustack
        mov ax,USS[bx]
        mov ss,ax               ! restore SS
        mov sp,USP[bx]          ! restore SP
xkmode:  
	pop ds
	pop es
	pop di
        pop si
        pop bp
        pop dx
        pop cx
        pop bx
        pop ax 
        iret

        !--------------------------------
        ! resetVideo[] : clear screen, home cursor
        !--------------------------------
_resetVideo:	
        mov     ax, #0x0012
        int     0x10                    ! call BIOS to do it

        mov     ax, #0x0200             ! Home the cursor
        xor     bx, bx
        xor     dx, dx
        int     0x10                    ! call BIOS to home cursor 
        ret 

_diskr:                             
        push  bp
        mov   bp,sp

        movb  dl, #0x00        ! drive 0=fd0
        movb  dh, 6[bp]        ! head
        movb  cl, 8[bp]        ! sector
	incb  cl
        movb  ch, 4[bp]        ! cyl
        mov   ax, #0x0202      ! READ 2 sectors 
        mov   bx, 10[bp]       ! put buf value in BX ==> addr=[ES,BX]
        int  0x13              ! call BIOS to read the block 
        jb   error             ! to error if CarryBit is on [read failed]

        pop   bp
        ret

_diskw:                             
        push  bp
        mov   bp,sp

        movb  dl, #0x00        ! drive 0=fd0
        movb  dh, 6[bp]        ! head
        movb  cl, 8[bp]        ! sector
        movb  ch, 4[bp]        ! cyl
        mov   ax, #0x0302      ! WRITE 2 sectors 
        mov   bx, 10[bp]       ! put buf value in BX ==> addr=[ES,BX]
        int  0x13              ! call BIOS to read the block 
        jb   error             ! to error if CarryBit is on [read failed]

        pop  bp
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
        movb   bl,#0x000B      ! bL = cyan
        int    0x10            ! call BIOS to display the char

        mov    sp,bp
        pop    bp
        ret
	
_setes:  
         push  bp
         mov   bp,sp

         mov   ax,4[bp]        
         mov   es,ax

         pop   bp
         ret

_inces:                        ! inces[] inc ES by 0x40, or 1K
         mov   ax,es
         add   ax,#0x40
         mov   es,ax
         ret

        !------------------------------
        !       error & reboot
        !------------------------------
error:
        mov  bx, #bad
        push bx
        call _prints
        
        int  0x19                       ! reboot
bad:            .asciz  "Error!"

!*===========================================================================*
!*				get_byte				     *
!*===========================================================================*
! This routine is used to fetch a byte from anywhere in memory.
! The call is: c = get_byte(segment, offset)
_get_byte:
	push bp			! save bp
	mov bp,sp		! we need to access parameters
	push es			! save es
	mov es,4[bp]		! load es with segment value
	mov bx,6[bp]		! load bx with offset from segment
	seg es			! go get the byte
	movb al,[bx]		! al = byte
	xorb ah,ah		! ax = byte
	pop es			! restore es
	pop bp			! restore bp
	ret			! return to caller

!*===========================================================================*
!*				put_byte				     *
!*===========================================================================*
! This routine is used to put a word to anywhere in memory.
! The call is: put_byte(char,segment,offset)
_put_byte:
	push bp			! save bp
	mov  bp,sp		! we need to access parameters
	push es			! save es
        push bx

	mov  es,6[bp]   	! load es with seg value
	mov  bx,8[bp]		! load bx with offset from segment
        movb al,4[bp]           ! load byte in aL
	seg  es			! go put the byte to [ES, BX]
	movb [bx],al		! there it goes

        pop  bx                 ! restore bx
	pop  es			! restore es
	pop  bp			! restore bp
	ret			! return to caller

	
! sr = int_off(), int_on(sr) functions
_int_off:             ! cli, return old flag register
        pushf
        cli
        pop ax
        ret

_int_on:              ! int_on(int SR)
        push bp
        mov  bp,sp
        mov  ax,4[bp] ! get SR passed in
        push ax
        popf
        pop  bp
        ret

!*===========================================================================*
!*				lock					     *
!*===========================================================================*
_lock:  
	cli			! disable interrupts
	ret			! return to caller

!*===========================================================================*
!*				unlock					     *
!*===========================================================================*
_unlock:
	sti			! enable interrupts
	ret			! return to caller


!*===========================================================================*
!*				in_byte					     *
!*===========================================================================*
! byte = in_byte(port)

_in_byte:
        push    bp
        mov     bp,sp
        mov     dx,4[bp]
	inb     al,dx		! input 1 byte
	subb	ah,ah		! unsign extend
        pop     bp
        ret

!*===========================================================================*
!*				out_byte				     *
!*==============================================================
! out_byte(port, int value);
! Write  value  (cast to a byte) to an I/O port

_out_byte:
        push    bp
        mov     bp,sp
        mov     dx,4[bp]
        mov     ax,6[bp]
	outb	dx,al   	! output 1 byte
        pop     bp
        ret
