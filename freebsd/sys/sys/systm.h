/*-
 * Copyright (c) 1982, 1988, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)systm.h	8.7 (Berkeley) 3/29/95
 * $FreeBSD$
 */

#ifndef _SYS_SYSTM_H_
#define	_SYS_SYSTM_H_

#include <machine/atomic.h>
#include <machine/cpufunc.h>
#include <sys/callout.h>
#include <sys/cdefs.h>
#include <sys/queue.h>
#include <sys/stdint.h>		/* for people using printf mainly */
#ifdef __rtems__
#include <string.h>
#endif /* __rtems__ */

__NULLABILITY_PRAGMA_PUSH

#ifndef __rtems__
extern int cold;		/* nonzero if we are doing a cold boot */
extern int suspend_blocked;	/* block suspend due to pending shutdown */
extern int rebooting;		/* kern_reboot() has been called. */
#else /* __rtems__ */
/* In RTEMS there is no cold boot and reboot */
#define cold 0
#define rebooting 0
#endif /* __rtems__ */
#ifndef __rtems__
extern const char *panicstr;	/* panic message */
#else /* __rtems__ */
#define panicstr NULL
#endif /* __rtems__ */
extern char version[];		/* system version */
extern char compiler_version[];	/* compiler version */
extern char copyright[];	/* system copyright */
extern int kstack_pages;	/* number of kernel stack pages */

extern u_long pagesizes[];	/* supported page sizes */
extern long physmem;		/* physical memory */
extern long realmem;		/* 'real' memory */

extern char *rootdevnames[2];	/* names of possible root devices */

extern int boothowto;		/* reboot flags, from console subsystem */
#ifndef __rtems__
extern int bootverbose;		/* nonzero to print verbose messages */
#else /* __rtems__ */
#ifdef BOOTVERBOSE
extern int bootverbose;		/* nonzero to print verbose messages */
#else
#define bootverbose    0        /* Remove all verbose code for the standard RTEMS build */
#endif /* BOOTVERBOSE */
#endif /* __rtems__ */


extern int maxusers;		/* system tune hint */
extern int ngroups_max;		/* max # of supplemental groups */
#ifndef __rtems__
extern int vm_guest;		/* Running as virtual machine guest? */
#else /* __rtems__ */
#define vm_guest VM_GUEST_NO
#endif /* __rtems__ */

/*
 * Detected virtual machine guest types. The intention is to expand
 * and/or add to the VM_GUEST_VM type if specific VM functionality is
 * ever implemented (e.g. vendor-specific paravirtualization features).
 * Keep in sync with vm_guest_sysctl_names[].
 */
enum VM_GUEST { VM_GUEST_NO = 0, VM_GUEST_VM, VM_GUEST_XEN, VM_GUEST_HV,
		VM_GUEST_VMWARE, VM_GUEST_KVM, VM_LAST };

#if defined(WITNESS) || defined(INVARIANT_SUPPORT)
#ifndef __rtems__
void	kassert_panic(const char *fmt, ...)  __printflike(1, 2);
#else /* __rtems__ */
#define	kassert_panic panic
#endif /* __rtems__ */
#endif

#ifdef	INVARIANTS		/* The option is always available */
#define	KASSERT(exp,msg) do {						\
	if (__predict_false(!(exp)))					\
		kassert_panic msg;					\
} while (0)
#define	VNASSERT(exp, vp, msg) do {					\
	if (__predict_false(!(exp))) {					\
		vn_printf(vp, "VNASSERT failed\n");			\
		kassert_panic msg;					\
	}								\
} while (0)
#else
#define	KASSERT(exp,msg) do { \
} while (0)

#define	VNASSERT(exp, vp, msg) do { \
} while (0)
#endif

#ifndef CTASSERT	/* Allow lint to override */
#define	CTASSERT(x)	_Static_assert(x, "compile-time assertion failed")
#endif

/*
 * Assert that a pointer can be loaded from memory atomically.
 *
 * This assertion enforces stronger alignment than necessary.  For example,
 * on some architectures, atomicity for unaligned loads will depend on
 * whether or not the load spans multiple cache lines.
 */
#define	ASSERT_ATOMIC_LOAD_PTR(var, msg)				\
	KASSERT(sizeof(var) == sizeof(void *) &&			\
	    ((uintptr_t)&(var) & (sizeof(void *) - 1)) == 0, msg)

/*
 * Assert that a thread is in critical(9) section.
 */
