#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

struct alumno {
    char nombre[50];
    int nota;
    int convocatoria;
};


void selection_sort(struct alumno *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j].nota < arr[min_idx].nota) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            struct alumno temp = arr[i];
            arr[i] = arr[min_idx];
            arr[min_idx] = temp;
        }
    }
}

void int_str(int value, char *str) {
        
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    // make negatives positive
    int negative = 0;
    if (value < 0) {
        negative = 1;
        value = -value;
    }

    int temp = value;
    int length = 0;
    // Determine amount of space for str
    while (temp > 0) {
        temp /= 10;
        length++;
    }
    
    if (negative) {
        length++;
    }
    
    str[length] = '\0';
    int i = length - 1;
    
    while (value > 0) {
        str[i--] = '0' + (value % 10);
        value /= 10;
    }
    
    if (negative) {
        str[0] = '-';
    }
}

void add_line(int fd, char grade_letter, int count, int total) {
    char buffer[100];
    int pos = 0;
    
    // Add letter grade
    buffer[pos++] = grade_letter;
    buffer[pos++] = ';';
    
    // Add number of students in category
    char count_str[20];
    int_str(count, count_str);
    int i = 0;
    while (count_str[i] != '\0') {
        buffer[pos++] = count_str[i++];
    }
    buffer[pos++] = ';';
    
    // Calculate percentage by splitting it into integer part and decimal part
    int percentage_100 = count * 10000 / total;
    int int_part = percentage_100 / 100;
    int decimal_part = percentage_100 % 100;
    
    // Int part
    char num_str[20];
    int_str(int_part, num_str);
    i = 0;
    while (num_str[i] != '\0') {
        buffer[pos++] = num_str[i++];
    }
    buffer[pos++] = '.';
    // Dec part
    if (decimal_part < 10) {
        buffer[pos++] = '0';
    }
    char decimal_str[5];
    int_str(decimal_part, decimal_str);
    i = 0;
    while (decimal_str[i] != '\0') {
        buffer[pos++] = decimal_str[i++];
    }
    buffer[pos++] = '%';
    buffer[pos++] = '\n';
    
    write(fd, buffer, pos);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        return -1;
    }
    
    int fd_out = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        return -1;
    }
    
    int stats_fd = open("estadisticas.csv", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (stats_fd == -1) {
        close(fd_out);
        return -1;
    }
    
    // Then open input files
    int fd1 = open(argv[1], O_RDONLY);
    if (fd1 == -1) {
        close(fd_out);
        close(stats_fd);
        return -1;
    }
    
    int fd2 = open(argv[2], O_RDONLY);
    if (fd2 == -1) {
        close(fd1);
        close(fd_out);
        close(stats_fd);
        return -1;
    }
    
    struct alumno students[100];
    int student_count = 0;
    
    int current_fd = fd1;
    int file_idx = 0;
    
    while (file_idx < 2) {
        // Read current file
        struct alumno student;
        ssize_t bytes_read;
        
        while ((bytes_read = read(current_fd, &student, sizeof(struct alumno))) > 0) {
            if (bytes_read != sizeof(struct alumno)) {
                break; // incomplete
            }
            
            // Check for duplicates more efficiently
            int is_duplicate = 0;
            
            // Linear search for student name
            for (int i = 0; i < student_count; i++) {
                if (strcmp(students[i].nombre, student.nombre) == 0) {
                    is_duplicate = 1;
                    break;
                }
            }
            
            // Add if not a duplicate
            if (!is_duplicate) {
                if (student_count < 100) {
                    students[student_count++] = student;
                } else {
                    char error_msg[] = "Error: Maximum student count reached\n";
                    write(2, error_msg, strlen(error_msg));
                    
                    close(fd1);
                    close(fd2);
                    close(fd_out);
                    close(stats_fd);
                    return -1;
                }
            }
        }
        
        // Switch to the second file
        file_idx++;
        if (file_idx < 2) {
            current_fd = fd2;
        }
    }
    close(fd1);
    close(fd2);
    
    int grades_count[11] = {0}; // Indices 0-10 for grades 0-10
    for (int i = 0; i < student_count; i++) {
        int grade = students[i].nota;
        if (grade >= 0 && grade <= 10) {
            grades_count[grade]++;
        }
    }
    
    // sort students by grade
    selection_sort(students, student_count);
    
    // Write sorted students to output file
    write(fd_out, students, sizeof(struct alumno) * student_count);
    close(fd_out);
    
    int m_count = grades_count[10];                    // Score 10
    int s_count = grades_count[9];                     // Score 9
    int n_count = grades_count[7] + grades_count[8];   // Score 7-8
    int a_count = grades_count[5] + grades_count[6];   // Score 5-6
    int f_count = 0;                                   // Score < 5
    
    // Sum all F grades
    for (int i = 0; i < 5; i++) {
        f_count += grades_count[i];
    }
    
    if (student_count > 0) {
        add_line(stats_fd, 'M', m_count, student_count);
        add_line(stats_fd, 'S', s_count, student_count);
        add_line(stats_fd, 'N', n_count, student_count);
        add_line(stats_fd, 'A', a_count, student_count);
        add_line(stats_fd, 'F', f_count, student_count);
    }
    
    close(stats_fd);
    return 0;
}