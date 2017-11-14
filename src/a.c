#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

int resolve_path(char *name, char *resolved_path, size_t size)
{  
    int is_resolved = 0;
    char *env_path,  *dir, *path;
    struct stat sb;
    char *sys_env_path = getenv("PATH");
    int sys_env_path_len = strlen(sys_env_path);
    if(!name[0])
        return 0;
        
    env_path = malloc(sys_env_path_len + 1);
    if(env_path == -1)
        return 0;
    strncpy(env_path, sys_env_path, sys_env_path_len); 
    
    dir = strtok (env_path,":");
    while (dir != NULL)
    {
        int path_len = strlen(dir) + 1 + strlen(name) + 1;
        path = malloc(path_len);
        if(path == -1)
            return 0;
        path[0] = '\0';
        strncat(path, dir, path_len);
        strncat(path, "/", path_len);
        strncat(path, name, path_len);

        memset(&sb, 0, sizeof(stat));
        if(stat(path, &sb) != -1 && 
           (sb.st_mode & S_IFMT) == S_IFREG && 
           !is_resolved) // not found
        {
            strncpy(resolved_path, path, size);
            resolved_path[strlen(resolved_path)] = '\0';
            is_resolved = 1; // for whole free processes
        }

        free(path);
        dir = strtok (NULL, ":");
    }
    
    free(env_path);
    return is_resolved == 0;
}

int main(int argc, char **argv)
{
    char path[256];
    resolve_path("", path, sizeof(path));
    printf("%s\n",path);
    return 0;
}

