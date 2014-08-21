#include <types.hh>

struct DWstruct {
    unint4 low, high;
};

typedef union {
    struct DWstruct s;
    int8 ll;
} DWunion;

extern "C" unint8 __udivmoddi4(unint8 num, unint8 den, unint8 *rem_p) {
    unint8 quot = 0, qbit = 1;

    if (den == 0)
    {
        //asm volatile("int $0"); /* Divide by zero */
        return 0; /* If trap returns... */
    }

    /* Left-justify denominator and count shift */
    while ((int8) den >= 0)
    {
        den <<= 1;
        qbit <<= 1;
    }

    while (qbit)
    {
        if (den <= num)
        {
            num -= den;
            quot += qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }

    if (rem_p)
        *rem_p = num;

    return quot;
}

/*!
 * \brief replacement for the 64bit integer divide funtion
 *
 * this function replaces the 64bit integer divide function supplied
 * with the glic. this makes the need of linking against glibc obsolute
 * which in turn saves some space in .text section of the resulting binary
 */
extern "C" unint8 __udivdi3(unint8 num, unint8 den) {
    return __udivmoddi4(num, den, 0);
}

extern "C" int8 __divdi3(int8 u, int8 v) {
    int8 c = 0;
    DWunion uu;
    uu.ll = u;
    DWunion vv;
    vv.ll = v;
    int8 w;

    if (uu.s.high < 0)
        c = ~c, uu.ll = -uu.ll;
    if (vv.s.high < 0)
        c = ~c, vv.ll = -vv.ll;

    w = __udivmoddi4(uu.ll, vv.ll, (unint8 *) 0);
    if (c)
        w = -w;

    return w;
}

extern "C" unint8 __moddi3(unint8 u, unint8 v) {
    int c = 0;
    DWunion uu;
    uu.ll = u;
    DWunion vv;
    vv.ll = v;
    int8 w;

    if (uu.s.high < 0)
        c = ~c, uu.ll = -uu.ll;
    if (vv.s.high < 0)
        vv.ll = -vv.ll;

    (void) __udivmoddi4(uu.ll, vv.ll, (unint8*) &w);
    if (c)
        w = -w;

    return w;
}
