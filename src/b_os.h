#ifndef OS_H
#define OS_H

void *os_reserve(u64 reserve_size);
b32   os_commit(void *ptr, u64 commit_size);
void  os_decommit(void *ptr, u64 size);
void  os_release(void *ptr, u64 size);

#endif // OS_H
