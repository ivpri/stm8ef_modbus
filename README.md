# STM8 eForth (stm8ef) modbus extensions

[STM8 eForth](https://github.com/TG9541/stm8ef) is a lightweight self-hosted Forth system for STM8 µCs with a good "feature-to-binary-size" ratio. It provides a [binary release](https://github.com/TG9541/stm8ef/releases), easy configuration, a plug-in system for board support, a library infrastructure, and uCsim based binary level simulation. With the help of [e4thcom](https://wiki.forth-ev.de/doku.php/en:projects:e4thcom) it can be used as a semi-tethered embedded Forth.

## Board support:

This project provides extensions with modbus driver for following targets:

* [C0135MOD](https://github.com/TG9541/stm8ef/wiki/Board-C0135) "Relay-4 Board" - it can be used as a *Nano PLC*
* [STM8S105K4MOD](https://github.com/TG9541/stm8ef/tree/master/STM8S105K4) for STM8S Medium Density devices (Value Line / Access Line)


## Installation:

The [binary release](https://github.com/ivpri/stm8ef_modbus/hex) provides hex files with stm8ef extended by optimized extension words. Use [stm8flash](https://github.com/vdudouyt/stm8flash) utility to upload to target e.g.:

`stm8flash -c stlinkv2 -p 'stm8s103f3' -w hex/C0135MOD.ihx`

The supported microcontrollers contains just single hardware UART which is used by modbus communication.
The secont uart used as forth console is half duplex emulated on SWIM (PD1) pin. To connect to Rx/Tx of host uart use a diode this way:

```
STM8 device    .      .----o serial TxD "TTL"
               .      |      (e.g. "PL2303 USB serial converter")
               .     ---
               .     / \  1N4148
               .     ---
               .      |
STM8 PD1/SWIM-->>-----*----o serial RxD "TTL"
               .
GND------------>>----------o serial GND
               .
................
```

Use [e4thcom](https://wiki.forth-ev.de/doku.php/en:projects:e4thcom) to connect e.g.:

`e4thcom -t stm8ef -d ttyUSB0`

Pres Enter and you should see `ok` prompt.

The binary distribution provides just forth extensions with some optimized low level words. To have full featured modbus device type in the forth console:

`#require hw/modbus/main.fs`

The file above might be edited before to change default configuration.
You should see the led of 'C0135' board to blinky indicating everything works fine.


## Usage

If the installation went well the board can be controlled both by forth console and modbus interface.

Here is an example how to send modbus commands using [mbpool](https://github.com/epsilonrt/mbpoll):

```
# (Note: Coil address 1 is mapped to address 0000 in frame etc. as done by mbpoll)
# (Note: mbpoll switch -0 to use PDU addressing starting with 0)
# (Note: -t # for input and holding regs is swapped with func codes, func 3 is for holding regs)
mbpoll -a 2 -b 9600 -P none -t 0 -r 1 -c 5 /dev/ttyUSB0     # read coils state
mbpoll -a 2 -b 9600 -P none -t 0 -0 -r 0 -c 5 /dev/ttyUSB0  # the same like above
mbpoll -a 2 -b 9600 -P none -t 1 -r 1 -c 5 /dev/ttyUSB0     # read discrete inputs
mbpoll -a 2 -b 9600 -P none -t 3 -r 1 -c 2 /dev/ttyUSB0     # read input regs
mbpoll -a 2 -b 9600 -P none -t 4:hex -r 1 -c 4 /dev/ttyUSB0 # read holding regs
mbpoll -a 2 -b 9600 -P none -t 0 -r 1 /dev/ttyUSB0 1        # write coil
mbpoll -a 2 -b 9600 -P none -t 4 -r 1 /dev/ttyUSB0 111      # write holding reg (change address to 111)
mbpoll -a 2 -b 9600 -P none -t 0 -r 3 /dev/ttyUSB0 1 1 1    # write multiple coils
mbpoll -a 2 -b 9600 -P none -t 4 -r 3 /dev/ttyUSB0 0x0110 0xabab 0xdede  # write multiple holding regs
```


### Modbus coils

Use coils 1-4 to directly control board relays. To preserve coil settings after power down set coil 8.

```
Coil Address  Pin 
1             12  PB4 Relay1 (0:on - inverted)
2             13  PC3 Relay2 (0:on - inverted)
3             14  PC4 Relay3 (0:on - inverted)
4             15  PC5 Relay4 (0:on - inverted)
5             1   PD4	LED  (1:on)
6             1   PD4   1 to configure TIM2_CH1 output with PWM
7             -   -     1 to reset statistic input register on coil write and each register read
8             -   -     flag - 1 to store coil settings in EEPROM (HR 3) after each change
9             20  PD3   1 to set as PWM / 0 input with pullup
10            19  PD2   1 to set as PWM / 0 input with pullup
11            17  PC7   1 to set as PWM / 0 input with pullup
12            16  PC6   1 to set as PWM / 0 input with pullup

Custom coils flags, can be accessed by user program with OUT @
13 - 16

```

To read and control coils from forth use `OUT` and `OUT!` words:

```
\ OUT  ( -- addr )
\ OUT! ( x -- )
\ read coils:
2 base ! out @
\ turn relays 2 and 4 on, other coils remains untouched:
out @ 0101 or out!
```



### Holding registers

```
HR Address    Content
1             Station address 1-255
2             Baud rate idx 0-7: 1200 2400 4800 9600 19200 38400 57600 115200
3             Coils 1-16 initial state after boot (OUT!)

Holding registers for event handling:

Triggers:
4-8   - Event handling for button presses  (IN1-IN4, button S2)
9-13  - Event handling for button releases (IN1-IN4, button S2)
14-17 - Event handling for timers 1-4

Trigger configuration bits:
TC4 TC3 TC2 TC1  TS4 TS3 TS2 TS1    OR4 OR3 OR2 OR1  OS4 OS3 OS2 OS1
OSi ORi: output i - 00 no action  10 set  01 reset  11 toggle
TCi TSi: timer  i - 00 no action  10 continue  01 start from init value  11 stop

Timers:
18-33 - Initial values for timers (in 1/100ths of second, doubles, most significant word first.
        Timing range: 0.01s - 11930h / 497 days

Holding registers to configure timer registers for PWM:

TIM2 is used for BG task (5ms) so it is available for 200Hz PWM (reload value 9950).
TIM1 is fully available for any PWM rate.
PWM duty set by write to holding regs starting on hree# offset:

34  PD4 LED      TIM2_CH1
35  PD3 IN1      TIM2_CH2
36  PD2 IN2      TIM2_CH3 (only when AFR1 option bit is set)
37  PC7 IN3      TIM1_CH2
38  PC6 IN4      TIM1_CH1

Configuration for TIM1:

39  Timer counter reload value (TIM1_ARRH,L)
40  Timer prescaler value (TIM1_PSCRH,L) (0 = f/1 etc., f=8MHz)
```

Read and write holding registers from forth using `HR@` and `HR!` words.
PDU addressing is used here so station address is on address 0:

```
\ read current station address
0 hr@
\ change modbus RTU baudrate to 38400 (index 5, address (PDU) 1):
5 1 hr!

```

#### Events Programming

The extension contains simple declarative timers and events programmable subsystem.

There are 12 event sources: 4 button presses, 4 button releases and 4 timers.
Each event source has dedicated holding register which contains triggers configuration.
See *Trigger configuration bits* above for details.

See also *Timers* section above how to setup four 32 bit timers.

Examples

Open forth console co write to holding regs.
PDU addressing is used here so holding register addresses is 1 less then described above.

Configure inputs so a *button press* on the input toggles corresponding relay state.
All relays are switched off 10 seconds after last *button press*.
Onboard button S2 press will switch on all relays which will switch off after 2 seconds.

```
decimal
\ for IN1-IN4: start timer 1, stop timer 2
%0010001100010001 3 hr! \ toggle output 1
%0010001100100010 4 hr! \ toggle output 2
%0010001101000100 5 hr! \ toggle output 3
%0010001110001000 6 hr! \ toggle output 4
\ S2 press: start timer 1, stop timer 2, all relays on
%0001001100001111 7 hr!
%0000000011110000 13 hr! \ Timer 1: all relays off
%0000000011110000 14 hr! \ Timer 2: all relays off
0 15 hr! 0 16 hr !       \ No action from timers 3, 4
0 17 hr! 100 10 * 18 hr! \ Timer 1: 10s
0 19 hr! 100 2 * 20 hr! \ Timer 2: 2s

```


#### PWM configuration

Holding registers 34-40 can be used to setup microcontroller hardware timers to generate pwm. See also coils configuration how to switch IN1-IN4 to pwm mode. For some pwm outputs also an AFR bit might need to be configured to switch between ADC and PWM peripherial.


### Discrete inputs

```
DI Address    Pin (with pull up settings)
1             PD3	IN1 (AIN4, TIM2_CH2)
2             PD2	IN2 (AIN3, TIM2_CH3)
3             PC7	IN3 (TIM1_CH2 / SPI_MISO)
4             PC6	IN4 (TIM1_CH1 / SPI_MOSI)
5             PA3	Key "S2" (0:pressed) (TIM2_CH3 / SPI_NSS) (inverted)
```

### Input registers

Input registers are used to read actual ADC value - by default two input channels on IN1 and IN2 are available:

```
1             PD3	IN1 AIN4 ADC last read value (sampled each 5ms)
2             PD2	IN2 AIN3 ADC last read value (sampled each 5ms)
3             -         AIN4 min value since last read of this register
4             -         AIN4 max value since last read of this register
5             -         AIN4 avg value since last read of this register (65k samples 5 min max)
6             -         AIN4 samples since last read of this register
7             -         AIN3 min value since last read of this register
8             -         AIN3 max value since last read of this register
9             -         AIN3 avg value since last read of this register (65k samples 5 min max)
10            -         AIN3 samples since last read of this register
```


# Disclaimer, copyright

This is a hobby project! Don't use the code if support, correctness, or dependability are required. Additional licenses might apply to the code which may require derived work to be made public!

Please refer to stm8ef LICENSE.md for details.

[WG1]: https://github.com/TG9541/stm8ef/wiki/STM8S-Value-Line-Gadgets
