#include "merge_sort.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <array_size> <num_processes>\n", argv[0]);
        exit(1);
    }

    int array_size = atoi(argv[1]);
    int num_processes = atoi(argv[2]);

    fd = shm_open("/merge_sort_shm", O_CREAT | O_RDWR, 0700);
    ftruncate(fd, sizeof(SharedData));
    shared_data = mmap(0, sizeof(SharedData), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);


    mutex = sem_open("/merge_sort_sem", O_CREAT, 0700, 1);
    if (mutex == SEM_FAILED) {
        perror("Erreur d'initialisation du sémaphore");
        exit(1);
    }


    shared_data->array = mmap(NULL, array_size * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    shared_data->size = array_size;
    srand(time(NULL));
    for (int i = 0; i < array_size; i++) {
        shared_data->array[i] = rand() % MAX_NUM_SIZE;
    }

 
    gettimeofday(&start_time, NULL);

 
    execute_merge_sort(0, array_size - 1, num_processes);


    gettimeofday(&end_time, NULL);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("Temps d'exécution du tri parallèle : %.6f secondes\n", elapsed_time);

   // show_array();
    write_array_into_file();


    sem_close(mutex);
    sem_unlink(SEM_NAME); 
    munmap(shared_data->array, array_size * sizeof(int));
    close(fd);
    shm_unlink("/merge_sort_shm");

    return 0;
}

void execute_merge_sort(int start, int end, int num_processes) {
    if (num_processes <= 1 || end - start < 1) {
        sem_wait(mutex);         
        merge_sort(start, end);       
        sem_post(mutex);
    } else {
        int fd[2];
        pipe(fd);  

        pid_t pid = fork();
        if (pid == 0) { 
            close(fd[0]);  
            execute_merge_sort(start, (start + end) / 2, num_processes / 2);
            write(fd[1], &start, sizeof(int));  
            close(fd[1]);
            exit(0);
        } else { 
            close(fd[1]);  
            execute_merge_sort((start + end) / 2 + 1, end, num_processes / 2);
            wait(NULL);  

            sem_wait(mutex);           
            merge(start, (start + end) / 2, end);  
            sem_post(mutex);

            int child_start;
            read(fd[0], &child_start, sizeof(int));  
            close(fd[0]);
            printf("Fusion terminée pour la section : [%d, %d]\n", child_start, end);
        }
    }
}



void write_array_into_file() {
    FILE *file = fopen("sorted_array.txt", "w");
    fprintf(file, "Tableau trié: ");
    for (int i = 0; i < shared_data->size; i++) {
        fprintf(file, "%d ", shared_data->array[i]);
    }
    fprintf(file, "\n");
    fclose(file);
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
