; STM8EF Global Configuration File
; Config for C0135 "Relay Board-4 STM8S"

        HALF_DUPLEX      = 0    ; Use UART in half duplex mode
        HAS_TXUART       = 1    ; No UART TXD, word TX!
        HAS_RXUART       = 1    ; No UART RXD, word ?RX
        ; Emulated second uart (150B)
        HAS_TXSIM        = 2    ; Enable TxD via GPIO/TIM4, word TXGP!
        HAS_RXSIM        = 2    ; Enable RxD via GPIO/TIM4, word ?RXGP
        PSIM     = PORTD        ; Port for UART simulation
        PNRX             = 1    ; Port GPIO# for HAS_RXDSIM
        PNTX             = 1    ; Port GPIO# for HAS_TXDSIM
        HAS_ADC          = 1    ; Enable ADC@ ADC! (51B)

        USE_UART2        = 1    ; Set to 1 to use UART2 instead of UART1 (e.g. on stm8s105)

        EMIT_BG  = DROP         ; 7S-LED background EMIT vector
        QKEY_BG  = QKEYB        ; ?KEYB for background ?KEY

        HAS_LED7SEG      = 0    ; no 7S-Display
        HAS_KEYS         = 1    ; yes, 1 key   (cannot disable: relocation error)
        HAS_OUTPUTS      = 5    ; yes, 4 relays, 1 LED (56B)  OUT! OUT (return address of outputs reg)      

        HAS_BACKGROUND   = 1    ; Background Forth task (TIM2 ticker)
        HAS_CPNVM        = 1    ; Can compile to Flash, always interpret to RAM
        HAS_DOES         = 0    ; CREATE-DOES> extension  (82B)
        HAS_DOLOOP       = 0    ; DO .. LOOP extension: DO LEAVE LOOP +LOOP (128B)

        CASEINSENSITIVE  = 1    ; Case insensitive dictionary search
        SPEEDOVERSIZE    = 0    ; Speed-over-size in core words: ROT - = <
        BAREBONES        = 1    ; (1 - save 213B) Remove or unlink some more: hi HERE .R U.R SPACES @EXECUTE AHEAD CALL, EXIT COMPILE [COMPILE] ! also remove RX! etc.

        WORDS_LINKINTER  = 0    ; Link interpreter words: ACCEPT QUERY TAP kTAP hi 'BOOT tmp >IN 'TIB #TIB eval CONTEXT pars PARSE NUMBER? DIGIT? WORD TOKEN NAME> SAME? find ABORT aborq $INTERPRET INTER? .OK ?STACK EVAL PRESET QUIT $COMPILE
        WORDS_LINKCOMP   = 0    ; Link compiler words: cp last OVERT $"| ."| $,n
        WORDS_LINKRUNTI  = 0    ; Link runtime words: doLit do$ doVAR donxt dodoes ?branch branch
        WORDS_LINKCHAR   = 0    ; Link char out words: DIGIT <# # #S SIGN #> str hld HOLD  (38B)
        WORDS_LINKMISC   = 0    ; Link composing words of SEE DUMP WORDS: >CHAR _TYPE dm+ .ID >NAME

        WORDS_EXTRASTACK = 0    ; Link/include stack core words: rp@ rp! sp! sp@ DEPTH
        WORDS_EXTRADEBUG = 0    ; Extra debug words: SEE
        WORDS_EXTRACORE  = 1    ; Extra core words: =0 I (12B)
        WORDS_EXTRAMEM   = 0    ; Extra memory words: B! 2C@ 2C! (71B) (2C@ 2C! required to access timers regs)
        WORDS_EXTRAEEPR  = 1    ; Extra EEPROM lock/unlock words: LOCK ULOCK ULOCKF LOCKF  (34B)

        ; Board specific words
        HAS_CRC16        = 0    ; CRC16  ( x1 c -- x2 ) for MODBUS (35B)
        HAS_INPUTS       = 1    ; INP@   ( -- c ) read inputs (4 bits) + button (inverted) (46B)
        HAS_2CSTORE      = 1    ; 2C!    ( x a -- )     (19B)
        HAS_RSHIFT       = 1    ; RSHIFT ( x1 n -- x2 ) (26B)
        HAS_LSHIFT       = 1    ; LSHIFT ( x1 n -- x2 ) (26B)
        HAS_INI          = 1    ; INI    ( -- ) init subsystems on boot time
        HAS_MODBUS_FB    = 1    ; Modbus frame buffer and utils: fb fb# fb! ... (167B)

        HAS_MBPHY        = 1    ; BR# MBI MBTX - modbus physical layer - init modbus, transmit byte
        MBPHY_DERE       = PORTB  ; which port is used for DE/RE control of half duplex modbus interface
        MBPHY_DERE_PIN   = 5    ; DE/RE pin
        MBPHY_DERE_INV   = 0    ; invert  DE/RE? (RE/DE - RX on active output)

        HAS_MBCRC        = 1    ; MBCRC ( n -- x ) (51B)

        ; Setup input registers for ADC readings
        ; 0 ir@ will read first ADC channel (INPUT_REG0_ADCCH) etc.
        HAS_INPUT_REGS_ADC = 2  ; Read ADC data with IR@ IR#, number of channels 0 - 4  (82-85B)
        HAS_INPUT_REGS_STA = 1  ; See ADCIRSTA (+189B for 2 channels, 10B for each additional channel)
        HAS_INPUT_REGS_TIM = 1  ; Access event timer (HAS_EVTIMS) counters with IR@ (after ADC regs) (13B)
        INPUT_REG0_ADCCH = 4    ; ADC channel for input register 0 ir@
        INPUT_REG1_ADCCH = 3    ; ADC channel for input register 1 ir@
        INPUT_REG2_ADCCH = 5    ; ADC channel for input register 2 ir@
        INPUT_REG3_ADCCH = 6    ; ADC channel for input register 3 ir@

        ; Setup holding register words HR# HR@ HR! HRI (130B)
        HAS_HOLDING_REGS  = 33  ; Count of base holding registers
        HOLDING_REGS_BASE = EEPROMBASE ; first holding register address
        COILS_HOLDING_REG = 3   ; holding reg id (>=1) to initialize coils from, 0 to disable
        TIM_HOLDING_REGS  = 7   ; Additional holding registers to initialize timer(s) regs
                                ; (starting with HAS_HOLDING_REGS)
        TIM_HOLDING_REG0 = TIM2_CCR1H  ; PD4  LED (TIM2_CH1)
        TIM_HOLDING_REG1 = TIM2_CCR2H  ; PD3  IN1 (TIM2_CH2)
        TIM_HOLDING_REG2 = TIM2_CCR3H  ; PD2  IN2 (TIM2_CH3) (when AFR1 option bit is set)
        TIM_HOLDING_REG3 = TIM1_CCR2H  ; PC7  IN3 (TIM1_CH2) (when AFR0 option bit is set)
        TIM_HOLDING_REG4 = TIM1_CCR1H  ; PC6  IN4 (TIM1_CH1) (when AFR0 option bit is set)
        TIM_HOLDING_REG5 = TIM1_ARRH   ; TIM1 auto reload register
        TIM_HOLDING_REG6 = TIM1_PSCRH  ; TIM1 prescaler
        

        HAS_EVTIMS       = 4           ; Has how many 32bit event timers (0 - 4)
        EVTIM_FIRST_HR   = 17          ; First holding register of 2 * HAS_EVTIMS (HAS_EVTIMS doubles)
                                       ; with initial timers values (PDU addressing >= 0)
        HAS_EVUPD        = 1           ; EVUPD (106B + 1+HAS_EVTIMS*2B RAM for timers)
        HAS_TIMCNT       = 1           ; TIMCNT - access timers counters (24B)
        HAS_TRIGG        = 1           ; TRIGG  - trigger outputs and timers (102B)
        HAS_EVU          = 1           ; EVU    - EVUPD + TRIGG for all events   TODO: tests
        HAS_CUST_TRIG    = 1           ; 'TRIG RST and extend EVU
        HAS_CUST_HR      = 1           ; HR#+ RST
        HAS_CUST_IR      = 1           ; IR#+ 'IR@ RST and extend IR@

        .ifne HAS_MODBUS_FB
        MODBUS_FBSIZE    = 41   ; Modbus frame buffer size in bytes (for 16 reg vals max)
;        MODBUS_FBSIZE   = 136  ; Modbus frame buffer size in bytes (max size by protocol)
        .else
        HAS_MBCRC        = 0
        MODBUS_FBSIZE    = 0    ; Used in target.inc to raise UPPLOC
        .endif

        INPUT_REGS_BUFSIZE = HAS_INPUT_REGS_ADC * (2 + 12 * HAS_INPUT_REGS_STA)

        UPPLOC_RAISE = MODBUS_FBSIZE + INPUT_REGS_BUFSIZE + (HAS_EVUPD * 17)
