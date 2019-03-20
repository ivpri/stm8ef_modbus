; Modbus support low level words

;===============================================================


        .macro  YSTOR
        DECW    X
        DECW    X
        LDW     (X),Y           ; push avg-addr to stack
        .endm


        .ifne   HAS_MBPHY * HAS_MODBUS_FB
        
        .macro  MBPHYRX
        .ifne   MBPHY_DERE_INV
        BSET    MBPHY_DERE+ODR,#MBPHY_DERE_PIN
        .else
        BRES    MBPHY_DERE+ODR,#MBPHY_DERE_PIN
        .endif
        .endm

        .macro  MBPHYTX
        .ifeq   MBPHY_DERE_INV
        BSET    MBPHY_DERE+ODR,#MBPHY_DERE_PIN
        .else
        BRES    MBPHY_DERE+ODR,#MBPHY_DERE_PIN
        .endif
        .endm

        .endif


	.ifeq	USE_UART2
	UART_SR	  = UART1_SR	
	UART_CR2  = UART1_CR2	
	UART_DR	  = UART1_DR
	UART_BRR1 = UART1_BRR1
	.else
	UART_SR	  = UART2_SR	
	UART_CR2  = UART2_CR2	
	UART_DR	  = UART2_DR
	UART_BRR1 = UART2_BRR1
	.endif


;===============================================================

        
;       INI  ( -- )
;       Initialization of available subsystem on boot time. 
;       - Initialize coils/hw from holding regs
;       - Input registers for ADC statistics
;       - Modbus physical layer initialization 
        
        .ifne   HAS_INI

        HEADERL 3 "INI"

        .ifne   COILS_HOLDING_REG * HAS_OUTPUTS
        
MBHRINI:
        LDW     Y,HOLDING_REGS_BASE+(2*(COILS_HOLDING_REG-1))
        YSTOR
        CALL    OUTSTOR
        .endif                  ; COILS_HOLDING_REG * HAS_OUTPUTS

        .ifne   TIM_HOLDING_REGS
        LD      A,#HAS_HOLDING_REGS
1$:     
        CALL    HRTOTIMR
        INC     A
        CP      A,#HAS_HOLDING_REGS+TIM_HOLDING_REGS
        JRULT   1$
        .endif                  ; TIM_HOLDING_REGS       


        .ifne   HAS_INPUT_REGS_ADC * HAS_INPUT_REGS_STA
        
MBIRSTARST_ALL:                 ; called by OUT!, A contains min reg offset (first is 0)
        CLR     A
1$:
        CALL    MBIRSTARST
        ADD     A,#8            ; move to next min reg
        CP      A,#8*HAS_INPUT_REGS_ADC
        JRNE    1$
        LD      A,(X)        

        .endif                  ; HAS_INPUT_REGS_ADC * HAS_INPUT_REGS_STA


        .ifne   HAS_MBPHY * HAS_MODBUS_FB

MBINIT:
        MOV     UART_CR2,#0x3C  ; RIEN + ILIEN + TEN + REN
        CLR     MODBUS_FBSZ
        CLR     MODBUS_FBRD

        PUSHW   X
        ;; Configure baud rate
        LDW     X,#HOLDING_REGS_BASE+2 ; baud idx
        LDW     X,(X)
        SLAW    X
        ADDW    X,#MB_BAUD_IDX_BRR
        LDW     X,(X)
        LDW     Y,#UART_BRR1
1$:
        BTJF    UART_SR,#6,1$  ; TC - transmission complete?
        LDW     (Y),X       
        MBPHYRX
        
        POPW    X

        .endif                  ; HAS_MBPHY * HAS_MODBUS_FB
        
        RET

        .endif                  ; HAS_INI        
        

;===============================================================

;       RST  ( -- )
;       Initialize HR# IR# and clear custom IR@ and 'TRIG
;       handlers. Use with reset e.g.: nvm : reset rst reset ;
        
        .ifne   HAS_CUST_TRIG + (HAS_HOLDING_REGS * HAS_CUST_HR) + HAS_CUST_IR

        HEADERL 3 "RST"

        CALL    UNLOCK_FLASH
        EXGW    X,Y
        CLRW    X

        .ifne   HAS_CUST_TRIG
        LDW     EVU_CUST_TRIG+1,X
        .endif

        .ifne   (HAS_HOLDING_REGS * HAS_CUST_HR)
        MOV     MBHRNUMBER+1,#(HAS_HOLDING_REGS+TIM_HOLDING_REGS)
        .endif
        
        .ifne   HAS_CUST_IR
        MOV     MBIRNUMBER+1,#INPUT_REGS_BUILDIN
        LDW     CUST_IR_CALL+1,X
        .endif

        EXGW    X,Y
        JP      LOCK_FLASH
        
        .endif
        
;===============================================================

        .ifne   HAS_LED7SEG
;       LED_MPX driver ( -- )
;       Code called from ISR for LED MPX

