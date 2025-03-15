#include "common/utils.h"

int highest_bit(int x)
{
    int index = -1;
    while (x > 0)
    {
        x >>= 1;
        index++;
    }
    return index;
}

int log2_approx(int x)
{
    int y = x + (x >> 1);

    return highest_bit(y);
}