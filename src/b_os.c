
void *os_reserve(u64 reserve_size)
{
    void *result = mmap(0, reserve_size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED) {
        result = 0;
    }
    return result;
}

b32 os_commit(void *ptr, u64 commit_size)
{
    mprotect(ptr, commit_size, PROT_READ|PROT_WRITE);
    return true;
}

void os_decommit(void *ptr, u64 size)
{
    madvise(ptr, size, MADV_DONTNEED);
    mprotect(ptr, size, PROT_NONE);
}

void os_release(void *ptr, u64 size)
{
    munmap(ptr, size);
}