LED_MPX:
        RET
        .endif       

                
;===============================================================


;       2C!  ( n a -- )
;       Store word C-wise to 16 bit HW registers "MSB first"
;       Note: 2C! and 2C@ are required to access 16bit timer
;             registers but on other side e.g. UART BRR needs
;             usage of @ and ! (LDW - LSB first)
        
        ;; This is just a copy of the words found in EXTRAMEM set
        .ifne   HAS_2CSTORE
        .ifeq   WORDS_EXTRAMEM

        HEADERL 3 "2C!"
DCSTOR:
        CALL    YFLAGS          ; a
        LD      A,(X)
        LD      (Y),A           ; write MSB(n) to a
        INCW    X
        LD      A,(X)
        LD      (1,Y),A         ; write LSB(n) to a+1
        INCW    X
        RET
        
        .endif
        .endif
        
;===============================================================

;       LSHIFT ( x1 n -- x2 )       

        .ifne   HAS_LSHIFT

        HEADERL 6 "LSHIFT"
LSHIFT:
        LD      A,(1,X)         ; n
        CALL    DROP
        LDW     Y,X             
        LDW     X,(X)
1$:
        DEC     A
        JRMI    2$
        SLAW    X
        JRA     1$
2$:
        EXGW    X,Y        
        LDW     (X),Y
        RET        
        .endif

;===============================================================

;       RSHIFT ( x1 n -- x2 )

        .ifne   HAS_RSHIFT
        HEADERL 6 "RSHIFT"
RSHIFT:
        LD      A,(1,X)         ; n
        CALL    DROP
        LDW     Y,X             
        LDW     X,(X)
1$:
        DEC     A
        JRMI    2$
        SRLW    X
        JRA     1$
2$:
        EXGW    X,Y        
        LDW     (X),Y
        RET        
        .endif

;===============================================================
        
;       EXG      ( n -- n )      ( TOS STM8: -- Y,Z,N )
;       Exchange high with low byte of n.

        .ifne   BAREBONES
        HEADERL 3 "EXG"
EXG:    
        CALL    DOXCODE
        SWAPW   X
        RET
        .endif
        
;===============================================================


;       CRC16  ( x1 c -- x2 )
;       add c to CRC16 x1 to produce x2.
;       Initial value for MODBUS is $FFFF

	.ifne   HAS_CRC16
        HEADERL 5 "CRC16"
CRC16:
        CALL    XORR
        CALL    DOXCODE         ; to save 2B
CRC16B:
        LD      A,#8

CRC16LOOP:
        SRLW    X
        JRNC    1$
        PUSH    A
        LD      A,XH
        XOR     A,#0xA0
        LD      XH,A
        LD      A,XL
        XOR     A,#0x01
        LD      XL,A
        POP     A
1$:
        DEC     A
        JRNE    CRC16LOOP

        RET        
        .endif

       
;===============================================================

        .ifne   HAS_MODBUS_FB

        MODBUS_FBSIZE_ALL = MODBUS_FBSIZE + 2

        RamBlck MODBUS_FBB, MODBUS_FBSIZE_ALL

        MODBUS_FBSZ = MODBUS_FBB
        MODBUS_FBRD = MODBUS_FBB + 1
        MODBUS_FB   = MODBUS_FBB + 2
        
;       FB  ( -- a )
;       Frame buffer address
;       First byte on address a contains actual size of data
;       in the buffer. The second byte is buffer data read index.
;       Buffer data start on third byte.
        
        HEADERL 2 "FB"
MBFBFIRST:
        LDW     Y,#(MODBUS_FBB) 
        JP      YSTOR

        
;       FBC!  ( c -- )
;       Append char to frame buffer

        HEADERL 4 "FBC!"
MBFBCSTOR:
        LD      A,MODBUS_FBSZ
        CP      A,#(MODBUS_FBSIZE) ; C if A is U<
        JRNC    1$

        CLRW    Y
        LD      YL,A
        LD      A,(1,X)         ; c
        ADDW    Y,#(MODBUS_FB)
        LD      (Y),A
        INC     MODBUS_FBSZ
1$:     
        JP      DROP

        
;       FB!  ( x -- )
;       Append word to frame buffer, MSB first

        HEADERL 3 "FB!"
MBFBSTOR:
        LD      A,(0,X)         ; MSB
        CALL    ASTOR
        CALL    MBFBCSTOR
        JP      MBFBCSTOR       
                
        
;       FBC@  ( -- c )
;       Retrieve next char from buffer

        HEADERL 4 "FBC@"
MBFBCFETCH:
        CALLR   MBFBCFA
        LD      A,(Y)
        JP      ASTOR

MBFBCFA:
        LD      A,MODBUS_FBRD        
        INC     MODBUS_FBRD
        CLRW    Y
        LD      YL,A
        ADDW    Y,#(MODBUS_FB)
        RET

        
;       FB@  ( -- c )
;       Retrieve next word from buffer

        HEADERL 3 "FB@"
