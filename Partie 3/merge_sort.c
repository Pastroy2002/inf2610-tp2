#include "merge_sort.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <array_size> <num_processes>\n", argv[0]);
        exit(1);
    }

    int array_size = atoi(argv[1]);
    int num_processes = atoi(argv[2]);

    
    fd = shm_open("/merge_sort_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedData));
    shared_data = mmap(0, sizeof(SharedData), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);

    
    sem_unlink(SEM_NAME);
    mutex = sem_open(SEM_NAME, O_CREAT, 0666, 1);

    
    mkfifo("/tmp/merge_pipe", 0666);
    int pipe_fd = open("/tmp/merge_pipe", O_RDWR);

    
    shared_data->array = mmap(NULL, array_size * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    shared_data->size = array_size;
    srand(time(NULL));
    for (int i = 0; i < array_size; i++) {
        shared_data->array[i] = rand() % MAX_NUM_SIZE;
    }

    
    gettimeofday(&start_time, NULL);

    
    execute_merge_sort(0, array_size - 1, num_processes, pipe_fd);

    
    gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("Temps d'exécution du tri parallèle : %.6f secondes\n", elapsed_time);

    
    show_array();
    write_array_into_file();

    
    sem_close(mutex);
    sem_unlink(SEM_NAME);
    close(pipe_fd);
    unlink("/tmp/merge_pipe");

    return 0;
}

void execute_merge_sort(int start, int end, int num_processes, int pipe_fd) {
    if (num_processes <= 1 || end - start < 1) {
        sem_wait(mutex);             
        merge_sort(start, end);
        sem_post(mutex);

        
        char message[100];
        sprintf(message, "Section triée de %d à %d terminée\n", start, end);
        write(pipe_fd, message, strlen(message));
    } else {
        pid_t pid = fork();
        if (pid == 0) { 
            execute_merge_sort(start, (start + end) / 2, num_processes / 2, pipe_fd);
            exit(0);
        } else { 
            execute_merge_sort((start + end) / 2 + 1, end, num_processes / 2, pipe_fd);
            wait(NULL); 

            sem_wait(mutex);          
            merge(start, (start + end) / 2, end);
            sem_post(mutex);

            
            char message[100];
            sprintf(message, "Fusion des sections de %d à %d terminée\n", start, end);
            write(pipe_fd, message, strlen(message));
        }
    }
}

void merge_sort( int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort(left, mid);
        merge_sort(mid + 1, right);
        merge(left, mid, right);
    }
}

void merge(int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int L[n1], R[n2];

    for (i = 0; i < n1; i++)
        L[i] = shared_data->array[left + i];
    for (j = 0; j < n2; j++)
        R[j] = shared_data->array[mid + 1 + j];

    i = 0;
    j = 0;
    k = left;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            shared_data->array[k] = L[i];
            i++;
        } else {
            shared_data->array[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        shared_data->array[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        shared_data->array[k] = R[j];
        j++;
        k++;
    }
}

void show_array(){
    printf("Sorted array: ");
    for (int i = 0; i < shared_data->size; i++) {
        printf("%d ", shared_data->array[i]);
    }
    printf("\n");
}