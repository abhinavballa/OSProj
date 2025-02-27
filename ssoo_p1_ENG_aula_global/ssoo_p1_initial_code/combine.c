#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

struct alumno{
	char nombre[50];
	int nota;
	int convocatoria;
};




int compare_records(const void *record1, const void *record2) {
    struct alumno *stud1 = (struct alumno *)record1;
    struct alumno *stud2 = (struct alumno *)record2;
    return stud1->nota - stud2->nota;  //Sort by grade
}

int main(int argc, char *argv[]){
    //Validate argument count
    if (argc != 4) {
        return -1;
    }
    
    //Access input files
    int src_file1 = open(argv[1], O_RDONLY);
    if (src_file1 == -1) {
        return -1;
    }
    
    int src_file2 = open(argv[2], O_RDONLY);
    if (src_file2 == -1) {
        close(src_file1);
        return -1;
    }
    
    //Create output file
    int dest_file = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_file == -1) {
        close(src_file1);
        close(src_file2);
        return -1;
    }
    
    //Setup student collection
    struct alumno student_array[100];
    int total_students = 0;
    
    //Process first file
    struct alumno current_record;
    while (read(src_file1, &current_record, sizeof(struct alumno)) == sizeof(struct alumno)) {
        if (total_students >= 100) {
            printf("Error: Student limit of 100 exceeded\n");
            close(src_file1);
            close(src_file2);
            close(dest_file);
            return -1;
        }
        
        //Check for existing entry
        int exists = 0;
        for (int idx = 0; idx < total_students; idx++) {
            if (strcmp(student_array[idx].nombre, current_record.nombre) == 0) {
                exists = 1;
                break;
            }
        }
        
        if (!exists) {
            student_array[total_students++] = current_record;
        }
    }
    
    // Process second file
    while (read(src_file2, &current_record, sizeof(struct alumno)) == sizeof(struct alumno)) {
        if (total_students >= 100) {
            printf("Error: Student limit of 100 exceeded\n");
            close(src_file1);
            close(src_file2);
            close(dest_file);
            return -1;
        }
        
        // Check for existing entry
        int exists = 0;
        for (int idx = 0; idx < total_students; idx++) {
            if (strcmp(student_array[idx].nombre, current_record.nombre) == 0) {
                exists = 1;
                break;
            }
        }
        
        if (!exists) {
            student_array[total_students++] = current_record;
        }
    }
    
    // Sort the student records
    qsort(student_array, total_students, sizeof(struct alumno), compare_records);
    
    // Write to destination file
    write(dest_file, student_array, sizeof(struct alumno) * total_students);
    
    // Clean up file descriptors
    close(src_file1);
    close(src_file2);
    close(dest_file);
    
    // Create grade category counters
    int category_m = 0; // Perfect 10
    int category_s = 0; // Score 9
    int category_n = 0; // Scores 7-8
    int category_a = 0; // Scores 5-6
    int category_f = 0; // Below 5
    
    // Categorize each student
    for (int idx = 0; idx < total_students; idx++) {
        int score = student_array[idx].nota;
        if (score == 10) category_m++;
        else if (score == 9) category_s++;
        else if (score == 8 || score == 7) category_n++;
        else if (score == 6 || score == 5) category_a++;
        else category_f++;
    }
    
    // Generate statistics file
    FILE *stats = fopen("estadisticas.csv", "w");
    if (stats == NULL) {
        return -1;
    }
    
    // Write statistics data
    if (total_students > 0) {
        fprintf(stats, "M;%d;%.2f%%\n", category_m, (float)category_m * 100 / total_students);
        fprintf(stats, "S;%d;%.2f%%\n", category_s, (float)category_s * 100 / total_students);
        fprintf(stats, "N;%d;%.2f%%\n", category_n, (float)category_n * 100 / total_students);
        fprintf(stats, "A;%d;%.2f%%\n", category_a, (float)category_a * 100 / total_students);
        fprintf(stats, "F;%d;%.2f%%\n", category_f, (float)category_f * 100 / total_students);
    }
    
    fclose(stats);
    return 0;
}