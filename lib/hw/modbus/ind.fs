\ Modbus Activity/standby led indicator using PWM pattern

\ Supported PWM values are 0-9950
\ Extends hr# by 68 holding registers:
\ - 0-255 (255-n max) delay time, n = 1-64 pattern values (0 = 64), bits 0,1 prescaller:
\         00 /4 (50Hz 0.02s)  01 /8 (25Hz 0.04s)  10 /16 (12.5Hz 0.08s)  11 /32 (6.25Hz 0.16s)
\         note: delay time + n must not exceed 255
\         set to -1 (0xFFFF) to disable standby indicator
\ - rx activity pwm value, -1 (0xFFFF) to disable rx indicator
\ - tx activity pwm value, -1 (0xFFFF) to disable tx indicator
\ - pwm value for delay time between patterns
\ - up to 64 pwm values for standby pattern

\ Also extend reset definition in caller with 68 hr#+ e.g.:
\ : reset  rst nvm 68 hr#+ reset ; \ uncomment when mbind.fs in use

ram

\ Configuration:
\ LED pwm on TIM2 CH1
$5311 constant pwmreg


#require [COMPILE]
#require :NVM
#require ALIAS
#require WIPE

: ]- \ DEC c-addr
    $ff over <
    if   $725A , ,
    else $3A C, C,
    then ]
; IMMEDIATE

\ : ]; \ tail call optimization JP addr
\     $cc c, , overt
\ ; IMMEDIATE

\ Optimize code size by accessing holding regs
\ content directly
$4000 hr# 2* + constant hr0

: ~ over hr! 1+ ;
hr# 6 exg 19 2* 2* 1 + + ~ \ delay 6, 19 steps, 1/25s
9900 ~ 2000 ~ 30 ~ \ rx, tx, delay pwm
60 ~ 125 ~ 250 ~ 500 ~ 1000 ~
2000 ~ 4000 ~ 2500 ~ 1500 ~ 1000 ~
1500 ~ 2500 ~ 4000 ~ 2000 ~ 1000 ~
500 ~ 250 ~ 125 ~ 60 ~  \ "heartbeat" pattern

here nvm 1 allot ram constant c

:nvm
    hr0 @ 2 rshift 1- $3f and 1+
;ram alias n
    
\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

nvm

: pwml pwmreg 2c! ;

: evu
    evu hr0 @ -1 = \ disabled standby indicator?
    if exit then
    1 [ hr0 1+ ] literal
    c@ 3 and 2+ lshift 1-
    tim and 0=
    if   c c@ dup dup 0= 
         if   n hr0 c@ + c c!
         then [ c ]- n <
         if   [ hr# 4 + ] literal + hr@
         else [ hr0 6 + ] literal @
         then pwml
    then ;

: fbc!
    [ hr0 2+  ] literal @ dup 0<
    if drop else pwml then fbc! ;
    
: mbtx
    [ hr0 4 + ] literal @ dup 0<
    if drop else pwml then mbtx ;

here u. $A000 here - u.
68 hr#+

ram
 
\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

\ Enable indicator by setting LED pwm coil 6
\ in the coils init holding register
2 hr@ $20 or 2 hr!
wipe

