#include <inc/types.hh>

typedef struct {
    unsigned int address;
    unsigned int length;
    char signature[64];
} __attribute__((packed)) DebugStrTableEntriesT;

typedef struct {
    unsigned int          numEntries;
    DebugStrTableEntriesT strTable[];
} __attribute__((packed)) DebugStrTableT;


#ifdef __cplusplus
extern "C" {
#endif

void backtrace_current();
void backtrace_addr(void* currentAddress, size_t stackPtr);
char* getMethodSignature(unint4 address);

#ifdef __cplusplus
}
#endif
