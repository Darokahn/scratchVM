__attribute__((naked))
void machineLog(const char* fmt, ...) {
    __asm__ volatile ("jmp printf\n");
}
