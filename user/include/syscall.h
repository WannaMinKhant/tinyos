#ifndef __SYSCALL_H
#define __SYSCALL_H

#include <types.h>

#define INTNO_SYSCALL	48

#define SYS_puts	1
#define SYS_gets	2
#define SYS_open	3
#define SYS_read	4
#define SYS_write	5
#define SYS_fsync	6
#define SYS_close	7
#define SYS_lseek	8
#define SYS_fork	9
#define SYS_yield	10
#define SYS_getpid	11
#define SYS_exit	12
#define SYS_wait	13
#define SYS_execute	14

extern int usys_puts(char *);
extern int usys_gets(char *, int);
extern int usys_open(char *, unsigned int);
extern int usys_read(int, char *, unsigned int);
extern int usys_write(int, char *, unsigned int);
extern int usys_fsync(int);
extern int usys_close(int);
extern off_t usys_lseek(int, off_t, unsigned int);
extern int usys_fork(void);
extern void usys_yield(void);
extern int usys_getpid(void);
extern void usys_exit(int);
extern int usys_wait(int *);
extern int usys_execute(char *, int, char **);

#endif	/* syscall.h */