#define	CRITICAL_ASSERT(td)						\
	KASSERT((td)->td_critnest >= 1, ("Not in critical section"));
 
/*
 * If we have already panic'd and this is the thread that called
 * panic(), then don't block on any mutexes but silently succeed.
 * Otherwise, the kernel will deadlock since the scheduler isn't
 * going to run the thread that holds any lock we need.
 */
#ifndef __rtems__
#define	SCHEDULER_STOPPED_TD(td)  ({					\
	MPASS((td) == curthread);					\
	__predict_false((td)->td_stopsched);				\
})
#else /* __rtems__ */
#define	SCHEDULER_STOPPED_TD(td) 0
#endif /* __rtems__ */
#define	SCHEDULER_STOPPED() SCHEDULER_STOPPED_TD(curthread)

/*
 * Align variables.
 */
#define	__read_mostly		__section(".data.read_mostly")
#define	__exclusive_cache_line	__aligned(CACHE_LINE_SIZE) \
				    __section(".data.exclusive_cache_line")
/*
 * XXX the hints declarations are even more misplaced than most declarations
 * in this file, since they are needed in one file (per arch) and only used
 * in two files.
 * XXX most of these variables should be const.
 */
extern int osreldate;
extern int envmode;
extern int hintmode;		/* 0 = off. 1 = config, 2 = fallback */
extern int dynamic_kenv;
extern struct mtx kenv_lock;
extern char *kern_envp;
extern char static_env[];
extern char static_hints[];	/* by config for now */

extern char **kenvp;

extern const void *zero_region;	/* address space maps to a zeroed page	*/

extern int unmapped_buf_allowed;

#ifdef __LP64__
#define	IOSIZE_MAX		iosize_max()
#define	DEVFS_IOSIZE_MAX	devfs_iosize_max()
#else
#define	IOSIZE_MAX		SSIZE_MAX
#define	DEVFS_IOSIZE_MAX	SSIZE_MAX
#endif

/*
 * General function declarations.
 */

struct inpcb;
struct lock_object;
struct malloc_type;
struct mtx;
struct proc;
struct socket;
struct thread;
struct tty;
struct ucred;
struct uio;
struct _jmp_buf;
struct trapframe;
struct eventtimer;

#ifndef __rtems__
int	setjmp(struct _jmp_buf *) __returns_twice;
void	longjmp(struct _jmp_buf *, int) __dead2;
#endif /* __rtems__ */
int	dumpstatus(vm_offset_t addr, off_t count);
int	nullop(void);
int	eopnotsupp(void);
int	ureadc(int, struct uio *);
void	hashdestroy(void *, struct malloc_type *, u_long);
void	*hashinit(int count, struct malloc_type *type, u_long *hashmask);
void	*hashinit_flags(int count, struct malloc_type *type,
    u_long *hashmask, int flags);
#define	HASH_NOWAIT	0x00000001
#define	HASH_WAITOK	0x00000002

void	*phashinit(int count, struct malloc_type *type, u_long *nentries);
void	*phashinit_flags(int count, struct malloc_type *type, u_long *nentries,
    int flags);
void	g_waitidle(void);

void	panic(const char *, ...) __dead2 __printflike(1, 2);
void	vpanic(const char *, __va_list) __dead2 __printflike(1, 0);

void	cpu_boot(int);
void	cpu_flush_dcache(void *, size_t);
void	cpu_rootconf(void);
#ifndef __rtems__
void	critical_enter(void);
void	critical_exit(void);
#else /* __rtems__ */
#include <rtems/score/threaddispatch.h>

static __inline void
critical_enter(void)
{
	_Thread_Dispatch_disable();
}

static __inline void
critical_exit(void)
{
	_Thread_Dispatch_enable(_Per_CPU_Get());
}
#endif /* __rtems__ */
void	init_param1(void);
void	init_param2(long physpages);
void	init_static_kenv(char *, size_t);
void	tablefull(const char *);
#ifdef  EARLY_PRINTF
typedef void early_putc_t(int ch);
extern early_putc_t *early_putc;
#endif
int	kvprintf(char const *, void (*)(int, void*), void *, int,
	    __va_list) __printflike(1, 0);
void	log(int, const char *, ...) __printflike(2, 3);
void	log_console(struct uio *);
void	vlog(int, const char *, __va_list) __printflike(2, 0);
int	asprintf(char **ret, struct malloc_type *mtp, const char *format, 
	    ...) __printflike(3, 4);
