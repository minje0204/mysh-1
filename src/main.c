#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "commands.h"
#include "built_in.h"
#include "utils.h"
#include "signal_handlers.h"

int server_fd;

void server_handler()
{
  struct sockaddr_in client_addr;
  int pid, client_addr_len;
  int sock_fd;
  while (1)
  {
    sock_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    pid = fork();
    if (pid == -1)
    {
      close(sock_fd);
      continue;
    }
    else if (pid == 0) //child
    {
      pid = fork();

      if (pid == 0)
      {
        //close(fd[OUTPUT_END]);
        close(0);
        dup2(sock_fd, 0);
        close(sock_fd);
        execlp("wc", "wc", "-l", (char *)NULL);
      }
      close(sock_fd);
      exit(0);
      break;
    }
    else //handler
    {
      close(sock_fd);
      printf("here2\n");
      continue;
    }
  }
}

int init_socket(char *addr, int port)
{
  struct sockaddr_in server_addr;
  int sock_fd;
  int tid;
  memset(&server_addr, 0, sizeof(server_addr));

  sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (sock_fd == -1)
  {
    perror("socket error"); 
    return 0;
  }
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(addr);
  server_addr.sin_port = htons(port);

  if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
  {
    perror("bind error");
    return 0;
  }

  if (listen(sock_fd, 5) == -1)
  {
    perror("listen error");
    return 0;
  }
  server_fd = sock_fd;
  pthread_create(&tid, NULL, server_handler, NULL);
}



int main()
{
  char buf[8096];
  init_sig();
  
  while (1) {
    fgets(buf, 8096, stdin);
    struct single_command commands[513];
    int n_commands = 0;
    mysh_parse_command(buf, &n_commands, &commands);
    int ret = evaluate_command(n_commands, &commands);

    memset(buf, 0, sizeof(buf));
    free_commands(n_commands, &commands);
    n_commands = 0;
    if (ret == 1) {
      break;
    }
  }

  return 0;
}
