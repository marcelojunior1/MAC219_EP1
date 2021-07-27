#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "mpi.h"
#include <unistd.h>
#include <string.h>
#include <pthread.h>

void update_rgb_buffer(int iteration, int x, int y);


#define OK printf("OK\n");

int taskid;
int numtask;
int threads;

double c_x_min;
double c_x_max;
double c_y_min;
double c_y_max;

double pixel_width;
double pixel_height;

int iteration_max = 200;

int image_size;
unsigned char *image_buffer;

int i_x_max;
int i_y_max;

int i_y_ini;
int i_y_fim;
int qtd_linhas;

int image_buffer_size;

int gradient_size = 16;
int colors[17][3] = {
                        {66, 30, 15},
                        {25, 7, 26},
                        {9, 1, 47},
                        {4, 4, 73},
                        {0, 7, 100},
                        {12, 44, 138},
                        {24, 82, 177},
                        {57, 125, 209},
                        {134, 181, 229},
                        {211, 236, 248},
                        {241, 233, 191},
                        {248, 201, 95},
                        {255, 170, 0},
                        {204, 128, 0},
                        {153, 87, 0},
                        {106, 52, 3},
                        {16, 16, 16},
                    };

/* FUNCAO PTHREAD ####################################################*/

struct thread_data
{
    long tid;
    int ini;
    int fim;
};

int threads;

void* funcao_thread (void *t_data)
{
    struct thread_data *tdata;
    tdata = (struct thread_data*) t_data;
    double c_x;
    double c_y;
    double z_x;
    double z_y;
    double z_x_squared;
    double z_y_squared;
    double escape_radius_squared = 4;
    int iteration;

    int i_y;
    int i_x;

    int a = tdata->ini;
    int b = tdata->fim;
    int c = tdata->tid;

    for (i_y = tdata->ini; i_y <=tdata->fim; i_y++)
    {
        c_y = c_y_min + i_y * pixel_height;

        if (fabs(c_y) < pixel_height/2) c_y = 0.0;

        for(i_x = 0; i_x < i_x_max; i_x++)
        {
            c_x         = c_x_min + i_x * pixel_width;

            z_x         = 0.0;
            z_y         = 0.0;

            z_x_squared = 0.0;
            z_y_squared = 0.0;

            for(iteration = 0;
                iteration < iteration_max && \
                ((z_x_squared + z_y_squared) < escape_radius_squared);
                iteration++){
                z_y         = 2 * z_x * z_y + c_y;
                z_x         = z_x_squared - z_y_squared + c_x;

                z_x_squared = z_x * z_x;
                z_y_squared = z_y * z_y;
            };

            update_rgb_buffer(iteration, i_x, i_y);
        };

    }

    pthread_exit((void*) 0);
};


void allocate_image_buffer()
{

    int rgb_size = 3;
    image_buffer = malloc(sizeof(char) * image_buffer_size* rgb_size);

};

void init(int argc, char *argv[])
{
    c_x_min = -0.188;
    c_x_max = -0.012;
    c_y_min = 0.554;
    c_y_max = 0.754;
    image_size = 4096;
    

    qtd_linhas = 4096/numtask;

    i_y_ini           = taskid*qtd_linhas;
    i_y_fim           = i_y_ini + qtd_linhas; /* Inicio da proxima linha*/

    i_x_max           = image_size;
    i_y_max           = image_size;
    

    image_buffer_size = i_x_max * qtd_linhas;

    pixel_width       = (c_x_max - c_x_min) / i_x_max;
    pixel_height      = (c_y_max - c_y_min) / i_y_max;
};

void update_rgb_buffer(int iteration, int x, int y)
{
    int color;

    y = y - taskid*qtd_linhas;

    if(iteration == iteration_max)
    {
        image_buffer[((i_y_max * y) + x) * 3]       = colors[gradient_size][0];
        image_buffer[((i_y_max * y) + x) * 3 + 1]   = colors[gradient_size][1];
        image_buffer[((i_y_max * y) + x) * 3 + 2]   = colors[gradient_size][2];
    }
    else
    {
        color = iteration % gradient_size;

        image_buffer[((i_y_max * y) + x) * 3]       = colors[color][0];
        image_buffer[((i_y_max * y) + x) * 3 + 1]   = colors[color][1];
        image_buffer[((i_y_max * y) + x) * 3 + 2]   = colors[color][2];
    }
};