MBFBFETCH:
        CALLR   MBFBCFA
        LDW     Y,(Y)
        INC     MODBUS_FBRD
        JP      YSTOR
        
        .endif

        
;===============================================================

        .ifne   HAS_MBPHY * HAS_MODBUS_FB

        ;; table of UART_BRR values for common baud rates:
        ;;  1200  2400  4800   9600
        ;; 19200 38400 57600 115200

MB_BAUD_IDX_BRR:
        .dw     0x4136
        .dw     0xA01B
        .dw     0xC005
        .dw     0x6803
        .dw     0x3401
        .dw     0x1A00
        .dw     0x1106
        .dw     0x080B
       
        
;       BR#  ( -- )
;       Available baud rates count

        HEADERL 3 "BR#"
        LD      A,#8
        JP      ASTOR
        
        
;       MBTX  ( -- )
;       Transmit next byte from frame buffer.

        HEADERL 4 "MBTX"
MBTX:
        MBPHYTX
        CALLR   MBFBCFA
        LD      A,(Y)
        LD      UART_DR,A

        LD      A,MODBUS_FBSZ
        CP      A,MODBUS_FBRD
        JRUGT   1$
        JP      MBINIT          ; end of buffer - switch to RX
1$:     
        BSET    UART_CR2,#7    ; TIEN = 1 => enable tx interrupts
        RET

        .endif                  ; HAS_MBPHY * HAS_MODBUS_FB

;===============================================================

;       MBCRC  ( n -- x )
;       Compute modbus CRC16 from first n
;       characters of frame buffer

        .ifne   HAS_MBCRC * HAS_MODBUS_FB
        HEADERL 5 "MBCRC"
MBCRC:
        LDW     Y,#(MODBUS_FB) 
        LD      A,(1,X) ; n
        LD      MODBUS_FBRD,A
        PUSHW   X
        LDW     X,#0xFFFF
        
MBCRCLOOP:
        PUSH    A
        LD      A,XL
        XOR     A,(Y)
        LD      XL,A

        .ifne   HAS_CRC16
        CALLR   CRC16B
        
        .else
        LD      A,#8

CRC16LOOP:
        SRLW    X
        JRNC    1$
        PUSH    A
        LD      A,XH
        XOR     A,#0xA0
        LD      XH,A
        LD      A,XL
        XOR     A,#0x01
        LD      XL,A
        POP     A
1$:
        DEC     A
        JRNE    CRC16LOOP
        
        .endif                  ; HAS_CRC16        

        POP     A
        INCW    Y
        DEC     A
        JRNE    MBCRCLOOP

        SWAPW   X
        EXGW    X,Y
        POPW    X
        LDW     (X),Y
        RET  
        .endif

        
;===============================================================

        .ifne   HAS_HOLDING_REGS

;       HR#  ( -- n )
;       Number of holding registers.

        HEADERL 3 "HR#"
MBHRNUMBER:
        LD      A,#(HAS_HOLDING_REGS+TIM_HOLDING_REGS)
        JP      ASTOR

        .ifne   HAS_CUST_HR

;       HR#+ ( n -- )
;       Extend number of holding registers. Use with NVM e.g.:
;       NVM 4 HR#+
        
        HEADERL 4 "HR#+"

        INCW    X
        LD      A,(X)
        ADD     A,MBHRNUMBER+1
        LD      MBHRNUMBER+1,A
        INCW    X
        RET
        
        .endif                  ; HAS_CUST_HR
        
        
;       HR@  ( n -- x )
;       Read content of holding register

        HEADERL 3 "HR@"
MBHRFETCH:
        CALL    DOXCODE
        SLAW    X
        ADDW    X,#HOLDING_REGS_BASE
        LDW     X,(X)
        RET
        

;       HR!  ( x n -- )
;       Write content of holding register

        HEADERL 3 "HR!"
MBHRSTOR:
        PUSHW   X
        LDW     Y,X
        LDW     X,(X)           ; n
        SLAW    X               ; 2*
        ADDW    X,#HOLDING_REGS_BASE   ; addr -> X
        LDW     Y,(2,Y)         ; x -> Y
        ;; only write if content changes
        CPW     Y,(X)
        JREQ    1$
        
        CALL    ULOCK
        LDW     (X),Y
        CALL    LOCK
1$:
        POPW    X
        LD      A,(1,X)         ; n
        CALLR   HRTOTIMR        
        JP      DDROP

        
        ;; Sync content of holding register n in A
        ;; with corresponding timer register
        ;; content of A is not affected
HRTOTIMR:
        CP      A,#HAS_HOLDING_REGS
        JRULT   1$

        PUSHW   X
        CLRW    X
        LD      XL,A
        SUBW    X,#HAS_HOLDING_REGS
        SLAW    X               ; 2*
        LDW     Y,X

        ADDW    X,#HRTIMRMAP
        LDW     X,(X)           ; Timer register address

        EXGW    X,Y
        ADDW    X,#HOLDING_REGS_BASE+(2*HAS_HOLDING_REGS)
        LDW     X,(X)           ; HR content
        EXGW    X,Y

        PUSH    A
        LD      A,YH
        LD      (X),A
        LD      A,YL
        LD      (1,X),A
        POP     A
        
        POPW    X
