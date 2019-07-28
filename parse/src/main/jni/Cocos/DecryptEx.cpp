//
// Created by Administrator on 2019/7/28.
//

typedef unsigned int _DWORD;

unsigned char *com_yoyo_dygame_GameApplication_decrypt(unsigned char *data, unsigned long len, unsigned long &out_len) {
    _DWORD * v5 = (_DWORD *)data;
    unsigned long v6 = len;
    unsigned long v7 = v6;
    if ( *(_DWORD *)v5 == 1902465708 )
    {
        v7 = v6 - 4;
        unsigned long v9 = (unsigned long)(v6 - 4) >> 2;
        if ( v9 >= 0xA )
            v9 = 10;
        if ( !v9 )
            goto LABEL_22;
        *(_DWORD *)(v5 + 4) = (*(_DWORD *)(v5 + 4) >> 16) & 0xAAAA | *(_DWORD *)(v5 + 4) & 0x5555 | (((*(_DWORD *)(v5 + 4) >> 16) & 0x5555 | *(_DWORD *)(v5 + 4) & 0xAAAA) << 16);
        if ( v9 == 1 )
            goto LABEL_22;
        *(_DWORD *)(v5 + 8) = (*(_DWORD *)(v5 + 8) >> 16) & 0xAAAA | *(_DWORD *)(v5 + 8) & 0x5555 | (((*(_DWORD *)(v5 + 8) >> 16) & 0x5555 | *(_DWORD *)(v5 + 8) & 0xAAAA) << 16);
        if ( v9 == 2 )
            goto LABEL_22;
        *(_DWORD *)(v5 + 12) = (*(_DWORD *)(v5 + 12) >> 16) & 0xAAAA | *(_DWORD *)(v5 + 12) & 0x5555 | (((*(_DWORD *)(v5 + 12) >> 16) & 0x5555 | *(_DWORD *)(v5 + 12) & 0xAAAA) << 16);
        if ( v9 != 3
             && (*(_DWORD *)(v5 + 16) = (*(_DWORD *)(v5 + 16) >> 16) & 0xAAAA | *(_DWORD *)(v5 + 16) & 0x5555 | (((*(_DWORD *)(v5 + 16) >> 16) & 0x5555 | *(_DWORD *)(v5 + 16) & 0xAAAA) << 16),
                v9 != 4)
             && (*(_DWORD *)(v5 + 20) = (*(_DWORD *)(v5 + 20) >> 16) & 0xAAAA | *(_DWORD *)(v5 + 20) & 0x5555 | (((*(_DWORD *)(v5 + 20) >> 16) & 0x5555 | *(_DWORD *)(v5 + 20) & 0xAAAA) << 16),
                v9 != 5)
             && (*(_DWORD *)(v5 + 24) = (*(_DWORD *)(v5 + 24) >> 16) & 0xAAAA | *(_DWORD *)(v5 + 24) & 0x5555 | (((*(_DWORD *)(v5 + 24) >> 16) & 0x5555 | *(_DWORD *)(v5 + 24) & 0xAAAA) << 16),
                v9 != 6)
             && (*(_DWORD *)(v5 + 28) = (*(_DWORD *)(v5 + 28) >> 16) & 0xAAAA | *(_DWORD *)(v5 + 28) & 0x5555 | (((*(_DWORD *)(v5 + 28) >> 16) & 0x5555 | *(_DWORD *)(v5 + 28) & 0xAAAA) << 16),
                v9 != 7)
             && (*(_DWORD *)(v5 + 32) = (*(_DWORD *)(v5 + 32) >> 16) & 0xAAAA | *(_DWORD *)(v5 + 32) & 0x5555 | (((*(_DWORD *)(v5 + 32) >> 16) & 0x5555 | *(_DWORD *)(v5 + 32) & 0xAAAA) << 16),
                v9 != 8)
             && (*(_DWORD *)(v5 + 36) = (*(_DWORD *)(v5 + 36) >> 16) & 0xAAAA | *(_DWORD *)(v5 + 36) & 0x5555 | (((*(_DWORD *)(v5 + 36) >> 16) & 0x5555 | *(_DWORD *)(v5 + 36) & 0xAAAA) << 16),
                v9 == 10) )
        {
            *(_DWORD *)(v5 + 40) = (*(_DWORD *)(v5 + 40) >> 16) & 0xAAAA | *(_DWORD *)(v5 + 40) & 0x5555 | (((*(_DWORD *)(v5 + 40) >> 16) & 0x5555 | *(_DWORD *)(v5 + 40) & 0xAAAA) << 16);
            v5 += 4;
        }
        else
        {
            LABEL_22:
            v5 += 4;
        }
    }

    out_len = v7;

    return (unsigned char *)v5;
}