; Created by The Bitripper
; < bitripper (at-sign) enigma.demon.nl >

.MODEL TINY
.386
.CODE
ORG 100h
Main:

                mov     al,13h
                int     10h

                xor     ax,ax

                mov     di,offset flames
                mov     cx,(flh*flw)
                rep     stosw

                mov     dx,3c8h
                out     dx,al
                inc     dx

                dec     cl
Check_Red:
                cmp     bl,60
                jae     check_green
                add     bl,4
                jmp     check_number
Check_Green:
                cmp     bh,60
                jae     check_number
                add     bh,4
Check_Number:
                mov     al,bl
                out     dx,al
                mov     al,bh
                out     dx,al
                xor     al,al
                out     dx,al
                loop    check_red

Do_Fire:        mov     cl,spots
Put_Spots:

                add     [randseed],62e9h
                add     byte ptr [randseed],62h
                adc     [randseed+2],3619h

                mov     ax,[randseed+2]
                xor     dx,dx
                mov     bx,flw
                div     bx

                mov     si,dx
                dec     byte ptr [flames+((flh-1)*flw)+si]
                loop    put_spots


		'average flames buffer, write to new_flames
                mov     si,offset flames+1+flw
                mov     di,offset new_flames+1

                mov     cl,flh-2
Avg_Col:        mov     dx,flw-2
Avg_Row:
                mov     bl,[si-flw]

                mov     al,[si-1]
                add     bx,ax

                mov     al,[si+1]
                add     bx,ax

                mov     al,[si+flw]
                add     bx,ax

                shr     bx,2
                mov     [di],bl

                inc     si
                inc     di
                dec     dx
                jnz     avg_row

                inc     si
                inc     si
                inc     di
                inc     di
                loop    avg_col

		'copy new flames to flames
                mov     si,offset new_flames+2
                mov     di,offset flames+2
                mov     cx,flw*flh/2-2
                push    cx
                push    di
                rep     movsw

                pop     si

                push    0a000h
                pop     es

		'store flames to video buffer
                mov     di,(200-flh)*320+((320-flw)/2)+2
                pop     cx
                rep     movsw

                push    ds
                pop     es

                mov     ah,1
                int     16h
                jz      do_fire
Key_Pressed:
                mov     ax,3
                int     10h
                ret

FlH             =       100
FlW             =       320
Spots           =       200
RandSeed        dw      ?,?
Flames          db      (flw*flh) dup (?)
New_Flames      db      (flw*flh) dup (?)

END main