1$:
        RET

HRTIMRMAP:
        .dw     TIM_HOLDING_REG0

        .ifge TIM_HOLDING_REGS - 2
        .dw     TIM_HOLDING_REG1
        .endif

        .ifge TIM_HOLDING_REGS - 3
        .dw     TIM_HOLDING_REG2
        .endif

        .ifge TIM_HOLDING_REGS - 4
        .dw     TIM_HOLDING_REG3
        .endif

        .ifge TIM_HOLDING_REGS - 5
        .dw     TIM_HOLDING_REG4
        .endif

        .ifge TIM_HOLDING_REGS - 6
        .dw     TIM_HOLDING_REG5
        .endif

        .ifge TIM_HOLDING_REGS - 7
        .dw     TIM_HOLDING_REG6
        .endif

        .ifge TIM_HOLDING_REGS - 8
        .dw     TIM_HOLDING_REG7
        .endif

        .ifge TIM_HOLDING_REGS - 9
        .dw     TIM_HOLDING_REG8
        .endif

        .ifge TIM_HOLDING_REGS - 10
        .dw     TIM_HOLDING_REG9
        .endif
        
        .endif  HAS_HOLDING_REGS

        ;; debug & dev
        ;; HEADERL 1 "D"
        ;; LDW     Y,#TIM2_CCR1H
        ;; YSTOR
        ;; LD      A,#10
        ;; CALL    ASTOR
        ;; JP      DUMP      

        
;===============================================================

        ; Input registers for ADC readings for Modbus driver
        .ifne   HAS_INPUT_REGS_ADC + HAS_INPUT_REGS_TIM

        INPUT_REGS_ADC_NUM = HAS_INPUT_REGS_ADC * (1 + 4 * HAS_INPUT_REGS_STA)
        INPUT_REGS_BUILDIN = INPUT_REGS_ADC_NUM + HAS_INPUT_REGS_TIM * 8
        
        RamBlck MBIRBUF, INPUT_REGS_BUFSIZE        


;       IR#  ( -- n )
;       Number of input registers. One per configured ADC
;       channel plus 4 (min, max, avg, n) per channel if statistics
;       are available (see: ADCIRSTA)
        
        HEADERL 3 "IR#"
MBIRNUMBER:
        LD      A,#INPUT_REGS_BUILDIN
        JP      ASTOR


        .ifeq   HAS_INPUT_REGS_STA
        ;; mapping IR number -> ADC channel
MBIRADCCH:
        .db     INPUT_REG0_ADCCH

        .ifge HAS_INPUT_REGS_ADC - 2
        .db     INPUT_REG1_ADCCH
        .endif

        .ifge HAS_INPUT_REGS_ADC - 3
        .db     INPUT_REG2_ADCCH
        .endif

        .ifge HAS_INPUT_REGS_ADC - 4
        .db     INPUT_REG3_ADCCH
        .endif

        .endif  ; HAS_INPUT_REGS_STA


        .ifne   HAS_CUST_IR

;       IR#+ ( n -- )
;       Extend number of input registers. Use with NVM e.g.:
;       NVM 4 IR#+
        
        HEADERL 4 "IR#+"

        INCW    X
        LD      A,(X)
        ADD     A,MBIRNUMBER+1
        LD      MBIRNUMBER+1,A
        INCW    X
        
        RET


;       'IR@ ( -- a )
;       Custom IR@ handler for input registers
;       called as ( n -- x ) where n is offset from buildin registers.
        
        HEADERL 4 "'IR@"

        LDW     Y,#CUST_IR_CALL+1
        JP      YSTOR
        
        .endif                  ; HAS_CUST_IR


        
;       IR@  ( n -- x )
;       Read input register n.
;       Let c is number of ADC channels available 
;       (see HAS_INPUT_REGS_ADC = c in globconf.inc)
;       Then registers 0 - c-1 contains actual values of 
;       ADC conversions on channels. 
;       If statistics are available (see word ADCIRSTA) 
;       and OUT! is available for coils write and coil 7 (PDU addressing)
;       is set then reading of n register causes all statistic registers 
;       of this channel to reset its values:
;       max = avg = n = 0, min = $ffff
;       Also transition 0 -> 1 on coil 7 causes all channels statistics to reset.       
        
        HEADERL 3 "IR@"
MBIRFETCH:
        
        .ifne   HAS_CUST_IR
        
        LD      A,(1,X)
        CP      A,#INPUT_REGS_BUILDIN
        JRULT   CUST_IR_CALL_SKIP

        LDW     Y,#CUST_IR_CALL+1
        LDW     Y,(Y)
        TNZW    Y
        JREQ    CUST_IR_CALL_SKIP

        SUB     A,#INPUT_REGS_BUILDIN
        LD      (1,X),A
