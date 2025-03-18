#include "arch/cc.h"
#include "lwip/api.h"

// this is all so it compiles without replacing the socket code
// will need to do something here if we want networking to actually work
// (or port some code so we can set NO_SYS)

void pico_lwip_custom_lock_tcpip_core()
{
}

void pico_lwip_custom_unlock_tcpip_core()
{
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    return ERR_ARG;
}

void sys_sem_signal(sys_sem_t *sem)
{
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    return SYS_ARCH_TIMEOUT;
}

void sys_sem_free(sys_sem_t *sem)
{
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    return ERR_ARG;
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    return ERR_ARG;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    return SYS_ARCH_TIMEOUT;
}
 
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    return SYS_MBOX_EMPTY;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
}

u32_t sys_now()
{
    return 0;
}