int	printf(const char *, ...) __printflike(1, 2);
int	snprintf(char *, size_t, const char *, ...) __printflike(3, 4);
int	sprintf(char *buf, const char *, ...) __printflike(2, 3);
int	uprintf(const char *, ...) __printflike(1, 2);
int	vprintf(const char *, __va_list) __printflike(1, 0);
int	vasprintf(char **ret, struct malloc_type *mtp, const char *format,
	    __va_list ap) __printflike(3, 0);
int	vsnprintf(char *, size_t, const char *, __va_list) __printflike(3, 0);
int	vsnrprintf(char *, size_t, int, const char *, __va_list) __printflike(4, 0);
int	vsprintf(char *buf, const char *, __va_list) __printflike(2, 0);
int	ttyprintf(struct tty *, const char *, ...) __printflike(2, 3);
int	sscanf(const char *, char const * _Nonnull, ...) __scanflike(2, 3);
int	vsscanf(const char * _Nonnull, char const * _Nonnull, __va_list)  __scanflike(2, 0);
long	strtol(const char *, char **, int);
u_long	strtoul(const char *, char **, int);
#ifndef __rtems__
quad_t	strtoq(const char *, char **, int);
u_quad_t strtouq(const char *, char **, int);
#else /* __rtems__ */
long long strtoll(const char *, char **, int);
unsigned long long strtoull(const char *, char **, int);

static inline quad_t
strtoq(const char *nptr, char **endptr, int base)
{

	return (strtoll(nptr, endptr, base));
}

static inline u_quad_t
strtouq(const char *nptr, char **endptr, int base)
{

	return (strtoull(nptr, endptr, base));
}
#endif /* __rtems__ */
void	tprintf(struct proc *p, int pri, const char *, ...) __printflike(3, 4);
void	vtprintf(struct proc *, int, const char *, __va_list) __printflike(3, 0);
void	hexdump(const void *ptr, int length, const char *hdr, int flags);
#define	HD_COLUMN_MASK	0xff
#define	HD_DELIM_MASK	0xff00
#define	HD_OMIT_COUNT	(1 << 16)
#define	HD_OMIT_HEX	(1 << 17)
#define	HD_OMIT_CHARS	(1 << 18)

#define ovbcopy(f, t, l) bcopy((f), (t), (l))
#ifndef __rtems__
void	bcopy(const void * _Nonnull from, void * _Nonnull to, size_t len);
void	bzero(void * _Nonnull buf, size_t len);
#else /* __rtems__ */
#define	bcopy(src, dst, len) memmove((dst), (src), (len))
#define	bzero(buf, size) memset((buf), 0, (size))
#endif /* __rtems__ */
void	explicit_bzero(void * _Nonnull, size_t);

void	*memcpy(void * _Nonnull to, const void * _Nonnull from, size_t len);
void	*memmove(void * _Nonnull dest, const void * _Nonnull src, size_t n);

#ifndef __rtems__
int	copystr(const void * _Nonnull __restrict kfaddr,
	    void * _Nonnull __restrict kdaddr, size_t len,
	    size_t * __restrict lencopied);
int	copyinstr(const void * __restrict udaddr,
	    void * _Nonnull __restrict kaddr, size_t len,
	    size_t * __restrict lencopied);
int	copyin(const void * _Nonnull __restrict udaddr,
	    void * _Nonnull __restrict kaddr, size_t len);
int	copyin_nofault(const void * _Nonnull __restrict udaddr,
	    void * _Nonnull __restrict kaddr, size_t len);
int	copyout(const void * _Nonnull __restrict kaddr,
	    void * _Nonnull __restrict udaddr, size_t len);
int	copyout_nofault(const void * _Nonnull __restrict kaddr,
	    void * _Nonnull __restrict udaddr, size_t len);
#else /* __rtems__ */
static inline int
copyinstr(const void * __restrict udaddr, void * __restrict kaddr,
	    size_t len, size_t * __restrict lencopied)
{
	if (lencopied != NULL) {
		*lencopied = len;
	}

	memcpy(kaddr, udaddr, len);

	return (0);
}

static inline int
copyin(const void * __restrict udaddr, void * __restrict kaddr,
    size_t len)
{
	memcpy(kaddr, udaddr, len);

	return (0);
}

static inline int
copyin_nofault(const void * __restrict udaddr, void * __restrict kaddr,
    size_t len)
{
	return copyin(udaddr, kaddr, len);
}

