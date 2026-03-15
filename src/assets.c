#include "assets.h"

#define BUFFER_SIZE 1024

Asset_Model *
load_model_from_file(const char *file)
{
    char buffer[BUFFER_SIZE];
    char *line = NULL;
    size_t len;
    ssize_t read;

    FILE *f = fopen(file, "r"); 
    if (f == NULL)
        goto ret;

    size_t file_length = fseek(f, 0, SEEK_END);
    fseek(f, 0, SEEK_SET);
        
    while ((read = getline(&line, &len, f)) != -1) {
        if (line[0] == 'v') {
            if (line[1] == 'n') {
                V3f v = {0};
                sscanf(line, "vn %f %f %f", &v.x, &v.y, &v.z);
            } else if (line[1] == 't') {
                V3f v = {0};
                sscanf(line, "vt %f %f %f", &v.x, &v.y, &v.z);
            } else {
                V3f v = {0};
                sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            }
        }
        if (line[0] == 'f') {
            FaceVertex fv = {0};
            int restul = sscanf(line, "f %d/%d");
        }
        printf("Line length = %llu\n", len);
        printf("Line = %s\n", line);
    }
    goto ret;


ret:
    fclose(f);
    return NULL;
}
