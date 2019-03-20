\ "heardbeat" standby indicator

: ~ over hr! 1+ ;

hr# 68 -
6 exg 19 2* 2* 1 + + ~ \ delay 6, 19 steps, 1/25s
1500 ~ 9900 ~ 10 ~ \ rx, tx, delay pwm
10 ~ 22 ~ 50 ~ 100 ~ 180 ~
320 ~ 500 ~ 320 ~ 180 ~ 100 ~
180 ~ 320 ~ 500 ~ 320 ~ 180 ~
100 ~ 50 ~ 22 ~ 10 ~  \ "heartbeat" pattern
