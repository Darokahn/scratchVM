#pragma once
extern "C" {
#define new new_
    #include "table.h"
#undef new
}

int* table_lookup(table_t* t, const char* string, unsigned int length);

typedef int (*dispatchFunc)(void*, char*, int);

typedef enum terminalState {
    TERMINAL_BIN,       // BIN      lets arbitrary read calls consume the unprocessed buffer.
    TERMINAL_PROMPT,    // PROMPT   interprets incoming characters to dispatch arbitrary functions. After matching, function owns input stream until it surrenders.
    TERMINAL_SPLIT=-1,  // SPLIT    would process commands while also maintaining a readable buffer. 
} terminalState;

typedef struct terminal_t {
    table_t table;
    const dispatchFunc* funcs;
    bool finishedRead;
    char bufferSentinel;
    char protocolByte;
    char* prompt;
} terminal_t;

static terminal_t* terminal_init(terminal_t* t, const dispatchFunc funcs[], const char* names[], int funcCount, char protocolByte) {
    machineLog("starting terminal with protocol byte %d\n\r", protocolByte);
    *t = (terminal_t){0};
    t->prompt = ">";
    table_init(&t->table, funcCount * 2); // give table room to breathe
    t->funcs = funcs;
    t->bufferSentinel = '\n';
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
    if (buff == NULL || len == 0) return 0;
    int result = readBytes(buff, len);
    return result;
}

void terminal_destroy(terminal_t* t) {
}
