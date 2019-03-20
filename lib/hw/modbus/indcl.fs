\ "classic" standby indicator

: ~ over hr! 1+ ;

hr# 68 -
28 exg 22 2* 2* 2 + + ~ \ delay 28, 22 steps, 0.08s tick
1500 ~ 9900 ~ 0 ~ \ rx, tx, delay pwm
10 ~ 15 ~ 22 ~ 33 ~ 47 ~ 75 ~ 100 ~
150 ~ 220 ~ 330 ~ 470 ~ 470 ~ 330 ~
220 ~ 150 ~ 100 ~ 75 ~ 47 ~ 33 ~
22 ~ 15 ~ 10 ~
