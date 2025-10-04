#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LENGTH 512
#define NUM_PROCESSES 2
#define BUFFER_SIZE 4096

pid_t create_process() {
    pid_t pid = fork();
    if (pid == -1) {
        perror("error: failed to spawn new process\n");
        exit(EXIT_FAILURE);
    }
    return pid;
}

int create_pipe(int pipe_fd[2]) {
    if (pipe(pipe_fd) == -1) {
        perror("error: failed to create pipe\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int main() {
    char file_names[NUM_PROCESSES][MAX_LENGTH];
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    for (int i = 0; i < NUM_PROCESSES; i++) {
        bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            perror("error: failed to read filename\n");
            exit(EXIT_FAILURE);
        }
        buffer[bytes_read] = '\0';
        char *newline = strchr(buffer, '\n');
        if (newline) {
            *newline = '\0';
            strncpy(file_names[i], buffer, MAX_LENGTH - 1);
        } else {
            strncpy(file_names[i], buffer, MAX_LENGTH - 1);
        }
        file_names[i][MAX_LENGTH - 1] = '\0';
    }

    int pipe_fds[NUM_PROCESSES][2];
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        create_pipe(pipe_fds[i]);
    }

    pid_t pids[NUM_PROCESSES];
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pids[i] = create_process();
        
        if (pids[i] == 0) {
            close(pipe_fds[i][1]);
            dup2(pipe_fds[i][0], STDIN_FILENO);
            close(pipe_fds[i][0]);
            
            for (int j = 0; j < NUM_PROCESSES; j++) {
                if (j != i) {
                    close(pipe_fds[j][0]);
                    close(pipe_fds[j][1]);
                }
            }
            
            execl("./child", "child", file_names[i], NULL);
            perror("error: exec failed\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        close(pipe_fds[i][0]);
    }

    int line_counter = 0;
    
    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
        char *current_pos = buffer;
        ssize_t remaining = bytes_read;
        
        while (remaining > 0) {
            char *newline = memchr(current_pos, '\n', remaining);
            ssize_t chunk_size;
            
            if (newline) {
                chunk_size = newline - current_pos + 1;
            } else {
                chunk_size = remaining;
            }
            
            line_counter++;
            int target_process = (line_counter % 2 == 1) ? 0 : 1;
            
            write(pipe_fds[target_process][1], current_pos, chunk_size);
            
            current_pos += chunk_size;
            remaining -= chunk_size;
        }
    }

    if (bytes_read < 0) {
        perror("error: failed to read from stdin\n");
        exit(EXIT_FAILURE);
    }

    const char eof[] = {1, '\n'};
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        write(pipe_fds[i][1], eof, sizeof(eof));
        close(pipe_fds[i][1]);
    }

    wait(NULL);
    wait(NULL);
}