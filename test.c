// This code snippet has been extracted from Vim (as of 2024-10-11) for a
// self-contained repro for Xcode 16 clang bug. Currently Xcode 16 seems to be
// generating incorrect inline code when using -O2 optimization settings.
// 
// Simply compile the following in Xcode 16 clang with "-O1" and "-O2"
// settings. With -O1, the program will exit cleanly, but with -O2 the program
// will terminate with an error code indicating a wrong result.
//
// Tested with Xcode clang-1600.0.26.3 / Apple M1 Max.
//
#include <stdio.h>

typedef unsigned char char_u;   
#define NUL '\000'

/*
 * Lookup table to quickly get the length in bytes of a UTF-8 character from
 * the first byte of a UTF-8 string.
 * Bytes which are illegal when used as the first byte have a 1.
 * The NUL byte has length 1.
 */
static char utf8len_tab[256] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1,
};

/*
 * Like utf8len_tab above, but using a zero for illegal lead bytes.
 */
static char utf8len_tab_zero[256] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,0,0,
};


    int
utf_char2len(int c)
{
    if (c < 0x80)
        return 1;
    if (c < 0x800)
        return 2;
    if (c < 0x10000)
        return 3;
    if (c < 0x200000)
        return 4;
    if (c < 0x4000000)
        return 5;
    return 6;
}

/*
 * Convert a UTF-8 byte sequence to a character number.
 * If the sequence is illegal or truncated by a NUL the first byte is
 * returned.
 * For an overlong sequence this may return zero.
 * Does not include composing characters, of course.
 */
    int
utf_ptr2char(char_u *p)
{
    if (p[0] < 0x80u) // be quick for ASCII
        return p[0];

    int len = utf8len_tab_zero[p[0]];
    if (len > 1 && (p[1] & 0xc0u) == 0x80u)
    {
        if (len == 2)
            return ((p[0] & 0x1fu) << 6) + (p[1] & 0x3fu);
        if ((p[2] & 0xc0u) == 0x80u)
        {
            if (len == 3)
                return ((p[0] & 0x0fu) << 12) + ((p[1] & 0x3fu) << 6)
                    + (p[2] & 0x3fu);
            if ((p[3] & 0xc0u) == 0x80u)
            {
                if (len == 4)
                    return ((p[0] & 0x07u) << 18) + ((p[1] & 0x3fu) << 12)
                        + ((p[2] & 0x3fu) << 6) + (p[3] & 0x3fu);
                if ((p[4] & 0xc0u) == 0x80u)
                {
                    if (len == 5)
                        return ((p[0] & 0x03u) << 24) + ((p[1] & 0x3fu) << 18)
                            + ((p[2] & 0x3fu) << 12) + ((p[3] & 0x3fu) << 6)
                            + (p[4] & 0x3fu);
                    if ((p[5] & 0xc0u) == 0x80u && len == 6)
                        return ((p[0] & 0x01u) << 30) + ((p[1] & 0x3fu) << 24)
                            + ((p[2] & 0x3fu) << 18) + ((p[3] & 0x3fu) << 12)
                            + ((p[4] & 0x3fu) << 6) + (p[5] & 0x3fu);
                }
            }
        }
    }
    // Illegal value, just return the first byte
    return p[0];
}

/*
 * Get the length of a UTF-8 byte sequence, not including any following
 * composing characters.
 * Returns 0 for "".
 * Returns 1 for an illegal byte sequence.
 */
    int
utf_ptr2len(char_u *p)
{
    if (*p == NUL)
        return 0;
    int len = utf8len_tab[*p];
    for (int i = 1; i < len; ++i)
        if ((p[i] & 0xc0u) != 0x80u)
            return 1;
    return len;
}

/*
 * Find the next illegal byte sequence.
 */
    int
utf_find_illegal(char_u *p_orig)
{
    char_u *p = p_orig;
    int coladd = 0;

    while (*p != NUL)
    {
        // Illegal means that there are not enough trail bytes (checked by
        // utf_ptr2len()) or too many of them (overlong sequence).
        int len = utf_ptr2len(p);
        if (*p >= 0x80u && (len == 1
                    || utf_char2len(utf_ptr2char(p)) != len))
        {
            coladd = (p - p_orig);
            return coladd;
        }
        p += len;
    }
    return coladd;
}


int main() {
    // Find illegal char with a truncated UTF-8 string. \xE2 implies the sequence
    // should be supposed to be 3-bytes long, but in this string it's only
    // 2-bytes long.  It's supposed to return "6" which is the index of the
    // first illegal sequence starting at \xE2.
    int output = utf_find_illegal((char_u*)"abcdef\xE2\x82xyz");

    //printf("Compiled with clang " __clang_version__ "\n");
    printf("Output: %d. Expected: 6\n", output);

    if (output != 6)
    {
        // Incorrect result. Return error code.
        return 1;
    }
    return 0;
}
