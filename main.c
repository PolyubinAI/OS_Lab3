//
// Created by Ars Polybin on 01.12.2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

struct timeval tv1,tv2,dtv;
struct timezone tz;
void time_start() { gettimeofday(&tv1, &tz); }
long time_stop()
{ gettimeofday(&tv2, &tz);
    dtv.tv_sec= tv2.tv_sec -tv1.tv_sec;
    dtv.tv_usec=tv2.tv_usec-tv1.tv_usec;
    if(dtv.tv_usec<0) { dtv.tv_sec--; dtv.tv_usec+=1000000; }
    return dtv.tv_sec*1000+dtv.tv_usec/1000;
}

void PrintArr(FILE* file, int* arr, int size) {
    for (int i = 0; i < size; ++i) {
        fprintf(file, "%d ", arr[i]);
    }
}

int min_arr(int *a, int **b, int n){
    int min = 0;
    int index = -1;
    for (int i = 0; i < n; i++){
        if(a[i] == -1)
            continue;
        else if(index == -1){
            index = i;
            min = b[i][a[i]];
        }
        else if(b[i][a[i]] < min){
            min = b[i][a[i]];
            index = i;
        }
    }
    return index;
}

void MergeSort(int *a, int l, int r)
{
    if (l == r)
        return; // границы сомкнулись
    int mid = (l + r) / 2; // определяем середину последовательности
    // и рекурсивно вызываем функцию сортировки для каждой половины
    MergeSort(a, l, mid);
    MergeSort(a, mid + 1, r);
    int i = l;  // начало первого пути
    int j = mid + 1; // начало второго пути
    int *tmp = (int*)malloc(r * sizeof(int)); // дополнительный массив
    for (int step = 0; step < r - l + 1; step++) // для всех элементов дополнительного массива
    {
        // записываем в формируемую последовательность меньший из элементов двух путей
        // или остаток первого пути если j > r
        if ((j > r) || ((i <= mid) && (a[i] < a[j])))
        {
            tmp[step] = a[i];
            i++;
        }
        else
        {
            tmp[step] = a[j];
            j++;
        }
    }
    // переписываем сформированную последовательность в исходный массив
    for (int step = 0; step < r - l + 1; step++)
        a[l + step] = tmp[step];
}

typedef struct {
    int *array;
    int part_size;
} ThreadToken;


int IntCount(FILE* file) {
    int count = 0;
    int i = 0;
    while (fscanf(file, "%d", &i) > 0) {
        ++count;
    }
    rewind(file);
    return count;
}

void ReadToArr(FILE* file, int* arr, int size) {
    for (int i = 0; i < size; ++i) {
        fscanf(file, "%d", &arr[i]);
    }
}

void* ThreadFunc(void* token) {
    ThreadToken* tok = (ThreadToken*) token;
    MergeSort(tok->array, 0, tok->part_size-1);
    return NULL;
}

int main(int argc, char** argv) {

    if (argc < 2) {
        printf( "Error: enter input file name and count of threads.\n" );
        return 0;
    }
    FILE* input = fopen(argv[1], "r");
    FILE* output = stdout;
    int number_of_threads = 1;
    if(argc == 3) {
        number_of_threads = atoi(argv[2]);
        if(number_of_threads < 1){
            printf("Error: number of threads less than 1!");
            return -1;
        }
    }
    int size = IntCount(input);
    int* arr = (int*) malloc(sizeof(int) * size);
    int **arrays = (int**)malloc(sizeof(int*)*number_of_threads);

    ReadToArr(input, arr, size);

    if(size < number_of_threads){
        printf("Error! Number of threads is bigger than array size!\n");
        return 1;
    }
    int last_value = size % number_of_threads;

    ThreadToken* tokens = (ThreadToken*)malloc(sizeof(ThreadToken) * number_of_threads);

    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * number_of_threads);

    time_start();

    for (int i = 0; i < number_of_threads; ++i){
        if(i == number_of_threads - 1 && last_value !=0){
            arrays[i] = (int *) malloc((size / number_of_threads)+last_value * sizeof(int));
            tokens[i].array = arrays[i];
            tokens[i].part_size = (size / number_of_threads)+last_value;
        }
        else {
            arrays[i] = (int *) malloc(size / number_of_threads * sizeof(int));
            tokens[i].array = arrays[i];
            tokens[i].part_size = size / number_of_threads;
        }
        for (int j = 0; j < tokens[i].part_size; ++j){
            arrays[i][j] = arr[(i * (size / number_of_threads)) + j];
        }
    }

    for (int i = 0; i < number_of_threads; ++i) {
        if (pthread_create(&threads[i], NULL, ThreadFunc, &tokens[i])) {
            printf("Error creating thread!\n");
            return -1;
        }
    }
    for (int i = 0; i < number_of_threads; ++i){
        if (pthread_join(threads[i], NULL)) {
            printf("Error executing thread!\n");
            return -1;
        }
    }
    int* indexes = (int*)malloc(sizeof(int)*number_of_threads);
    for (int i = 0; i < number_of_threads; i++){
        indexes[i] = 0;
    }

    for (int i = 0; i < size; i++){
        int curr_ind = min_arr(indexes, arrays, number_of_threads);
        arr[i] = arrays[curr_ind][indexes[curr_ind]];
        indexes[curr_ind]++;
        if(indexes[curr_ind] >= tokens[curr_ind].part_size)
            indexes[curr_ind] = -1;
    }
    printf("\n");
    PrintArr(output, arr, size);

    printf("Time: %ld\n", time_stop());

    fclose(input);
    fclose(output);

    return 0;
}