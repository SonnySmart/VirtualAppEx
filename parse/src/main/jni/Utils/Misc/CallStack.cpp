//
// Created by Administrator on 2019/12/26.
//

#include "CallStack.h"
#include "../Includes.h"
#include <unwind.h>
#include <dlfcn.h>

struct BacktraceState
{
    intptr_t* current;
    intptr_t* end;
};


static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg)
{
    BacktraceState* state = static_cast<BacktraceState*>(arg);
    intptr_t ip = (intptr_t)_Unwind_GetIP(context);
    if (ip) {
        if (state->current == state->end) {
            return _URC_END_OF_STACK;
        } else {
            state->current[0] = ip;
            state->current++;
        }
    }
    return _URC_NO_REASON;


}

size_t captureBacktrace(intptr_t* buffer, size_t maxStackDeep)
{
    BacktraceState state = {buffer, buffer + maxStackDeep};
    _Unwind_Backtrace(unwindCallback, &state);
    return state.current - buffer;
}

void dumpBacktraceIndex(char *out, intptr_t* buffer, size_t count)
{
    for (size_t idx = 0; idx < count; ++idx) {
        intptr_t addr = buffer[idx];
        const char* symbol = "      ";
        const char* dlfile="      ";

        Dl_info info;
        if (dladdr((void*)addr, &info)) {
            if(info.dli_sname){
                symbol = info.dli_sname;
            }
            if(info.dli_fname){
                dlfile = info.dli_fname;
            }
        }else{
            strcat(out,"#                               \n");
            continue;
        }
        char temp[50];
        memset(temp,0,sizeof(temp));
        sprintf(temp,"%zu",idx);
        strcat(out,"#");
        strcat(out,temp);
        strcat(out, ": ");
        memset(temp,0,sizeof(temp));
        sprintf(temp,"%zu",addr);
        strcat(out,temp);
        strcat(out, "  " );
        strcat(out, symbol);
        strcat(out, "      ");
        strcat(out, dlfile);
        strcat(out, "\n" );
    }
}

void call_stack() {
    const size_t maxStackDeep = 100;
    intptr_t stackBuf[maxStackDeep];
    char outBuf[4096];
    memset(outBuf,0,sizeof(outBuf));
    dumpBacktraceIndex(outBuf, stackBuf, captureBacktrace(stackBuf, maxStackDeep));
    DUALLOGD("dumpBacktraceIndex----------------------------------------\n[%s]\n----------------------------------------", outBuf);
}