CUST_IR_CALL:
        JP      0        
CUST_IR_CALL_SKIP:
        
        .endif                  ; HAS_CUST_IR

        
        CALL    DOXCODE
        LD      A,XL            ; n

        .ifne   HAS_INPUT_REGS_TIM * HAS_EVTIMS
        CP      A,#INPUT_REGS_ADC_NUM
        JRULT   2$
        
        SUB     A,#INPUT_REGS_ADC_NUM
        LD      XL,A
        SLLW    X
        ADDW    X,#TIMERSBUF
        LDW     X,(X)
        RET
2$:     
        .endif                  ; HAS_INPUT_REGS_TIM

        .ifeq   HAS_INPUT_REGS_STA
        ;; directly read assigned channel
        ADDW    X,#(MBIRADCCH)
        LD      A,(X)

        BSET    ADC_CR2,#3      ; align ADC to LSB
        LD      ADC_CSR,A       ; select channel
        BSET    ADC_CR1,#0      ; enable ADC
        BSET    ADC_CR1,#0      ; second bit write to start conversion
1$:
        BTJF    ADC_CSR,#7,1$   ; wait until EOC
        LDW     X,ADC_DRH       ; read ADC
        BRES    ADC_CR1,#0      ; disable ADC
        

        .else                   ; HAS_INPUT_REGS_STA
        SLAW    X               ; *2
        ADDW    X,#(MBIRBUF)
        
        SUB     A,#(HAS_INPUT_REGS_ADC)
        AND     A,#0x03
        CP      A,#0x03         ; n register?
        JRNE    MBIRFETCHEND

        .ifne   HAS_OUTPUTS
        ;; reset statistics only if bit 6 is set
        LD      A,OUTPUTS+1
        AND     A,#0x20
        JREQ    MBIRFETCHEND    
        .endif                  ; HAS_OUTPUTS

        LDW     Y,X
        LDW     X,(X)
        PUSHW   X        
        CLRW    X               ; reading of n reg resets channel's input regs
        EXGW    X,Y

        SUBW    X,#6            ; min addr

MBIRSTARSTX:
        LDW     (2,X),Y         ; clear max
        LDW     (4,X),Y         ; clear avg
        LDW     (6,X),Y         ; clear n
        CPLW    Y
        LDW     (X),Y           ; min = $FFFF

        POPW    X
        RET

MBIRFETCHEND:
        LDW     X,(X)
        RET

        
        MBIRBUFSTA = MBIRBUF+(2*HAS_INPUT_REGS_ADC)
        
MBIRSTARST:                     ; called by OUT!, A contains min reg offset (first is 0)
        PUSHW   X
        CLRW    X
        LDW     Y,X
        LD      XL,A
        ADDW    X,#MBIRBUFSTA ; origin to min reg address
        JRA     MBIRSTARSTX

        
;       ADCIRSTA  ( n -- x )
;       Start ADC conversion on configured channels and update input registers.
;       Also update register quads min max avg n which follow for each channel.
;       When n is full ($FFFF) statistics (all min, max, avg, n) are not
;       updated by new values read by ADCIRSTA until statistics are reset (see IR@)
        
        HEADERL 8 "ADCIRSTA"

ADCIRSTA_START:
        BSET    ADC_CR2,#3                      ; align ADC to LSB
        MOV     ADC_CSR,#INPUT_REG0_ADCCH       ; select first channel
        BSET    ADC_CR1,#0                      ; enable ADC
        LDW     Y,#MBIRBUF                      ; IR buffer for first channel
        CALLR   ADCIRSTA_UPD

        .ifge HAS_INPUT_REGS_ADC - 2
        MOV     ADC_CSR,#INPUT_REG1_ADCCH       ; select first channel
        LDW     Y,#MBIRBUF+2                    ; IR buffer for second channel
        CALLR   ADCIRSTA_UPD
        .endif

        .ifge HAS_INPUT_REGS_ADC - 3
        MOV     ADC_CSR,#INPUT_REG2_ADCCH       ; select first channel
        LDW     Y,#MBIRBUF+4                    ; IR buffer for third channel
        CALLR   ADCIRSTA_UPD
        .endif

        .ifge HAS_INPUT_REGS_ADC - 4
        MOV     ADC_CSR,#INPUT_REG3_ADCCH       ; select first channel
        LDW     Y,#MBIRBUF+6                    ; IR buffer for fourth channel
        CALLR   ADCIRSTA_UPD
        .endif        

        BRES    ADC_CR1,#0      ; disable ADC
        RET

        ;; Start ADC conversion for a channel already set in ADC_CSR
        ;; and update input register buffers which actual value address is in Y
