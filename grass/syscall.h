/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: the system call interface to user applications
 */

#include "egos.h"
#include "syscall.h"
#include <string.h>

static struct syscall *sc = (struct syscall *)SYSCALL_ARG;

static void sys_invoke()
{
    /* The standard way of system call is using the `ecall` instruction;
     * Here uses software interrupt for simplicity. */
    // *((int *)0x2000000) = 1;
    asm("ecall");
    // while (sc->type != SYS_UNUSED)
    //     ;

    // While loop was previously necessary because it is uncertain when the interrupt will be handled.
    // Thus, the while loop causes the instruction to repeat until the system call can be resolved, as many more instructions may be executed in between.
    // Comparatively, ecall triggers an exception. As exceptions are handled immediately, the while loop is not needed, as the system call will be resolved immediately in sys_invoke.
}

int sys_send(int receiver, char *msg, int size)
{
    if (size > SYSCALL_MSG_LEN)
        return -1;

    sc->type = SYS_SEND;
    sc->msg.receiver = receiver;
    memcpy(sc->msg.content, msg, size);
    sys_invoke();
    return sc->retval;
}

int sys_recv(int *sender, char *buf, int size)
{
    if (size > SYSCALL_MSG_LEN)
        return -1;

    sc->type = SYS_RECV;
    sys_invoke();
    memcpy(buf, sc->msg.content, size);
    if (sender)
        *sender = sc->msg.sender;
    return sc->retval;
}

void sys_exit(int status)
{
    struct proc_request req;
    req.type = PROC_EXIT;
    sys_send(GPID_PROCESS, (void *)&req, sizeof(req));
}
