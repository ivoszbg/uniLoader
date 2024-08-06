
__syscall:
	stmfd	sp!, {r4, r5, r7, lr}
	ldr	r4, [sp, #16]
	ldr	r5, [sp, #20]
	mov	r7, r12
	swi	#0

	cmn	r0, #4096
	rsbcs	r2, r0, #0
	ldrcs	r3, =errno
	mvncs	r0, #0
	strcs	r2, [r3]
	ldmfd	sp!, {r4, r5, r7, pc}

.extern errno

.global _exit
_exit:
	mov	r12, #1
	b	__syscall

.global fork
fork:
	mov	r12, #2
	b	__syscall

.global read
read:
	mov	r12, #3
	b	__syscall

.global write
write:
	mov	r12, #4
	b	__syscall

.global open
open:
	mov	r12, #5
	b	__syscall

.global close
close:
	mov	r12, #6
	b	__syscall

.global waitpid
waitpid:
	mov	r12, #7
	b	__syscall

.global creat
creat:
	mov	r12, #8
	b	__syscall

.global link
link:
	mov	r12, #9
	b	__syscall

.global unlink
unlink:
	mov	r12, #10
	b	__syscall

.global execve
execve:
	mov	r12, #11
	b	__syscall

.global chdir
chdir:
	mov	r12, #12
	b	__syscall

.global time
time:
	mov	r12, #13
	b	__syscall

.global mknod
mknod:
	mov	r12, #14
	b	__syscall

.global chmod
chmod:
	mov	r12, #15
	b	__syscall

.global lseek
lseek:
	mov	r12, #19
	b	__syscall

.global getpid
getpid:
	mov	r12, #20
	b	__syscall

.global mount
mount:
	mov	r12, #21
	b	__syscall

.global umount
umount:
	mov	r12, #22
	b	__syscall

.global setuid
setuid:
	mov	r12, #23
	b	__syscall

.global getuid
getuid:
	mov	r12, #24
	b	__syscall

.global access
access:
	mov	r12, #33
	b	__syscall

.global sync
sync:
	mov	r12, #36
	b	__syscall

.global kill
kill:
	mov	r12, #37
	b	__syscall

.global mkdir
mkdir:
	mov	r12, #39
	b	__syscall

.global rmdir
rmdir:
	mov	r12, #40
	b	__syscall

.global dup
dup:
	mov	r12, #41
	b	__syscall

.global pipe
pipe:
	mov	r12, #42
	b	__syscall

.global brk
brk:
	mov	r12, #45
	b	__syscall

.global setgid
setgid:
	mov	r12, #46
	b	__syscall

.global getgid
getgid:
	mov	r12, #47
	b	__syscall

.global geteuid
geteuid:
	mov	r12, #49
	b	__syscall

.global getegid
getegid:
	mov	r12, #50
	b	__syscall

.global ioctl
ioctl:
	mov	r12, #54
	b	__syscall

.global fcntl
fcntl:
	mov	r12, #55
	b	__syscall

.global dup2
dup2:
	mov	r12, #63
	b	__syscall

.global getppid
getppid:
	mov	r12, #64
	b	__syscall

.global setsid
setsid:
	mov	r12, #66
	b	__syscall

.global sigaction
sigaction:
	mov	r12, #67
	b	__syscall

.global gettimeofday
gettimeofday:
	mov	r12, #78
	b	__syscall

.global settimeofday
settimeofday:
	mov	r12, #79
	b	__syscall

.global mmap
mmap:
	mov	r12, #192
	b	__syscall

.global munmap
munmap:
	mov	r12, #91
	b	__syscall

.global stat
stat:
	mov	r12, #106
	b	__syscall

.global lstat
lstat:
	mov	r12, #107
	b	__syscall

.global fstat
fstat:
	mov	r12, #108
	b	__syscall

.global sigreturn
sigreturn:
	mov	r12, #119
	b	__syscall

.global clone
clone:
	mov	r12, #120
	b	__syscall

.global uname
uname:
	mov	r12, #122
	b	__syscall

.global fchdir
fchdir:
	mov	r12, #133
	b	__syscall

.global getdents
getdents:
	mov	r12, #141
	b	__syscall

.global nanosleep
nanosleep:
	mov	r12, #162
	b	__syscall

.global poll
poll:
	mov	r12, #168
	b	__syscall

.global chown
chown:
	mov	r12, #182
	b	__syscall

.global getcwd
getcwd:
	mov	r12, #183
	b	__syscall
