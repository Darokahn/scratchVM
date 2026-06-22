#pragma once
#ifdef INSTANTIATE
#define instance
#define maybe(value) value
#else
#define instance extern
#define maybe(value)
#endif

extern int x maybe(= 10);