void write_to_file()
{
    char * filename               = "output.ppm";
    char * comment                = "# ";
    int max_color_component_value = 255;
    
    MPI_File     fh; 
    MPI_Request request;
    MPI_Datatype arraytype;
    MPI_Offset disp;
    MPI_Status status;
    int message;


    MPI_Type_contiguous(3, MPI_CHAR, &arraytype);
    MPI_Type_commit(&arraytype);


    int access_mode = MPI_MODE_CREATE /* Create the file if it does not exist */
                    | MPI_MODE_WRONLY; /* With write-only access */

    if(MPI_File_open(MPI_COMM_WORLD, "output.ppm", 
        access_mode, MPI_INFO_NULL, &fh) != MPI_SUCCESS)
    {
        printf("[MPI process %d] Failure in opening the file.\n", taskid);
    }

    unsigned int qtd = 3*image_buffer_size * taskid + 24;

    if (taskid == 0)
    {
        char str[50];
        sprintf(str, "P6\n %s\n %d\n %d\n %d\n", 
            comment, i_x_max, i_y_max, max_color_component_value);

        MPI_File_write_shared(fh, str, strlen(str), MPI_CHAR, MPI_STATUS_IGNORE);
    }

    MPI_File_set_view(fh, qtd, MPI_BYTE, MPI_UNSIGNED_CHAR, "native", MPI_INFO_NULL);

    MPI_File_iwrite_all (fh, image_buffer, image_buffer_size, arraytype, &request);
};

void compute_mandelbrot()
{
    void *status;
    int error_code;
    struct thread_data thread_array[threads+1];

    pthread_t thread[threads+1];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    int ini = i_y_ini;

    int step = (i_y_fim-i_y_ini)/threads;

    for(int i=1; i<=threads; i++)
    {
        thread_array[i].tid = i;
        thread_array[i].ini = ini;

        if (i == threads) thread_array[i].fim = i_y_fim -1;
        else
        {
            thread_array[i].fim = ini + step -1;
            ini += step ;
        }

        error_code = pthread_create(&thread[i], &attr, funcao_thread, &thread_array[i]);

        if (error_code)
        {
            printf("ERROR; return code from pthread_create() is %d\n", error_code);
            exit(-1);
        };
    }

    pthread_attr_destroy(&attr);

    for(int i=1; i<=threads; i++)
    {
        error_code = pthread_join(thread[i], &status);
        if (error_code)
        {
            printf("ERROR; return code from pthread_join() is %d\n", error_code);
            exit(-1);
        };
    };
};

double tempo (struct timespec inicio_thread, struct timespec fim_thread )
{
    double tempo_real;

    tempo_real = (double) (fim_thread.tv_sec - inicio_thread.tv_sec) + 1e-9 * (double) (fim_thread.tv_nsec - inicio_thread.tv_nsec);

    return tempo_real;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtask);

    threads = atoi(argv[1]);

    struct timespec t1, t2, t3, t4, t5;

    clock_gettime(CLOCK_MONOTONIC, &t1);
    
    init(argc, argv);

    clock_gettime(CLOCK_MONOTONIC, &t2);

    allocate_image_buffer();

    clock_gettime(CLOCK_MONOTONIC, &t3);

    compute_mandelbrot();

    clock_gettime(CLOCK_MONOTONIC, &t4);

    write_to_file();

    clock_gettime(CLOCK_MONOTONIC, &t5);

    printf("TEMPOS %d: %f %f %f\n",  taskid, tempo(t1, t2), tempo(t2, t3), tempo(t4, t5));

    MPI_Finalize();

    return 0;
};
