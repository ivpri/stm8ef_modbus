\ Modbus functions 1-6,15,16 implementation
\
\ Features:
\ - Fuctions 1 - 6, 15, 16 for up to 16 coils and 16 discrete inputs
\ - Configurable station address (holding register 1)
\ - Configurable baud rate (holding register 2 - rate index)
\
\ Prerequisities:
\ Modbus frame buffer utilities (fb fbc! fb# ...)
\ Modbus physical layer words: MBI MBTX
\ br# - available baud rates for holding register 2
\ co# di# ir# hr# ( -- n ) - count of coils, discrete inputs,
\                            input registers, holding registers
\ out! ( x -- )  out ( -- addr ) - set coils, coils out buffer address
\ inp@ ( -- u )   - read discrete inputs
\ ir@  ( n -- x ) - read input register
\ hr@  ( n -- x ) - read holding register
\ hr!  ( x n -- ) - write holding register

ram decimal

#require [COMPILE]
#require :NVM
#require ALIAS
#require ]B!
#require ]C!

nvm 

#require LSHIFT
#require RSHIFT

variable errc  \ crc error counter for frames with our station address

ram

fb    constant fb#
fb 2+ constant fb

:nvm fb# c@
;ram alias fb#@

:nvm [ 2 fb# ]c! [ 1 fb 1+ 7 ]b! fbc! \ set MS bit of func
;ram alias mbexcp ( n -- )

\ check the start address is less then n
\ and count is allowed. Set address (2)
\ exception frame and return true if not.
:nvm [ fb 2+  ] literal \ resaddr
     @ over u< >r
     [ fb 4 + ] literal \ count/val
     @ dup 0= 0= >r
     [ fb 2+  ] literal
     @ + u< 0= r> and r> and
     if   0
     else 2 mbexcp -1 \ illegal address
     then
;ram alias adrexcp? ( n -- flag )


\ 01 Read Coils
\ 02 Read Discrete Inputs
\ Coil 5 is LED / DI 5 is "S2" button
:nvm [ fb 1+ ] literal c@ 1 =  \ func
     if   out @ co#
     else inp@  di# 
     then adrexcp?
     if   drop exit            \ illegal address exception
     then fb@ rshift >r        \ shift by first address
     fb@    1 over lshift 1-   \ make and apply mask
     r> and [ 2 fb# ]c! swap 9 <    \ up to 16 coils/inputs
     if   1 fbc!               \ <= 8 coils/inputs
     else 2 fbc! dup fbc!
          8 rshift             \ 8 - 16 (2 bytes)
     then fbc! ; 
;ram constant rd-c/di


\ 03 Read Holding Registers
\ 04 Read Input Registers
\ Address 1 for adc0 and address 2 for adc1
:nvm [ fb 1+ ] literal c@ 4 =  \ func
     if   [ ' ir@ ] literal ir#
     else [ ' hr@ ] literal hr#
     then adrexcp?
     if   drop exit            \ illegal address exception
     then fb@ fb@ [ 2 fb# ]c!  \ handler first-addr count
     dup 2* fbc! 1-            \ byte count
     for  2dup swap execute fb! 1+
     next
;ram constant rd-ir/hr

     
\ 05 Write Single Coil
:nvm fb@ dup co# u<  \ Coil address
     if   out @ 1 rot lshift
          fb@ ?dup   \ Value
          if   $FF00 =
               if   or
               else 3 mbexcp \ illegal value
               then
          else not and
          then out!
     else drop 2 mbexcp \ illegal address
     then \ response is the same like request
;ram constant wr-co

     
\ Check address and baud rate idx is within allowed range
:nvm dup 0=
     if   over 1- $FF u< \ check adress is 1 - 255
     else dup 1 =
          if   over br# u< \ check baud rate idx
          else -1 then
     then
     if   hr!
     else 2drop 3 mbexcp \ illegal value
     then
;ram alias hr2!


\ 06 Write Holding Register
:nvm fb@ dup hr# u<     \ Register address
     if   fb@ swap hr2! \ Store value
     else drop 2 mbexcp \ illegal address
     then \ response is the same like request
;ram constant wr-hr
     

\ 15 Write Multiple Coils
:nvm co# adrexcp?
     if   drop exit
     then fb@ >r          \ first address 
     fb@ 1 swap lshift 1- \ prepare bit mask   mask R: adr
     r@ lshift dup not    \ positive and negative mask
     out @ and swap       \ apply mask to coils, zeroes req coils
     fbc@ 2 = fbc@ swap
     if   fbc@ 8 lshift or
     then r> lshift and
     or out! [ 6 fb# ]c!
;ram constant wr-coils


\ 16 Write Multiple Holding Registers
:nvm hr# adrexcp?
     if   drop exit
     then fb@ fb@ fbc@ over 2* = 
     if   1-
          for  fb@ over hr2! 1+
          next drop [ 6 fb# ]c!
     else 2drop 3 mbexcp \ illegal value
     then
;ram constant wr-hrs


\ Function decode: supported functions:
nvm
here rd-c/di  , \ 01 Read Coils
     rd-c/di  , \ 02 Read Discrete Inputs
     rd-ir/hr , \ 03 Read Input Registers
     rd-ir/hr , \ 04 Read Holding Registers
     wr-co ,    \ 05 Write Single Coil
     wr-hr ,    \ 06 Write Holding Register
     wr-coils , \ 15 Write Coils
     wr-hrs ,   \ 16 Write Holding Registers
ram constant mbfunc

     
:nvm fbc@ 1- dup 1 or 15 =
     if   8 - \ frame size:
          [ fb 6 + ] literal c@ 7 +
          over 8
     else 6 over 6 then u<  \ frame size 6
     if   fb#@ =
          if   2* mbfunc + @ execute
          else 3 mbexcp \ bad value (wrong frame size) / illegal value
          then 
     else 1 mbexcp then \ illegal function
;ram alias pf    
     
\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Idle irq handler
\ Use in RX handler by jumping to the mbii
\ instead of call e.g.: [ $CC c, mbii , ]
\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
:nvm 3 fb#@ < \ frame at least addr func crc
     if   0 hr@ fbc@ = \ station address check
          if   fb#@ 2- mbcrc fb@ =
               if   fb#@ 2- fb# c!
                    [ 1 fb# 1+ ]c! 
                    pf fb#@ mbcrc fb!
                    [ 0 fb# 1+ ]c!
                    mbtx exit
               else 1 errc +! then
          then   
     then [ 0 fb# ]c! [ 0 fb# 1+ ]c!
;ram alias mbii
\ \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