ADCIRSTA_UPD:
        BSET    ADC_CR1,#0      ; second bit write to start conversion

        ;; dev / debug
        ;; PUSHW   Y
        ;; BSET    ADC_CSR,#7
        ;; LDW     Y,X
        ;; LDW     X,(X)
        ;; LDW     ADC_DRH,X
        ;; EXGW    X,Y
        ;; POPW    Y
        
1$:
        BTJF    ADC_CSR,#7,1$   ; wait until EOC
        PUSHW   X
        LDW     X,ADC_DRH       ; read ADC
        EXGW    X,Y
        LDW     (X),Y           ; write to actual value IR
        SUBW    X,#MBIRBUF
        SLAW    X               ; 2*
        SLAW    X               ; 2*
        ADDW    X,#MBIRBUFSTA   ; move to min IR for actual channel

        LDW     Y,#0xFFFF
        CPW     Y,(6,X)
        JRNE    2$              ; skip updating statistics if n is on max val
        POPW    X       
        RET        
2$:
        LDW     Y,ADC_DRH       ; read ADC again        
        CPW     Y,(X)
        JRUGE   3$              ; update min?
        LDW     (X),Y
3$:
        CPW     Y,(2,X)
        JRULE   4$              ; update max?
        LDW     (2,X),Y     
4$:
        ADDW    X,#4            ; avg address
        POPW    Y               ; forth SP
        EXGW    X,Y
       
        PUSHW   Y
        YSTOR                   ; push avg-addr to both stacks
        CALL    DAT             ; 2@  \ avg n                
        CALL    UMSTA           ; UM* \ dsum

        LDW     Y,X        
        LDW     X,(2,X)         ; dsum LSW
        ADDW    X,ADC_DRH
        LDW     (2,Y),X
        EXGW    X,Y
        JRNC    5$              ; carry for dsum MSW?
        CALL    ONEP            ; 1+
5$:
        PUSHW   X
        LDW     X,(3,SP)        ; get avg-addr and retrieve n
        INCW    X
        INCW    X
        LDW     Y,X
        LDW     X,(X)
        INCW    X
        EXGW    X,Y
        LDW     (X),Y           ; store n+1 to IR buffer
        POPW    X
        YSTOR                   ; dsum n+1
        
        CALL    UMMOD           ; UM/MOD ( dsum n+1 -- mod avg )

        POPW    Y               ; get avg-addr
        PUSHW   X
        LDW     X,(X)           ; get avg from param stack
        LDW     (Y),X           ; store new avg
        POPW    X
        
        JP      DDROP
        
        .endif                  ; HAS_INPUT_REGS_STA
               
        .endif                  ; HAS_INPUT_REGS_ADC


;===============================================================

; Button and timer events

        .ifne   HAS_EVUPD

        RamByte LASTINP         ; last value of INP@

        .ifne   HAS_EVTIMS

        EVTIMS_BUF_SIZE = 4 * HAS_EVTIMS
        
        RamByte TIMSEN          ; bits 0-3: timer 1-4 enables
        RamBlck TIMERSBUF, EVTIMS_BUF_SIZE   ; 32bit timers counters


        .ifne   HAS_TIMCNT

;       TIMCNT  ( n -- a )
;       a is address of 32 bit timer counter n (0 - 3)

        HEADERL 6 "TIMCNT"
TIMCNT:
        CALL    DOXCODE
        SLLW    X
        SLLW    X
        ADDW    X,#TIMERSBUF
        RET

        .endif
        .endif                  ; HAS_EVTIMS

        .ifne   HAS_EVU
        
;       EVU    ( -- )
;       Run this as BG job, each 5ms.
;       Execute ADCIRSTA and EVUPD each 1/100s. If 'TRIG custom handler is set
;       then run it as ( x1 -- x2 ). Then call TRIGG for each event and assigned holding register.
        
        HEADERL 3 "EVU"

        LD      A,TICKCNT+1
        BCP     A,#1
        JREQ    1$              ; proceed on last bit = 0 every second tick (assume 5ms BG)
        RET
1$:     

        .ifne   HAS_INPUT_REGS_ADC * HAS_INPUT_REGS_STA
        CALL    ADCIRSTA_START  
        .endif                  ; HAS_INPUT_REGS_ADC * HAS_INPUT_REGS_STA

        CALLR   EVUPD
        LDW     Y,X
        LDW     X,(X)
        JREQ    EVU_END
        EXGW    X,Y

        .ifne   HAS_CUST_TRIG
        LDW     Y,EVU_CUST_TRIG+1
        JREQ    EVU_CUST_TRIG_SKIP
        
EVU_CUST_TRIG:
        CALL    0
EVU_CUST_TRIG_SKIP:        
        .endif                  ; HAS_CUST_TRIG

        LD      A,#3           ; 10 input events, 4 timer events HR3 - HR16
        LDW     Y,X
        LDW     X,(X)
