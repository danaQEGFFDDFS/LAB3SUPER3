#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int provided;
    int process_Rank = 0;

    MPI_Init_thread(0, 0, MPI_THREAD_FUNNELED,  &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);

    if (process_Rank == 0) {
        if (argc < 2) {
            printf("Введите все параметры при запуске программы [размер] [имя файла]");
            exit(0);
        }

        FILE *f;
        f = fopen(argv[2],"r");
        if(!f) {
            printf("ERROR: Error open %s in mode %s \n", argv[2], "r");
            exit(0);
        }
        printf("INFO: File %s in mode %s successfully opened\n", argv[2], "r");

        int resInRows = (atoi(argv[1]) * 1024) / sqrt(3*atoi(argv[1]));

        int** matrix;
        int* arr;
        int** res_matrix;
        int flag = 0;

        arr = (int*) malloc(resInRows * sizeof(int));

        matrix = (int**) malloc(resInRows * sizeof(int*));
        for (int j = 0; j < resInRows; j++) {
            matrix[j] = (int*) malloc(resInRows * sizeof(int));
        }

        res_matrix = (int**) malloc(resInRows * sizeof(int*));
        for (int j = 0; j < resInRows; j++) {
            res_matrix[j] = (int*) malloc(resInRows * sizeof(int));
        }

        printf("imherer \n");

        int index_row = 0;
        int index_col = 0;

        char data[2];

        char *val = malloc(80);

        while(fgets(data, 2, f) != NULL) {

            if (flag == 0) {

                if (strcmp(data, " ") == 0) {
                    matrix[index_col][index_row] = atoi(val);
                    memset(val, 0, 80);
                    index_row++;
                    continue;
                }

                if (strcmp(data, "\n") == 0) {

                    if (index_row == 0) {
                        flag = 1;
                    } else {
                        matrix[index_col][index_row] = atoi(val);
                        index_row = 0;
                        index_col++;
                    }
                    memset(val, 0, 80);
                    continue;
                }

                if (strcmp(data, "\t") == 0) {
                    continue;
                }
                sprintf(val, "%s%s", val, data);
                continue;
            }

            if (strcmp(data, " ") == 0) {
                arr[index_row] = atoi(val);
                memset(val, 0, 80);
                index_row++;
                continue;
            }

            if (strcmp(data, "\n") == 0) {
                arr[index_row] = atoi(val);
                continue;
            }

            if (strcmp(data, "\t") == 0) {
                continue;
            }

            sprintf(val, "%s%s", val, data);

            bzero(data, 1);
        }

        fclose(f);
        f = fopen("res.txt","w+b");

        //count

        //prepare data

        int ** new_arr;

        new_arr = (int**) malloc((resInRows) * sizeof(int*));
        for (int j = 0; j < resInRows; j++) {
            new_arr[j] = (int*) malloc((resInRows+2) * sizeof(int));
        }

        for (int i = 0; i < resInRows; i++) {
            new_arr[i][0] = i;
            new_arr[i][1] = arr[i];
            for (int j = 2; j < resInRows + 2; j++) {
                new_arr[i][j] = matrix[i][j-1];
            }
        }

        int *response_arr;
        response_arr = (int*) malloc(resInRows * sizeof(int*) + 1);

        float res_time = 0;
        time_t start = clock();
        int data_counter = 0;

        int process_Rank, size_Of_Cluster, message_Item;
        MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
        MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);

        for (int i = 1; i < 4; i++) {
            MPI_Send(&resInRows, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            printf("INFO: Sended rows %d\n", i);
        }

        for (int i = 0; i < resInRows / 3; i++) {
            for (int j = 0 ; j < 3; j++) {
                MPI_Send(new_arr[i*3 + j], resInRows+2, MPI_INT, j+1, 0, MPI_COMM_WORLD);
            }

            for (int j = 0 ;j < 3; j++) {
                MPI_Recv(response_arr, resInRows+1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);
                for (int j = 1; j < resInRows+1; j++) {
                    res_matrix[response_arr[0]][j] = response_arr[j];
                }
            }
        }

        for (int i = resInRows - resInRows % 3; i < resInRows; i++) {
            MPI_Send(new_arr[i], resInRows+2, MPI_INT, i%3+1, 0, MPI_COMM_WORLD);
        }

        for (int i = resInRows - resInRows % 3; i < resInRows; i++) {
            MPI_Recv(response_arr, resInRows+1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            for (int j = 1; j < resInRows+1; j++) {
                res_matrix[response_arr[0]][j] = response_arr[j];
            }
        }

        printf("INFO: Sended last\n");

        time_t end = clock();
        res_time = ((float)(end - start) / 1000000.0F ) * 1000;
        printf("Count time: %f ms\n", res_time);
        for (int i = 0; i < resInRows;i++) {
            for (int j = 0; j < resInRows - 1; j++) {
                fprintf(f, "%d ", res_matrix[i][j]);
            }
            // array
            fprintf(f, "%d", res_matrix[i][resInRows - 1]);
            fprintf(f, "\n");
        }

        fprintf(f, "Count time: %f ms\n", res_time);
        fprintf(f, "Count of digits: %d ms\n", data_counter);
        fclose(f);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
    int* res_arr;
    int res_arr_size = 0;
    int* respones_arr;

    printf("%d \n", process_Rank);

    for (;;) {

        if (res_arr_size == 0) {
            MPI_Recv(&res_arr_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            res_arr = (int*) malloc((res_arr_size + 2) * sizeof(int));
            respones_arr = (int*) malloc((res_arr_size + 1) * sizeof(int));
        } else {
            MPI_Recv(res_arr, res_arr_size + 2, MPI_INT, 0, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            int res = 0;
            for(int jj = 2 ; jj < res_arr_size + 2; jj++) {
                respones_arr[jj-1] = res_arr[jj] * res_arr[2];
            }

            respones_arr[0] = res_arr[0];

            MPI_Send(respones_arr, res_arr_size + 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();

}