#pragma once

#include <stdint.h>

struct letterRow {
    char cols[3];
};

struct letter {
    struct letterRow rows[5];
};

struct letterSet {
    const struct letter* letters;
    int letterMax;
};

const extern struct letter* const letters;
const extern struct letterSet basicFont;
