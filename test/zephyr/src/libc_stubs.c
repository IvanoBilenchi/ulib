/**
 * @copyright Copyright (c) 2026 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * Zephyr's embedded C libraries declare the file syscalls that stdio needs, but provide no
 * implementation unless a filesystem is configured. Referencing fopen, as ustream does, is
 * therefore enough to fail the link. These stubs satisfy it, and report that no filesystem is
 * present, which is what a target without one should report anyway.
 */

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

int open(char const *path, int flags, ...);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, void const *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int unlink(char const *path);

int open(char const *path, int flags, ...) {
    (void)path;
    (void)flags;
    errno = ENOSYS;
    return -1;
}

int close(int fd) {
    (void)fd;
    errno = ENOSYS;
    return -1;
}

ssize_t read(int fd, void *buf, size_t count) {
    (void)fd;
    (void)buf;
    (void)count;
    errno = ENOSYS;
    return -1;
}

ssize_t write(int fd, void const *buf, size_t count) {
    (void)fd;
    (void)buf;
    (void)count;
    errno = ENOSYS;
    return -1;
}

off_t lseek(int fd, off_t offset, int whence) {
    (void)fd;
    (void)offset;
    (void)whence;
    errno = ENOSYS;
    return -1;
}

int unlink(char const *path) {
    (void)path;
    errno = ENOSYS;
    return -1;
}
