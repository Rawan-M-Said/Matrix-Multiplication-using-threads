#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>

int **mat1;
int **mat2;
int row1;
int row2;
int col1;
int col2;
int **result;

void initialize_path(char *path, char *input1, char *input2){
    strcpy(input1, path);
    strcpy(input2, path);
}

void path_create(char *input, char *name){
    strcat(input, "/");
    strcat(input, name);
    strcat(input, ".txt");
}

void path_out_mat(char *output, char *name){
    strcat(output, "/");
    strcat(output, name);
    strcat(output, "_per_matrix.txt");
}

void path_out_row(char *output, char *name){
    strcat(output, "/");
    strcat(output, name);
    strcat(output, "_per_row.txt");
}

void path_out_element(char *output, char *name){
    strcat(output, "/");
    strcat(output, name);
    strcat(output, "_per_element.txt");
}

int** read_matrix(FILE *file, int *row, int *col) {
    char line[100];
    //int row, col;

    fgets(line, 100, file);
    if (sscanf(line, "row=%d col=%d", row, col) != 2) {
        printf("Error: Unable to read matrix dimensions.\n");
        return NULL;
    }

    // Allocate memory for the matrix
    int **matrix = (int **)malloc(*row * sizeof(int *));
    if (matrix == NULL) {
        printf("Error: Memory allocation failed.\n");
        return NULL;
    }
    for (int i = 0; i < *row; i++) {
        matrix[i] = (int *)malloc(*col * sizeof(int));
        if (matrix[i] == NULL) {
            printf("Error: Memory allocation failed.\n");
            // Free previously allocated memory to prevent memory leaks
            for (int j = 0; j < i; j++) {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
    }
    
    for (int i=0; i<*row; i++){
        fgets(line, 100, file);
        if (*col == 1){
            char *temp = strtok(line, "\n");
            matrix[i][0] = atoi(temp);
        }
        else {
            char *temp = strtok(line, " ");
            matrix[i][0] = atoi(temp);
            for (int j = 1; j < *col-1; j++){
                temp = strtok(NULL, " ");
                matrix[i][j] = atoi(temp);
            }
            temp = strtok(NULL, "\n");
            matrix[i][*col-1] = atoi(temp);
        }
    }

    return matrix;
}

int** per_matrix() {
    // Allocate memory for the matrix
    int *r1 = &row1;
    int *cn2 = &col2;
    int *r2 = &row2;
    int **matrix = (int **)malloc(*r1 * sizeof(int *));
    if (matrix == NULL) {
        printf("Error: Memory allocation failed.\n");
        return NULL;
    }
    for (int i = 0; i < *r1; i++) {
        matrix[i] = (int *)malloc(*cn2 * sizeof(int));
        if (matrix[i] == NULL) {
            printf("Error: Memory allocation failed.\n");
            // Free previously allocated memory to prevent memory leaks
            for (int j = 0; j < i; j++) {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
    }
    for (int i=0; i<*r1; i++){
        for (int j=0; j<*cn2; j++){
            matrix[i][j] = 0;
            for (int k=0; k<*r2; k++){
                matrix[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }
    return matrix;
}

void * row_calc(void *num) {
    int *n = (int *) num;
    for (int j=0; j<col2; j++){
        result[*n][j] = 0;
        for (int k=0; k<row2; k++){
            result[*n][j] += mat1[*n][k] * mat2[k][j];
        }
    }
    pthread_exit(NULL);
}

void per_row (){
    pthread_t threads[row1];
    int rc;
    int i;
    for (i=0; i<row1; i++){
        int *arg;
        arg = &i;
        rc = pthread_create(&threads[i], NULL, row_calc,(void *) arg);
        if (rc){
            printf("ERROR in creating threads.\n");
            exit(-1);
        }
    }
    printf("\tThe number of threads created = %d\n", row1);
    for (i=0; i<row1; i++){
        pthread_join(threads[i], NULL);
    }
    return;
}

void * element_calc (void *arg){
    int *dim = (int *) arg;
    result[dim[0]][dim[1]] = 0;
    for (int k=0; k<row2; k++){
        result[dim[0]][dim[1]] += mat1[dim[0]][k] * mat2[k][dim[1]];
    }
    pthread_exit(NULL);
}

void per_element() {
    pthread_t threads[row1 * col2];
    int rc;
    int i;
    int count = 0;
    for (i=0; i<row1; i++){
        for (int j=0; j<col2; j++){
            int *arg = (int *)malloc(2 * sizeof(int));
            arg[0] = i;
            arg[1] = j;
            rc = pthread_create(&threads[count++], NULL, element_calc,(void *) arg);
            if (rc){
                printf("ERROR in creating threads.\n");
                exit(-1);
            }
        }
    }
    printf("\tThe number of threads created = %d\n", row1*col2);
    for (count=0; i<row1 * col2; i++){
        pthread_join(threads[count], NULL);
    }
    return;
}

void write_matrix(FILE *file, int type, int **mat, int *row, int *col) {
    if (type == 1){   //per matrix
        fprintf(file, "Method: A thread per matrix\n");
    }
    else if (type == 2){  //per row
        fprintf(file, "Method: A thread per row\n");
    }
    else if (type == 3){  //per element
        fprintf(file, "Method: A thread per element\n");
    }
    fprintf(file, "row=%d col=%d\n", *row, *col);
    for (int i=0; i<*row; i++){
        for (int j=0; j<*col; j++){
            fprintf(file, "%d ", mat[i][j]);
        }
        fprintf(file, "\n");
    }
}

void free_matrix(int **matrix, int row){
    for (int i=0; i<row; i++){
        free(matrix[i]);
    }
    free(matrix);
}

int main() {
    printf("Example: ");
    char str[100];
    char input1[100];
    char input2[100];
    char outmat[100];
    char outrow[100];
    char outelement[100];

    fgets(str, sizeof(str), stdin);
    char *temp = NULL;
    temp = strtok(str, " ");
    if (temp != NULL){
        char path[50];
        path[0] = '\0';
        strcat(path, temp);
        initialize_path(path, input1, input2);
        initialize_path(path, outmat, outrow);
        initialize_path(path, outelement, outrow);

        FILE *m1;
        FILE *m2;
        FILE *out_per_mat;
        FILE *out_per_row;
        FILE *out_per_element;

        temp = strtok(NULL, " ");
        if (temp != NULL){
            path_create(input1, temp);
            m1 = fopen(input1, "r");
            if (m1 == NULL) {
                printf("Error: Unable to open file %s.\n", temp);
                return 1;
            }

            printf("Input files:\n\t%s.txt\n", temp);

            temp = strtok(NULL, " ");
            if (temp != NULL){
                path_create(input2, temp);
                m2 = fopen(input2, "r");
                if (m2 == NULL) {
                    printf("Error: Unable to open file %s.\n", temp);
                    return 1;
                }

                printf("\t%s.txt", temp);

                temp = strtok(NULL, "\n");
                if (temp != NULL){
                    path_out_mat(outmat, temp);
                    path_out_row(outrow, temp);
                    path_out_element(outelement, temp);
                    out_per_mat = fopen(outmat, "w");
                    out_per_row = fopen(outrow, "w");
                    out_per_element = fopen(outelement, "w");
                    if (out_per_mat == NULL || out_per_row == NULL || out_per_element == NULL) {
                        printf("Error: Unable to open the output files.\n");
                        return 1;
                    }
                    printf("Output files:\n\t%s_per_matrix.txt\n", temp);
                    printf("\t%s_per_row.txt\n\t%s_per_element.txt\n", temp, temp);
                }
                
                mat1 = read_matrix(m1, &row1, &col1);
                mat2 = read_matrix(m2, &row2, &col2);
                if (mat1 == NULL || mat2 == NULL){
                    fclose(m1);
                    fclose(m2);
                    return 0;
                }
                struct timeval stop, start;

                gettimeofday(&start, NULL); //start checking time
                result = per_matrix();
                gettimeofday(&stop, NULL); //end checking time
                printf("\nIn case of a thread per matrix:\n");
                printf("\tSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
                printf("\tMicroseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
                write_matrix(out_per_mat, 1, result, &row1, &col2);

                printf("\nIn case of a thread per row:\n");
                gettimeofday(&start, NULL); //start checking time
                per_row();
                gettimeofday(&stop, NULL); //end checking time
                printf("\tSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
                printf("\tMicroseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
                write_matrix(out_per_row, 2, result, &row1, &col2);

                printf("\nIn case of a thread per element:\n");
                gettimeofday(&start, NULL); //start checking time
                per_element();
                gettimeofday(&stop, NULL); //end checking time
                printf("\tSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
                printf("\tMicroseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
                write_matrix(out_per_element, 3, result, &row1, &col2);
                
                free_matrix(mat1, row1);
                free_matrix(mat2, row2);
                free_matrix(result, row1);
            }
        }
        else {
            temp = strtok(str, "\n");
            initialize_path(temp, input1, input2);
            initialize_path(temp, outmat, outrow);
            initialize_path(temp, outelement, outrow);
            strcat(input1, "/a.txt");
            strcat(input2, "/b.txt");
            
            m1 = fopen(input1, "r");
            if (m1 == NULL) {
                printf("Error: Unable to open file a1.\n");
                return 1;
            }

            m2 = fopen(input2, "r");
            if (m2 == NULL) {
                printf("Error: Unable to open file b.\n");
                return 1;
            }

            printf("Input files:\n\ta.txt\n\tb.txt\n");

            path_out_mat(outmat, "c");
            path_out_row(outrow, "c");
            path_out_element(outelement, "c");
            out_per_mat = fopen(outmat, "w");
            out_per_row = fopen(outrow, "w");
            out_per_element = fopen(outelement, "w");
            if (out_per_mat == NULL || out_per_row == NULL || out_per_element == NULL) {
                printf("Error: Unable to open/create the output files.\n");
                return 1;
            }
            
            printf("Output files:\n\tc_per_matrix.txt\n");
            printf("\tc_per_row.txt\n\tc_per_element.txt\n");

            mat1 = read_matrix(m1, &row1, &col1);
            mat2 = read_matrix(m2, &row2, &col2);
            if (mat1 == NULL || mat2 == NULL){
                fclose(m1);
                fclose(m2);
                return 0;
            }
            struct timeval stop, start;

            gettimeofday(&start, NULL); //start checking time
            result = per_matrix();
            gettimeofday(&stop, NULL); //end checking time
            printf("\nIn case of a thread per matrix:\n");
            printf("\tSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
            printf("\tMicroseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
            write_matrix(out_per_mat, 1, result, &row1, &col2);

            printf("\nIn case of a thread per row:\n");
            gettimeofday(&start, NULL); //start checking time
            per_row();
            gettimeofday(&stop, NULL); //end checking time
            printf("\tSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
            printf("\tMicroseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
            write_matrix(out_per_row, 2, result, &row1, &col2);

            printf("\nIn case of a thread per element:\n");
            gettimeofday(&start, NULL); //start checking time
            per_element();
            gettimeofday(&stop, NULL); //end checking time
            printf("\tSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
            printf("\tMicroseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
            write_matrix(out_per_element, 3, result, &row1, &col2);
            
            free_matrix(mat1, row1);
            free_matrix(mat2, row2);
            free_matrix(result, row1);
        }
        fclose(m1);
        fclose(m2);
        fclose(out_per_mat);
        fclose(out_per_row);
        fclose(out_per_element);
    }
}

//58 64
//139 154