static inline int
copyout(const void * __restrict kaddr, void * __restrict udaddr,
    size_t len)
{
	memcpy(udaddr, kaddr, len);

	return (0);
}

static inline int
copyout_nofault(const void * __restrict kaddr, void * __restrict udaddr,
    size_t len)
{
	return copyout(kaddr, udaddr, len);
}
#endif /* __rtems__ */

#ifndef __rtems__
int	fubyte(volatile const void *base);
#else /* __rtems__ */
static inline int
fubyte(const void *base)
{
  const unsigned char *byte_base = (const unsigned char *)base;

  return byte_base[0];
}
#endif /* __rtems__ */
long	fuword(volatile const void *base);
int	fuword16(volatile const void *base);
int32_t	fuword32(volatile const void *base);
int64_t	fuword64(volatile const void *base);
int	fueword(volatile const void *base, long *val);
int	fueword32(volatile const void *base, int32_t *val);
int	fueword64(volatile const void *base, int64_t *val);
int	subyte(volatile void *base, int byte);
int	suword(volatile void *base, long word);
int	suword16(volatile void *base, int word);
int	suword32(volatile void *base, int32_t word);
int	suword64(volatile void *base, int64_t word);
uint32_t casuword32(volatile uint32_t *base, uint32_t oldval, uint32_t newval);
u_long	casuword(volatile u_long *p, u_long oldval, u_long newval);
int	casueword32(volatile uint32_t *base, uint32_t oldval, uint32_t *oldvalp,
	    uint32_t newval);
int	casueword(volatile u_long *p, u_long oldval, u_long *oldvalp,
	    u_long newval);

void	realitexpire(void *);

int	sysbeep(int hertz, int period);

void	hardclock(int usermode, uintfptr_t pc);
void	hardclock_cnt(int cnt, int usermode);
void	hardclock_cpu(int usermode);
void	hardclock_sync(int cpu);
#ifndef __rtems__
void	softclock(void *);
#endif /* __rtems__ */
void	statclock(int usermode);
void	statclock_cnt(int cnt, int usermode);
void	profclock(int usermode, uintfptr_t pc);
void	profclock_cnt(int cnt, int usermode, uintfptr_t pc);

int	hardclockintr(void);

void	startprofclock(struct proc *);
void	stopprofclock(struct proc *);
void	cpu_startprofclock(void);
void	cpu_stopprofclock(void);
sbintime_t 	cpu_idleclock(void);
void	cpu_activeclock(void);
void	cpu_new_callout(int cpu, sbintime_t bt, sbintime_t bt_opt);
void	cpu_et_frequency(struct eventtimer *et, uint64_t newfreq);
extern int	cpu_disable_c2_sleep;
extern int	cpu_disable_c3_sleep;

char	*kern_getenv(const char *name);
void	freeenv(char *env);
int	getenv_int(const char *name, int *data);
int	getenv_uint(const char *name, unsigned int *data);
int	getenv_long(const char *name, long *data);
int	getenv_ulong(const char *name, unsigned long *data);
int	getenv_string(const char *name, char *data, int size);
int	getenv_int64(const char *name, int64_t *data);
int	getenv_uint64(const char *name, uint64_t *data);
int	getenv_quad(const char *name, quad_t *data);
int	kern_setenv(const char *name, const char *value);
int	kern_unsetenv(const char *name);
int	testenv(const char *name);

typedef uint64_t (cpu_tick_f)(void);
void set_cputicker(cpu_tick_f *func, uint64_t freq, unsigned var);
extern cpu_tick_f *cpu_ticks;
uint64_t cpu_tickrate(void);
uint64_t cputick2usec(uint64_t tick);

#ifdef APM_FIXUP_CALLTODO
struct timeval;
void	adjust_timeout_calltodo(struct timeval *time_change);
#endif /* APM_FIXUP_CALLTODO */

#include <sys/libkern.h>

/* Initialize the world */
void	consinit(void);
void	cpu_initclocks(void);
void	cpu_initclocks_bsp(void);
void	cpu_initclocks_ap(void);
void	usrinfoinit(void);

/* Finalize the world */
void	kern_reboot(int) __dead2;
void	shutdown_nice(int);

/* Timeouts */
typedef void timeout_t(void *);	/* timeout function type */
#define CALLOUT_HANDLE_INITIALIZER(handle)	\
	{ NULL }

void	callout_handle_init(struct callout_handle *);
struct	callout_handle timeout(timeout_t *, void *, int);
void	untimeout(timeout_t *, void *, struct callout_handle);

