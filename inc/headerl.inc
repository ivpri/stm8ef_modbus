        ;; Just like HEADER in forth.asm but without UNLINK_'Label and wiithout size computed
        .macro  HEADERL nSize wName
        .dw     LINK
        LINK    = .
        .db     nSize
        .ascii  wName
        .endm
