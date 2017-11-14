#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#include "commands.h"
#include "built_in.h"

char **bg_argv;
int bg_argc;
int bg_pid;

int pipe_pid;
char **pipe_argv;
int pipe_argc;
int pipe_fd[2];

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}


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
           S_ISREG(sb.st_mode) &&
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
    return is_resolved == 1;
}

int execute(char **argv, int argc)
{
  int pid, status;
  int background = 0;
  int i;

  pid = fork();

  if (pid == -1)
  {
    return 0;
  }

  char *arg = argv[argc - 1];
  char *end;
  if (*(end = &arg[strlen(arg) - 1]) == '&')
  {
    *end = '\0';
    if (!arg[0]) //if seperated white space
    {
      argv[argc-1] = NULL;
      argc--;
    }
    bg_pid = pid;
    background = 1;
  }

  if (pid == 0)
  {
    char resolved_path[512];
    char *path = argv[0];
    if(resolve_path(path, resolved_path, sizeof(resolved_path)))
    {
      path = resolved_path;
      argv[0] = path;
    }
    if (execv(path, argv) < 0)
    {
      exit(-1);
    }
    exit(0);
  }
  else
  {
    if (background)
    {
      bg_argv = malloc(sizeof(*argv)*(argc+1));
      for (i = 0; i < argc; i++)
      {
        if(argv[i])
          bg_argv[i] = strdup(argv[i]);
      }
      bg_argc = argc;
      bg_argv[i] = 0;
    }
    else
      wait(&status);//pid(pid, &status, 0);
  }
  return 1;
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
  int ret;
  if (n_commands > 0) {
    struct single_command *com = commands;
    for(int i_com = 0; i_com < n_commands; i_com++, com++)
    {
        ret = 0;
        int exec_ret = 0;
        assert(com->argc != 0);

        int built_in_pos = is_built_in_command(com->argv[0]);
        if (built_in_pos != -1) {
            if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
                if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
                    fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
                    ret = -1;
                }
            }
            else {
                fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
                ret = -1;
            }
        } else if (strcmp(com->argv[0], "") == 0) {

        } else if (strcmp(com->argv[0], "exit") == 0) {
            return 1;
        } else if ((exec_ret = execute(com->argv, com->argc))) {
        } else {
            fprintf(stderr, "%s: command not found\n", com->argv[0]);
            if(!exec_ret) //exec failed
              exit(1);
            ret = -1;
        }
    }
  }
  return ret;
}

void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
