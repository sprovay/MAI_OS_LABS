#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 4096
#define SHM_SIZE (BUFFER_SIZE + sizeof(uint32_t))

typedef struct {
    uint32_t length;
    char data[BUFFER_SIZE];
} shm_data_t;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        exit(EXIT_FAILURE);
    }

    char* filename = argv[1];
    char* shm_name = argv[2];
    char* sem_name = argv[3];
    
    int output_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (output_fd == -1) {
        perror("error: can't open file");
        exit(EXIT_FAILURE);
    }

    int shm_fd = shm_open(shm_name, O_RDWR, 0);
    if (shm_fd == -1) {
        perror("error: failed to open SHM");
        close(output_fd);
        exit(EXIT_FAILURE);
    }

    shm_data_t *shm_data = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_data == MAP_FAILED) {
        perror("error: failed to map SHM");
        close(shm_fd);
        close(output_fd);
        exit(EXIT_FAILURE);
    }

    sem_t *sem = sem_open(sem_name, O_RDWR);
    if (sem == SEM_FAILED) {
        perror("error: failed to open semaphore");
        munmap(shm_data, SHM_SIZE);
        close(shm_fd);
        close(output_fd);
        exit(EXIT_FAILURE);
    }

    const char vowels[] = {'a', 'e', 'i', 'o', 'u', 'y', 
                          'A', 'E', 'I', 'O', 'U', 'Y'};
    const int vowels_count = sizeof(vowels) / sizeof(vowels[0]);
    
    char output_buffer[BUFFER_SIZE];
    bool running = true;
    
    while (running) {
        sem_wait(sem);
        
        if (shm_data->length == UINT32_MAX) {
            running = false;
        } else if (shm_data->length > 0) {
            int output_index = 0;
            for (uint32_t i = 0; i < shm_data->length; i++) {
                if (memchr(vowels, shm_data->data[i], vowels_count) == NULL) {
                    output_buffer[output_index++] = shm_data->data[i];
                }
            }
            
            if (output_index > 0) {
                write(output_fd, output_buffer, output_index);
            }
            
            shm_data->length = 0;
        }
        
        sem_post(sem);
    }

    sem_close(sem);
    munmap(shm_data, SHM_SIZE);
    close(shm_fd);
    close(output_fd);

    return 0;
}