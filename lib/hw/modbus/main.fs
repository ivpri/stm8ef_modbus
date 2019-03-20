ram decimal

\ Configuration:

\ See also `Modbus configuration' on the bottom of this file
\ USARTSR register - select one for STM8S103 or STM8S105
$5230 constant uartsr \ 103 - USART1
\ $5240 constant uartsr \ 105 - USART2

\ Optional activity/standby led indicator
\ When enabled also uncomment the needed reset definition market as *MBIND* below
#require ../mbind.fs

\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

\ Define count of resources needed by mbfn.fs:
16 constant co# \ coils (12 build-in, 4 custom)
5  constant di# \ discrete inputs
\ ir# and hr# etc. are set by globconf

#require ]B!
#require ../mbrtu.fs \ TODO: mbfn.fs
#require ../]C!

:nvm uartsr c@ $10 and
;ram alias idle?

:nvm savec idle?
     if   [ 0 uartsr 5 + 4 ]b! \ ILIEN clear
          [ 0 uartsr 5 + 5 ]b! \ RIEN  clear         
     then uartsr c@ $20 and    \ RXNE?     
     if   [ uartsr 1+ ] literal
          c@ fbc!
     then iret
;ram dup nvm
$8052 ! \ INT_UART1RX
$805C ! \ INT_UART2RX (s105) - no need to comment one of these, just write to both locations

:nvm savec mbtx iret
;ram dup nvm
$804E ! \ INT_UART1TX
$805A ! \ INT_UART2TX (s105)

:nvm idle?
     if   [ uartsr 1+ ] literal
          c@ drop mbii
          [ uartsr 5 + ]
          literal $80 and 0= \ TIEN not set?
          if   [ 1 uartsr 5 + 4 ]b! \ ILIEN set
               [ 1 uartsr 5 + 5 ]b! \ RIEN  set
          then
     then evu
;ram constant b

\ nvm b alias b ram
     
:nvm ini b bg      !
     [ 0 errc    ]C!
     [ 0 errc 1+ ]C!
;ram nvm 'boot     !

\ : reset  rst reset ;
: reset rst nvm 68 hr#+ reset ; \ *MBIND* use this when mbind.fs in use

$A000 here - . ram

\ Modbus configuration
2 0 hr! \ default address: 2
3 1 hr! \ default baudrate: 9600

cold
#require PERSIST
persist
cold