/* Stubs for obsolete functions that used to be for interrupt management */
#ifdef __rtems__
typedef int intrmask_t;
#endif /* __rtems__ */
static __inline intrmask_t	splbio(void)		{ return 0; }
static __inline intrmask_t	splcam(void)		{ return 0; }
static __inline intrmask_t	splclock(void)		{ return 0; }
static __inline intrmask_t	splhigh(void)		{ return 0; }
static __inline intrmask_t	splimp(void)		{ return 0; }
static __inline intrmask_t	splnet(void)		{ return 0; }
static __inline intrmask_t	spltty(void)		{ return 0; }
static __inline void		splx(intrmask_t ipl __unused)	{ return; }

/*
 * Common `proc' functions are declared here so that proc.h can be included
 * less often.
 */
int	_sleep(void * _Nonnull chan, struct lock_object *lock, int pri,
	   const char *wmesg, sbintime_t sbt, sbintime_t pr, int flags);
#define	msleep(chan, mtx, pri, wmesg, timo)				\
	_sleep((chan), &(mtx)->lock_object, (pri), (wmesg),		\
	    tick_sbt * (timo), 0, C_HARDCLOCK)
#define	msleep_sbt(chan, mtx, pri, wmesg, bt, pr, flags)		\
	_sleep((chan), &(mtx)->lock_object, (pri), (wmesg), (bt), (pr),	\
	    (flags))
#ifndef __rtems__
int	msleep_spin_sbt(void * _Nonnull chan, struct mtx *mtx,
	    const char *wmesg, sbintime_t sbt, sbintime_t pr, int flags);
#else /* __rtems__ */
#define	msleep_spin_sbt(chan, mtx, wmesg, sbt, pr, flags)		\
	msleep_sbt(chan, mtx, 0, wmesg, sbt, pr, flags)
#endif /* __rtems__ */
#define	msleep_spin(chan, mtx, wmesg, timo)				\
	msleep_spin_sbt((chan), (mtx), (wmesg), tick_sbt * (timo),	\
	    0, C_HARDCLOCK)
int	pause_sbt(const char *wmesg, sbintime_t sbt, sbintime_t pr,
	    int flags);
#ifdef __rtems__
#include <unistd.h>
#endif /* __rtems__ */
#define	pause(wmesg, timo)						\
	pause_sbt((wmesg), tick_sbt * (timo), 0, C_HARDCLOCK)
#define	tsleep(chan, pri, wmesg, timo)					\
	_sleep((chan), NULL, (pri), (wmesg), tick_sbt * (timo),		\
	    0, C_HARDCLOCK)
#define	tsleep_sbt(chan, pri, wmesg, bt, pr, flags)			\
	_sleep((chan), NULL, (pri), (wmesg), (bt), (pr), (flags))
void	wakeup(void * chan);
void	wakeup_one(void * chan);

/*
 * Common `struct cdev *' stuff are declared here to avoid #include poisoning
 */

struct cdev;
dev_t dev2udev(struct cdev *x);
const char *devtoname(struct cdev *cdev);

#ifdef __LP64__
size_t	devfs_iosize_max(void);
size_t	iosize_max(void);
#endif

int poll_no_poll(int events);

/* XXX: Should be void nanodelay(u_int nsec); */
void	DELAY(int usec);

/* Root mount holdback API */
struct root_hold_token;

struct root_hold_token *root_mount_hold(const char *identifier);
void root_mount_rel(struct root_hold_token *h);
int root_mounted(void);


/*
 * Unit number allocation API. (kern/subr_unit.c)
 */
struct unrhdr;
struct unrhdr *new_unrhdr(int low, int high, struct mtx *mutex);
void init_unrhdr(struct unrhdr *uh, int low, int high, struct mtx *mutex);
void delete_unrhdr(struct unrhdr *uh);
void clean_unrhdr(struct unrhdr *uh);
void clean_unrhdrl(struct unrhdr *uh);
int alloc_unr(struct unrhdr *uh);
int alloc_unr_specific(struct unrhdr *uh, u_int item);
int alloc_unrl(struct unrhdr *uh);
void free_unr(struct unrhdr *uh, u_int item);

void	intr_prof_stack_use(struct thread *td, struct trapframe *frame);

void counted_warning(unsigned *counter, const char *msg);

__NULLABILITY_PRAGMA_POP

#endif /* !_SYS_SYSTM_H_ */
