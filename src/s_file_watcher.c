

typedef struct {
    String8 name;
    String8 directory;
    u64 last_saved;
    b32 is_dirty;
} File;

typedef struct {
    String8 directory;
    File *files;
    pthread_t thread;
} Directory_Watcher;


void init_directory_watcher(const char *dir_name)
{
    
}
