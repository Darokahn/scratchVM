#pragma once
extern "C" {
// "just rename the variable in table.h" WAHH WAHH WAHH I'm using C++ because arduino uses C++ and I cuss them out for it every day
// I will not touch my gorgeous C header on behalf of some wicked danish despot
// There are three types of languages:
//      the ones people complain about,
//      the ones nobody uses,
//      and the ones that are invented by some cottonball headed pedant who thinks "durr youre only complaining about it because it's GOOD" is a smart thing to say
// and SAAS pundits love to put it on the first slide of their google slides presentation because they're just as stupid
#define new new_
    #include "table.h"
#undef new
}

int* table_lookup(table_t* t, const char* string, unsigned int length);

typedef int (*dispatchFunc)(void*, char*, int);

typedef struct terminal_t {
    table_t table;
    const dispatchFunc* funcs;
    dispatchFunc currentOwner;
    void* data;
    uint8_t* buffer[1024];
    int readState;
} terminal_t;

static terminal_t* terminal_init(terminal_t* t, const dispatchFunc funcs[], const char* names[], int funcCount, void* data) {
    *t = (terminal_t){0};
    table_init(&t->table, funcCount * 2); // give table room to breathe
    t->funcs = funcs;
    t->data = data;
    for (int i = 0; i < funcCount; i++) {
        *table_insert(&t->table, names[i], strlen(names[i])) = i;
    }
    return t;
}
#define strEnum(str) const char* str##_s = #str; const int str = __COUNTER__;

const struct {
    strEnum(passingText);
    strEnum(parsingCommand);
    strEnum(dispatchingCommand);
} states;

static int terminal_read(terminal_t* t, uint8_t* buffer, int bufsize) {
    int byteCount = readBytes(buffer, bufsize);
    machineLog("%c", 6);
    machineLog("read %d bytes: %.*s\n\r", byteCount, byteCount, buffer);
    int index = 0;
    return index;
}

void terminal_destroy(terminal_t* t) {
}