1$:
        SRAW    X
        JRNC    2$

        PUSH    A
        PUSHW   X
        LDW     X,Y

        CALL    ASTOR
        CALL    MBHRFETCH
        CALL    TRIGG
                
        EXGW    X,Y
        POPW    X
        POP     A
2$:     
        INC     A
        CP      A,#16
        JRULE   1$

EVU_END:        
        EXGW    X,Y     
        JP      DROP

        .endif                  ; HAS_EVU


        .ifne   HAS_CUST_TRIG * HAS_EVU

;       'TRIG    ( -- a )
;       NVM address of custom trigger handler ( x1 -- x2 ) where
;       x1 is event mask from EVUPD and x2 is result event mask to be processed
;       by triggers in the holding registers.
        
        HEADERL 5 "'TRIG"

        LDW     Y,#EVU_CUST_TRIG+1
        JP      YSTOR

        .endif                  ; HAS_CUST_TRIG * HAS_EVU

        
        
;       EVUPD  ( -- x )
;       Run this word each 1/100 s.
;       Read inputs, decrease timer counters.
;       x contains bits indicating events (MS bit first):
;       0 0 t1 t2 t3 t4 s2_rel ... in1_rel s2_press ... in1_press
;       Timer event is generated when decreasing from 1 to 0 and timer is then stopped
;       Note: counter value 0 means counting from 2^32, not timer disable!
        
        HEADERL 5 "EVUPD"
EVUPD:
        CALL    INPFETCH
        LD      A,(1,X)
        CPL     A
        AND     A,LASTINP
        LD      (X),A
        LD      A,(1,X)
        EXG     A,LASTINP
        CPL     A
        AND     A,(1,X)
        LD      (1,X),A

        .ifne   HAS_EVTIMS
        
        ;; timers update and check for events
        PUSHW   X
        PUSH    #0              ; timer events flags
        LD      A,#1
        LDW     X,#TIMERSBUF
1$:
        BCP     A,TIMSEN
        JREQ    4$

        ;; counting flag -> decrease timer counter
        LDW     Y,X
        LDW     X,(2,X)
        DECW    X
        LDW     (2,Y),X
        JRNE    2$

        LDW     X,Y
        LDW     X,(X)
        TNZW    X
        JRNE    3$

        ;; both timer words are zero -> timer stop and event
        PUSH    A
        CPL     A
        AND     A,TIMSEN
        LD      TIMSEN,A

        LD      A,(1,SP)
        OR      A,(2,SP)        ; timer events flags
        LD      (2,SP), A

        CALLR   EVTIM_RESET     ; timer counter reset from holding regs
        
        POP     A                
        JRA     3$        
2$:     
        ;; lower word 0 -> -1 => decrease higer word
        CPW     X,#-1
        JRNE    3$

        LDW     X,Y
        LDW     X,(X)
        DECW    X
        LDW     (Y),X        
3$:
        LDW     X,Y
4$:
        ADDW    X,#4
        SLL     A

        .ifeq   HAS_EVTIMS - 4
        BCP     A,#0x0F
        .endif

        .ifeq   HAS_EVTIMS - 3
        BCP     A,#0x07
        .endif

        .ifeq   HAS_EVTIMS - 2
        BCP     A,#0x03
        .endif

        .ifeq   HAS_EVTIMS - 1
        BCP     A,#0x01
        .endif
        
        JRNE    1$
        
        POP     A               ; timer event flags
        POPW    X

        .endif                  ; HAS_EVTIMS

        ;; merge timer event bits with button ones
        INCW    X
        SLL     (X)
        SLL     (X)
        SLL     (X)             ; button press masks
        DECW    X
        
        LDW     Y,X
        LDW     X,(X)
        SRLW    X
        SRLW    X
        SRLW    X               ; compact press and release masks: 000000RR RRRPPPPP
        EXGW    X,Y
        LDW     (X),Y

        .ifne   HAS_EVTIMS        
        SLL     A
        SLL     A               ; Timer flags: 00TTTT00 merge to MSB:
        OR      A,(X)
        LD      (X),A
        .endif                  ; HAS_EVTIMS
        RET

        
        ;; HEADERL 1 "u"
        ;; CALLR   EVUPD
        ;; JP      DOT
        

        .macro  PUSHPS x
        LDW     Y,x
        CALL    YSTOR
        .endm

        ;; HEADERL 1 "t"
        ;; CALL    HEX
        ;; PUSHPS  #TIMSEN
        ;; CALL    DUPP
        ;; CALL    CAT
        ;; CALL    DOT
        ;; CALL    ONEP        
        ;; PUSHPS  #5
        ;; CALL    DUMP

        ;; CALL    EVUPD
        ;; CALL    DOT
        ;; RET

        ;; HEADERL 2 "ii"
        ;; INCW    X
        ;; LD      A,(X)
        ;; LD      TIMSEN, A
        ;; INCW    X        
        ;; MOV     TIMSEN+2,#1
        ;; MOV     TIMSEN+4,#3
        ;; MOV     TIMSEN+8,#5
        ;; MOV     TIMSEN+16,#6
        ;; RET
               
        .endif                  ; HAS_EVUPD

        .ifne   HAS_EVTIMS * (HAS_EVUPD + HAS_TRIGG)

        EVTIM_HR_BASE = HOLDING_REGS_BASE + (2 * EVTIM_FIRST_HR)

        ;; Reset timer counter value from corresponding holding regs
        ;; Parameter: timer counter address in Y, Y is not changed, X is modified
