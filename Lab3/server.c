#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <stdint.h>
#include <errno.h>

#define MAX_LENGTH 512
#define NUM_PROCESSES 2
#define BUFFER_SIZE 4096
#define SHM_SIZE (BUFFER_SIZE + sizeof(uint32_t))

typedef struct {
    uint32_t length;
    char data[BUFFER_SIZE];
} shm_data_t;


void generate_names(char* shm_name, char* sem_name, int index, size_t size) {
    pid_t pid = getpid();
    snprintf(shm_name, size, "lab3-shm-%d-%d", pid, index);
    snprintf(sem_name, size, "lab3-sem-%d-%d", pid, index);
}

pid_t create_process() {
    pid_t pid = fork();
    if (pid == -1) {
        perror("error: failed to spawn new process\n");
        exit(EXIT_FAILURE);
    }
    return pid;
}

int main() {
    char file_names[NUM_PROCESSES][MAX_LENGTH];
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    char shm_names[NUM_PROCESSES][64];
    char sem_names[NUM_PROCESSES][64];
    int shm_fds[NUM_PROCESSES];
    sem_t *sems[NUM_PROCESSES];
    shm_data_t *shm_data[NUM_PROCESSES];

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

    for (int i = 0; i < NUM_PROCESSES; i++) {
        generate_names(shm_names[i], sem_names[i], i, sizeof(shm_names[i]));

        shm_fds[i] = shm_open(shm_names[i], O_RDWR | O_CREAT | O_EXCL, 0600);
        if (shm_fds[i] == -1) {
            perror("error: failed to create SHM\n");
            for (int j = 0; j < i; j++) {
                close(shm_fds[j]);
                shm_unlink(shm_names[j]);
                sem_close(sems[j]);
                sem_unlink(sem_names[j]);
            }
            exit(EXIT_FAILURE);
        }

        if (ftruncate(shm_fds[i], SHM_SIZE) == -1) {
            perror("error: failed to set SHM size\n");
            close(shm_fds[i]);
            shm_unlink(shm_names[i]);
            for (int j = 0; j < i; j++) {
                close(shm_fds[j]);
                shm_unlink(shm_names[j]);
                sem_close(sems[j]);
                sem_unlink(sem_names[j]);
            }
            exit(EXIT_FAILURE);
        }

        shm_data[i] = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fds[i], 0);
        if (shm_data[i] == MAP_FAILED) {
            perror("error: failed to map SHM\n");
            close(shm_fds[i]);
            shm_unlink(shm_names[i]);
            for (int j = 0; j < i; j++) {
                close(shm_fds[j]);
                shm_unlink(shm_names[j]);
                sem_close(sems[j]);
                sem_unlink(sem_names[j]);
            }
            exit(EXIT_FAILURE);
        }

        shm_data[i]->length = 0;

        sems[i] = sem_open(sem_names[i], O_CREAT | O_EXCL, 0600, 1);
        if (sems[i] == SEM_FAILED) {
            perror("error: failed to create semaphore\n");
            munmap(shm_data[i], SHM_SIZE);
            close(shm_fds[i]);
            shm_unlink(shm_names[i]);
            for (int j = 0; j < i; j++) {
                munmap(shm_data[j], SHM_SIZE);
                close(shm_fds[j]);
                shm_unlink(shm_names[j]);
                sem_close(sems[j]);
                sem_unlink(sem_names[j]);
            }
            exit(EXIT_FAILURE);
        }
    }

    pid_t pids[NUM_PROCESSES];
    
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pids[i] = create_process();
        
        if (pids[i] == 0) {
            execl("./client", "client", file_names[i], shm_names[i], sem_names[i], NULL);
            perror("error: exec failed\n");
            exit(EXIT_FAILURE);
        }
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
            
            sem_wait(sems[target_process]);
            
            if (chunk_size <= BUFFER_SIZE) {
                shm_data[target_process]->length = chunk_size;
                memcpy(shm_data[target_process]->data, current_pos, chunk_size);
            }
            
            sem_post(sems[target_process]);
            
            current_pos += chunk_size;
            remaining -= chunk_size;
        }
    }

    if (bytes_read < 0) {
        perror("error: failed to read from stdin\n");
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        sem_wait(sems[i]);
        shm_data[i]->length = UINT32_MAX;
        sem_post(sems[i]);
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        waitpid(pids[i], NULL, 0);
    }

    for (int i = 0; i < NUM_PROCESSES; i++) {
        munmap(shm_data[i], SHM_SIZE);
        close(shm_fds[i]);
        shm_unlink(shm_names[i]);
        sem_close(sems[i]);
        sem_unlink(sem_names[i]);
    }

    return 0;
}