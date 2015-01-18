#include <inc/types.hh>

typedef struct {
    unint4 address;
    unint4 length;
    char* signature;
} DebugStrTableT;

#ifdef __cplusplus
extern "C" {
#endif

void backtrace_current();
void backtrace_addr(void* currentAddress, size_t stackPtr);
char* getMethodSignature(unint4 address);

#ifdef __cplusplus
}
#endif
