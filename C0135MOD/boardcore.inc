; C0135 "Nano PLC with modbus" STM8S device dependent routines

        .include "dummy_irq.inc"
        .include "headerl.inc"
        
;===============================================================


;       Init board GPIO (except COM ports)
        
BOARDINIT:
        ; "Nano PLC Relay board" GPIO for relay & LED
        MOV     PB_DDR,#0x10    ; PB4 Relay1
        MOV     PC_DDR,#0x38    ; PC3-5 Relay2-4
        MOV     PD_DDR,#0x10    ; PD4 LED
        MOV     PD_CR1,#0x1C    ; PD4 LED Push-Pull Output, PD2,3 (IN2,1) pull-up inputs
        MOV     PA_CR1,#0x08    ; PA3  (key S2) pull-up input
        MOV     PC_CR1,#0xC0    ; PC6,7 (IN4,3) pull-up inputs

        BSET    MBPHY_DERE+DDR,#MBPHY_DERE_PIN ; RS485 DE/RE pin set to outpit

        ; Set PWM mode 1 on pwm outputs
        LD      A, #0x68        ; PWM mode 1, OCxPE set
        LD      TIM1_CCMR1,A
        LD      TIM1_CCMR2,A
        LD      TIM2_CCMR1,A
        LD      TIM2_CCMR2,A
        LD      TIM2_CCMR3,A
        MOV     TIM1_CR1,#0x01  ; CEN - TIM1 counter enabled

        ;; Note: stack not initialized yet here, cannot call forth
       
        RET



;===============================================================
        
        
        .ifne   HAS_OUTPUTS
;       OUT!  ( c -- )
;       Put c to board outputs, storing a copy in OUTPUTS
       
        HEADERL 4 "OUT!"
OUTSTOR:
        LD      A,(X)
        LD      OUTPUTS,A

        RRC     A
        BCCM    TIM2_CCER1,#4   ; "IN1" PWM enable (CC2E)       
        RRC     A
        BCCM    TIM2_CCER2,#0   ; "IN2" PWM enable (CC3E)
        RRC     A
        BCCM    TIM1_CCER1,#4   ; "IN3" PWM enable (CC2E)
        RRC     A
        BCCM    TIM1_CCER1,#0   ; "IN4" PWM enable (CC1E)        
        
        INCW    X      
        LD      A,(X)

        .ifne   HAS_INPUT_REGS_ADC
        .ifne   HAS_INPUT_REGS_STA
        ;; reset ADC statistic register on 0 -> 1 change of bit 6
        BCP     A,#0x40
        JREQ    2$              ; new is 1?
        BTJT    OUTPUTS+1,#6,2$ ; old is 0?

        ;; 0 -> 1 bit change detected, reset all statistics input regs
        CALL    MBIRSTARST_ALL
2$:     
        .endif
        .endif
        
        LD      OUTPUTS+1,A
        INCW    X
        XOR     A,#0x0F         ; C0135 Relay-4 Board LED not inverted
        RRC     A
        BCCM    PB_ODR,#4       ; Relay1
        RRC     A
        BCCM    PC_ODR,#3       ; Relay2
        RRC     A
        BCCM    PC_ODR,#4       ; Relay3
        RRC     A
        BCCM    PC_ODR,#5       ; Relay4
        RRC     A
        BCCM    PD_ODR,#4       ; LED
        RRC     A        
        BCCM    TIM2_CCER1,#0   ; LED PWM enable (CC1E)       
        
        RET
        .endif


;===============================================================

        .ifne   HAS_INPUTS
;       INP@  ( -- c )
;       Read board discrete inputs (active 0) as bits of c

        HEADERL 4 "INP@"
INPFETCH:
        CLR     A        
        BTJT    PD_IDR,#3,1$    ; IN1
        OR      A,#0x01
1$:     
        BTJT    PD_IDR,#2,2$    ; IN2
        OR      A,#0x02
2$:     
        BTJT    PC_IDR,#7,3$    ; IN3
        OR      A,#0x04
3$:     
        BTJT    PC_IDR,#6,4$    ; IN4
        OR      A,#0x08
4$:     
        BTJT    PA_IDR,#3,5$    ; Button "S2"
        OR      A,#0x10
5$:     
        JP      ASTOR
        .endif

        
;===============================================================

; Workaround for rellocation failed if HAS_KEYS = 0 to save ~50B
        .ifeq   HAS_KEYS
;        .ifne   HAS_KEYS

;       BKEY  ( -- c )     ( TOS STM8: -- A,Z,N )
;       Read board key state as a bitfield.

        HEADERL 3 "BKEY"
BKEY:
        ; Key "S2" port PA3 (inverted)
        LD      A,PA_IDR
        SLA     A
        SWAP    A
        CPL     A
        AND     A,#0x01
        JP      ASTOR


;       BKEYC  (  -- c )   ( TOS STM8: -- A,Z,N )
;       Read and translate board dependent key bitmap into char

BKEYCHAR:
        CALLR   BKEY
        JREQ    1$
        ADD     A,#'@'
        LD      (1,X),A
1$:     RET
       .else
BKEYCHAR: RET
       .endif


;===============================================================


        .include "modbus.inc"
