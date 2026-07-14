/**
 ******************************************************************************
 * @file    syscalls.c
 * @brief   Syscall / retargeting stubs for printf → USART1
 * @note    Portable across GCC (newlib-nano) and ARMCC (microlib).
 *
 *          GCC newlib-nano :  printf → _write(fd, buf, len)
 *          ARMCC microlib  :  printf → fputc(ch, file)
 ******************************************************************************
 */

#include "main.h"

#if defined(__ARMCC_VERSION)
/*============================================================================
  ARM Compiler (Keil) — microlib retargeting
  microlib's printf calls fputc() for each character.
  Enable microlib: Options for Target → Target → Use MicroLIB = ✅
 ============================================================================*/

#include <stdio.h>

int fputc(int ch, FILE *f)
{
    (void)f;
    while (!LL_USART_IsActiveFlag_TXE(DEBUG_USART)) { }
    LL_USART_TransmitData8(DEBUG_USART, (uint8_t)ch);
    return ch;
}

#else /* !__ARMCC_VERSION */
/*============================================================================
  GCC (newlib-nano) — syscall stubs
  Overrides the weak nosys implementations that trigger GCC 15 linker warnings.
 ============================================================================*/

int _write(int fd, const char *buf, int len)
{
    (void)fd;
    for (int i = 0; i < len; i++)
    {
        while (!LL_USART_IsActiveFlag_TXE(DEBUG_USART)) { }
        LL_USART_TransmitData8(DEBUG_USART, (uint8_t)buf[i]);
    }
    return len;
}

int _read(int fd, char *buf, int len)
{
    (void)fd;
    (void)buf;
    (void)len;
    return -1;
}

int _close(int fd)
{
    (void)fd;
    return -1;
}

int _lseek(int fd, int ptr, int dir)
{
    (void)fd;
    (void)ptr;
    (void)dir;
    return -1;
}

int _isatty(int fd)
{
    (void)fd;
    return 1;
}

int _fstat(int fd, void *buf)
{
    (void)fd;
    (void)buf;
    return -1;
}

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    return -1;
}

void _exit(int status)
{
    (void)status;
    while (1) { }
}

#endif /* __ARMCC_VERSION */
