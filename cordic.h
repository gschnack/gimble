

extern int16_t cordic6_10(fix6_10_t x,fix6_10_t y);


typedef int32_t fix16_t;


static const fix16_t fix16_pi  = 205887;     /*!< fix16_t value of pi */
static const fix16_t fix16_e   = 178145;     /*!< fix16_t value of e */
static const fix16_t fix16_one = 0x00010000; /*!< fix16_t value of 1 */

/* Conversion functions between fix16_t and float/integer.
 * These are inlined to allow compiler to optimize away constant numbers
 */

static inline fix16_t fix16_from_int(int a)     { return a * fix16_one; }
static inline fix16_t fix16_from_lower(int a)     { return (fix16_t ) a; }


static inline float   fix16_to_float(fix16_t a) { return (float)a / fix16_one; }
static inline double  fix16_to_dbl(fix16_t a)   { return (double)a / fix16_one; }

static inline int fix16_to_int(fix16_t a)
{
    return (a >> 16);
}

static inline fix16_t fix16_from_float(float a)
{
        float temp = a * fix16_one;
        return (fix16_t)temp;
}






/* 32-bit implementation of fix16_mul. Potentially fast on 16-bit processors,
 * and this is a relatively good compromise for compilers that do not support
 * uint64_t. Uses 16*16->32bit multiplications.
 */

