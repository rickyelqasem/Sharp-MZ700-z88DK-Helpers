SECTION code_user

PUBLIC _mz700_joy1_x
PUBLIC _mz700_joy1_y
PUBLIC _mz700_joy1_fire1
PUBLIC _mz700_joy1_fire2
PUBLIC _mz700_joy2_x
PUBLIC _mz700_joy2_y
PUBLIC _mz700_joy2_fire1
PUBLIC _mz700_joy2_fire2
PUBLIC _mz700_bank_vram_mmio
PUBLIC _mz700_bank_ram

PORTC           EQU 0E002H
PORTJ           EQU 08H
BANKH_VRAM_MMIO EQU 0E3H
BANKH_RAM       EQU 0E1H

_mz700_bank_vram_mmio:
    push bc
    ld c,BANKH_VRAM_MMIO
    out (c),a
    pop bc
    ret

_mz700_bank_ram:
    push bc
    ld c,BANKH_RAM
    out (c),a
    pop bc
    ret

_mz700_joy1_x:
    call bank_vram_mmio_local
    call JA1
    call bank_ram_local
    ld l,a
    ld h,00H
    ret

_mz700_joy1_y:
    call bank_vram_mmio_local
    call JA2
    call bank_ram_local
    ld l,a
    ld h,00H
    ret

_mz700_joy1_fire1:
    call bank_vram_mmio_local
    call SW1
    call bank_ram_local
    ld l,a
    ld h,00H
    ret

_mz700_joy1_fire2:
    call bank_vram_mmio_local
    call SW2
    call bank_ram_local
    ld l,a
    ld h,00H
    ret

_mz700_joy2_x:
    call bank_vram_mmio_local
    call JB1
    call bank_ram_local
    ld l,a
    ld h,00H
    ret

_mz700_joy2_y:
    call bank_vram_mmio_local
    call JB2
    call bank_ram_local
    ld l,a
    ld h,00H
    ret

_mz700_joy2_fire1:
    call bank_vram_mmio_local
    call SW3
    call bank_ram_local
    ld l,a
    ld h,00H
    ret

_mz700_joy2_fire2:
    call bank_vram_mmio_local
    call SW4
    call bank_ram_local
    ld l,a
    ld h,00H
    ret

bank_vram_mmio_local:
    push bc
    ld c,BANKH_VRAM_MMIO
    out (c),a
    pop bc
    ret

bank_ram_local:
    push bc
    ld c,BANKH_RAM
    out (c),a
    pop bc
    ret

; Original MZ-1X03 stick control routine from joy.asm
JA1:
    ld a,4EH
    jr LP0

JA2:
    ld a,56H
    jr LP0

JB1:
    ld a,5EH
    jr LP0

JB2:
    ld a,66H

LP0:
    di
    push hl
    push de
    ld (LP4+2),a
    ld de,0FFFFH
    ld hl,PORTC

LP1:
    bit 7,(hl)
    jp z,LP1

LP2:
    bit 7,(hl)
    jp nz,LP2

    ld l,PORTJ
    ld a,11H

LP3:
    dec a
    jr nz,LP3

LP4:
    inc de
    bit 0,(hl)
    jp z,LP4

    ld a,d
    or a
    ld a,e
    pop de
    pop hl
    ei
    ret z

    ld a,0FFH
    ret

; Original MZ-1X03 switch routine from joy.asm
SW1:
    ld a,4EH
    jr LP5

SW2:
    ld a,56H
    jr LP5

SW3:
    ld a,5EH
    jr LP5

SW4:
    ld a,66H

LP5:
    di
    ld (LPB+1),a
    ld (LP7+1),a
    push hl
    ld hl,PORTC

LP6:
    bit 7,(hl)
    jp z,LP6

    ld l,PORTJ

LP7:
    bit 0,(hl)
    ld a,l
    jp z,LP8
    xor a

LP8:
    ld (LPD),a
    ld a,9

LP9:
    push af
    xor a

LP9A:
    dec a
    jr nz,LP9A
    pop af
    dec a
    jr nz,LP9

    ld l,2

LPA:
    bit 7,(hl)
    jp z,LPA

    ld l,PORTJ

LPB:
    bit 0,(hl)
    ld a,l
    jp z,LPC
    xor a

LPC:
    ld l,a
    db 3EH
LPD:
    db 00H
    and l
    pop hl
    ei
    ret
