        ;; .globl _UART1_RX_IRQHandler
        ;; .globl _UART1_TX_IRQHandler
        ;; .globl _UART2_RX_IRQHandler
        ;; .globl _UART2_TX_IRQHandler


;      Dummy labels for PSIM interrupts declared in main.c
;      These are for unused interrupt handlers

        .ifne   PSIM-PORTA
;       Dummy label for _EXTIA_IRQHandler
_EXTI0_IRQHandler:
        .endif

        .ifne   PSIM-PORTB
;       Dummy label for _EXTIB_IRQHandler
_EXTI1_IRQHandler:
        .endif

        .ifne   PSIM-PORTC
;       Dummy label for _EXTIC_IRQHandler
_EXTI2_IRQHandler:
        .endif

        .ifne   PSIM-PORTD
;       Dummy label for _EXTID_IRQHandler
_EXTI3_IRQHandler:
        .endif

        .ifne   PSIM-PORTE
;       Dummy label for _EXTIE_IRQHandler
_EXTI4_IRQHandler:
        .endif

;;         .ifeq   HAS_RXTXIRQ
;; ;       Dummy labels for _UART[12]_[RT]X_IRQHandler
;; _UART1_RX_IRQHandler:
;; _UART1_TX_IRQHandler:
;; _UART2_RX_IRQHandler:
;; _UART2_TX_IRQHandler:
;;         .endif
        