EVTIM_RESET:
        LDW     X,Y
        SUBW    X,#TIMERSBUF
        ADDW    X,#EVTIM_HR_BASE
        PUSHW   X

        LDW     X,(X)
        LDW     (Y),X
        POPW    X
        LDW     X,(2,X)
        LDW     (2,Y),X
        RET
        
        .endif                  ; HAS_EVTIMS * (HAS_EVUPD + HAS_TRIGG)

        .ifne   HAS_TRIGG

;       TRIGG  ( x -- )
;       Trigger outputs and timers
;       x is bit pattern describing how to trigger four OUT! outputs
;       and four timers:
;       MSB      LSB
;       AAAABBBB CCCCDDDD
;                   ^   ^ CD0 - 2 bit pattern to trigger output 0
;                  ^   ^  CD1 - 2 bit pattern to trigger output 1
;          ^   ^          AB0 - 2 bit pattern to trigger timer 0 etc.
;       Patterns for output triggering:
;       00 - no action  01 - set to 1  10 - set to 0  11 - toggle
;       Patterns for timers triggering (optional, when HAS_EVTIMS is set):
;       00 - no action  01 - reset to initial (defined by a holding register) value and continue
;       10 - continue   11 - stop
;       Note 10 does not reset to initial value so if current value is 0 counting 
;       continues from 2^32 (see EVUPD)! Auto reset when 0 is reached must be implemented
;       on higher level when an event is generated                                ;
        
        HEADERL 5 "TRIGG"

TRIGG:
        .ifne   HAS_EVTIMS

        LD      A,(X)           ; MSB with timers action bits
        JREQ    3$              ; skip if nothing set
        SWAP    A
        OR      A,(X)
        OR      A,TIMSEN        ; enable timers for 01, 10 (and 11 for now)
        LD      TIMSEN,A

        LD      A,(X)
        SWAP    A
        AND     A,(X)
        CPL     A
        AND     A,TIMSEN        ; disable timers for 11 patterns
        AND     A,#0x0F
        LD      TIMSEN,A

        LD      A,(X)
        AND     A,TIMSEN        ; take all enabled timers with x1 pattern (so 11 is excluded => 01 only)
        PUSHW   X
        LDW     X,#TIMERSBUF
1$:     
        SRL     A
        JRNC    2$

        EXGW    X,Y             ; and resets counter from holding regs
        CALLR   EVTIM_RESET
        EXGW    X,Y
2$:     
        ADDW    X,#4
        CPW     X,#TIMERSBUF+16
        
        JRULT   1$
        POPW    X
3$:     
        .endif                  ; HAS_EVTIMS
        
        INCW    X
        LD      A,(X)           ; LSB with outputs action bits
        JREQ    4$              ; skip if nothing set
        SWAP    A
        AND     A,(X)           ; 11 pattern to toggle outputs
        AND     A,#0x0F         ; only touch low 4 bits
        XOR     A,OUTPUTS+1
        PUSH    A
        
        LD      A,(X)
        CPL     A
        SWAP    A
        AND     A,(X)           ; 01 pattern to set outputs
        AND     A,#0x0F
        OR      A,(1,SP)
        LD      (1,SP),A

        LD      A,(X)
        CPL     A
        SWAP    A
        OR      A,(X)           ; 10 pattern to clear outputs
        OR      A,#0xF0
        AND     A,(1,SP)

        LD      (X),A
        POP     A
        DECW    X       
        LD      A,OUTPUTS       ; MSB of output unchanged
        LD      (X),A
        JP      OUTSTOR        
4$:     
        INCW    X        
        RET


        ;; HEADERL 1 "T"

        ;; LDW     Y,#5
        ;; LDW     EVTIM_HR_BASE+2,Y
        ;; LDW     Y,#0xaae
        ;; LDW     EVTIM_HR_BASE+4,Y
        ;; LDW     Y,#0x3
        ;; LDW     EVTIM_HR_BASE+6,Y
        ;; LDW     Y,#0xacdc
        ;; LDW     EVTIM_HR_BASE+10,Y
        

        ;; CALLR   TRIGG
        ;; PUSHPS  OUTPUTS
        ;; CALL    DOT
        ;; PUSHPS  #TIMSEN
        ;; CALL    CAT
        ;; CALL    DOT
        ;; PUSHPS  #TIMERSBUF
        ;; PUSHPS  #5
        ;; JP      DUMP
        
        .endif                  ; HAS_TRIGG
        
;===============================================================
