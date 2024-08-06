__syscall:
    stp     x4, x5, [sp, #-16]!
    stp     x7, lr, [sp, #-16]!
    ldr     x4, [sp, #32]
    ldr     x5, [sp, #40]
    mov     x7, x12
    svc     #0

    cmn     x0, #4096
    csneg   x2, x0, x0, cs
    ldr     x3, =errno
    cset    x0, cs
    str     x2, [x3], cs
    ldp     x4, x5, [sp], #16
    ldp     x7, lr, [sp], #16
    ret

.extern errno

.global _exit
_exit:
    mov     x12, #1
    b       __syscall

.global fork
fork:
    mov     x12, #2
    b       __syscall

.global read
read:
    mov     x12, #3
    b       __syscall

.global write
write:
    mov     x12, #4
    b       __syscall

.global open
open:
    mov     x12, #5
    b       __syscall

.global close
close:
    mov     x12, #6
    b       __syscall

.global waitpid
waitpid:
    mov     x12, #7
    b       __syscall

.global creat
creat:
    mov     x12, #8
    b       __syscall

.global link
link:
    mov     x12, #9
    b       __syscall

.global unlink
unlink:
    mov     x12, #10
    b       __syscall

.global execve
execve:
    mov     x12, #11
    b       __syscall

.global chdir
chdir:
    mov     x12, #12
    b       __syscall

.global time
time:
    mov     x12, #13
    b       __syscall

.global mknod
mknod:
    mov     x12, #14
    b       __syscall

.global chmod
chmod:
    mov     x12, #15
    b       __syscall

.global lseek
lseek:
    mov     x12, #19
    b       __syscall

.global getpid
getpid:
    mov     x12, #20
    b       __syscall

.global mount
mount:
    mov     x12, #21
    b       __syscall

.global umount
umount:
    mov     x12, #22
    b       __syscall

.global setuid
setuid:
    mov     x12, #23
    b       __syscall

.global getuid
getuid:
    mov     x12, #24
    b       __syscall

.global access
access:
    mov     x12, #33
    b       __syscall

.global sync
sync:
    mov     x12, #36
    b       __syscall

.global kill
kill:
    mov     x12, #37
    b       __syscall

.global mkdir
mkdir:
    mov     x12, #39
    b       __syscall

.global rmdir
rmdir:
    mov     x12, #40
    b       __syscall

.global dup
dup:
    mov     x12, #41
    b       __syscall

.global pipe
pipe:
    mov     x12, #42
    b       __syscall

.global brk
brk:
    mov     x12, #45
    b       __syscall

.global setgid
setgid:
    mov     x12, #46
    b       __syscall

.global getgid
getgid:
    mov     x12, #47
    b       __syscall

.global geteuid
geteuid:
    mov     x12, #49
    b       __syscall

.global getegid
getegid:
    mov     x12, #50
    b       __syscall

.global ioctl
ioctl:
    mov     x12, #54
    b       __syscall

.global fcntl
fcntl:
    mov     x12, #55
    b       __syscall

.global dup2
dup2:
    mov     x12, #63
    b       __syscall

.global getppid
getppid:
    mov     x12, #64
    b       __syscall

.global setsid
setsid:
    mov     x12, #66
    b       __syscall

.global sigaction
sigaction:
    mov     x12, #67
    b       __syscall

.global gettimeofday
gettimeofday:
    mov     x12, #78
    b       __syscall

.global settimeofday
settimeofday:
    mov     x12, #79
    b       __syscall

.global mmap
mmap:
    mov     x12, #192
    b       __syscall

.global munmap
munmap:
    mov     x12, #91
    b       __syscall

.global stat
stat:
    mov     x12, #106
    b       __syscall

.global lstat
lstat:
    mov     x12, #107
    b       __syscall

.global fstat
fstat:
    mov     x12, #108
    b       __syscall

.global sigreturn
sigreturn:
    mov     x12, #119
    b       __syscall

.global clone
clone:
    mov     x12, #120
    b       __syscall

.global uname
uname:
    mov     x12, #122
    b       __syscall

.global fchdir
fchdir:
    mov     x12, #133
    b       __syscall

.global getdents
getdents:
    mov     x12, #141
    b       __syscall

.global nanosleep
nanosleep:
    mov     x12, #162
    b       __syscall

.global poll
poll:
    mov     x12, #168
    b       __syscall

.global chown
chown:
    mov     x12, #182
    b       __syscall

.global getcwd
getcwd:
    mov     x12, #183
    b       __syscall

