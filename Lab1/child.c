#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LENGTH 512
#define BUFFER_SIZE 4096

int main(int argc, char* argv[]) {
    if (argc != 2) {
        exit(EXIT_FAILURE);
    }

    char* filename = argv[1];
    
    int output_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (output_fd == -1) {
        perror("error: can`t open file");
        exit(EXIT_FAILURE);
    }

    dup2(output_fd, STDOUT_FILENO);
    close(output_fd);

    const char vowels[] = {'a', 'e', 'i', 'o', 'u', 'y', 
                          'A', 'E', 'I', 'O', 'U', 'Y'};
    const int vowels_count = sizeof(vowels) / sizeof(vowels[0]);
    
    char input_buffer[BUFFER_SIZE];
    char output_buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    while ((bytes_read = read(STDIN_FILENO, input_buffer, BUFFER_SIZE)) > 0) {
        if (bytes_read >= 1 && input_buffer[0] == 1) {
            break;
        }
        int output_index = 0;
        for (int i = 0; i < bytes_read; i++) {
            if (memchr(vowels, input_buffer[i], vowels_count) == NULL) {
                output_buffer[output_index++] = input_buffer[i];
            }
        }
        if (output_index > 0) {
            write(STDOUT_FILENO, output_buffer, output_index);
        }
    }
    if (bytes_read < 0) {
        perror("error: read failed in child");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}