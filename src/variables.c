
typedef struct {
    const char *scene;
} Variables;

static Variables global_variables;


Variables default_variables(void);

void init_variables(void)
{
    FILE *f = fopen("data/all.variables", "r");
    if (!f) {
        log_warn("Failed to find variables (all.variables) file in the data directory, loading defaults");
    }
    char *line = NULL;
    size_t len;
    ssize_t read;

    while ((read = getline(&line, &len, f)) != -1) {

        if (strcmp(line, ":/Scene\n") == 0) {
            read = getline(&line, &len, f);
        }
    }

    fclose(f);
    return;
}

Variables default_variables(void)
{
    return (Variables){
        .scene = "Main",
    };
}
