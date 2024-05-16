
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>



#define DEFAULT_PHONEBOOK_FILE "phonebook.txt" // Assuming phonebook.txt is in the same directory



int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s <name> [<phonebook_file>]\n", argv[0]);
    exit(1);
  }

  char *name = argv[1];
  char *phonebook_file = (argc == 3) ? argv[2] : DEFAULT_PHONEBOOK_FILE;

  int pipe1[2], pipe2[2]; // Two pipes for chaining commands
  if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
    perror("pipe");
    exit(1);
  }

  int pid1 = fork(); // First child process (grep)
  if (pid1 == -1) {
    perror("fork");
    exit(1);
  }

  if (pid1 == 0) {
    dup2(pipe1[1], STDOUT_FILENO); // Redirect grep output to pipe1
    close(pipe1[0]); // Close unused read end of pipe1
    close(pipe2[0]); // Close unused read and write ends of pipe2
    close(pipe2[1]);

    // Search for the entire line containing the name (case-insensitive)
    execlp("grep", "grep", "-i", name, phonebook_file, NULL);

    perror("execlp");
    exit(1);
  } else {
    int pid2 = fork(); // Second child process (awk)
    if (pid2 == -1) {
      perror("fork");
      exit(1);
    }

    if (pid2 == 0) {
      dup2(pipe1[0], STDIN_FILENO);  // Redirect awk input from pipe1
      dup2(pipe2[1], STDOUT_FILENO); // Redirect awk output to pipe2
      close(pipe1[1]); // Close unused write end of pipe1
      close(pipe2[0]); // Close unused read end of pipe2

      // Print only the second field (phone number) after splitting by comma
      execlp("awk", "awk", "-F,", "{print $2}", NULL);

      perror("execlp");
      exit(1);
    } else { // Parent process
      close(pipe1[0]); // Close unused read ends
      close(pipe1[1]);
      close(pipe2[1]); // Close unused write end

      int status;
      waitpid(pid1, &status, 0); // Wait for grep (pid1)
      waitpid(pid2, &status, 0); // Wait for awk (pid2)

      if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        // Both grep and awk succeeded, read phone number
        printf("Phone number: ");
        char buffer[12]; // Assuming max phone number length is 9 digits + 1 separator
        ssize_t bytes_read = read(pipe2[0], buffer, sizeof(buffer) - 1);
        if (bytes_read == 0) {
          fprintf(stderr, "Name not found in phonebook\n");
        } else {
          buffer[strcspn(buffer, "\n")] = '\0'; // Remove trailing newline
          printf("%s\n", buffer);
        }
      } else {
        fprintf(stderr, "Name not found in phonebook\n");
      }
    }
  }

  return 0;
}

