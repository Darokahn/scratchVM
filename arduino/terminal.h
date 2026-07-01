#pragma once
extern "C" {
#define new new_
    #include "table.h"
#undef new
}

int readBytes(uint8_t* buf, int size);
int* table_lookup(table_t* t, const char* string, unsigned int length);

typedef int (*dispatchFunc)(void*, char*, int);

typedef struct terminal_t {
    table_t table;
    const dispatchFunc* funcs;
    bool finishedRead;
    char protocolByte;
    char* prompt;
} terminal_t;

static terminal_t* terminal_init(terminal_t* t, const dispatchFunc funcs[], const char* names[], int funcCount, char protocolByte) {
    *t = (terminal_t){0};
    t->prompt = ">";
    table_init(&t->table, funcCount * 2); // give table room to breathe
    t->funcs = funcs;
    t->protocolByte = protocolByte;
    for (int i = 0; i < funcCount; i++) {
        *table_insert(&t->table, names[i], strlen(names[i])) = i;
    }
    return t;
}

static int terminal_processText(terminal_t* t) {
    while (Serial.available()) {
    }
}

static int terminal_read(terminal_t* t, uint8_t* buff, int len) {
    Serial.print(t->protocolByte);
    if (buff == NULL || len == 0) return 0;
    int result = readBytes(buff, len);
    return result;
}

void terminal_destroy(terminal_t* t) {
}
