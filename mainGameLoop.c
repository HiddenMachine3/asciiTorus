#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <string.h>

#define w 100
#define h 100
#define circle_radius 15
#define torus_radius 15
#define num_circle_points 60
#define num_torus_rings 60
const int num_torus_points = (num_circle_points * num_torus_rings);

char screen[w][h];
gsl_vector *circlepoints[num_circle_points];
gsl_vector *toruspoints[num_torus_points];

void start(), init(), loop(), update(), display(), cleanup();
void delay_millis(int milli_seconds);

#define frames 2000

/**** Math members ****/
gsl_matrix *YRotationMatrix, *XRotationMatrix, *ZRotationMatrix, *worldMatrix, *identityMatrix;

void update_rotation_matrix_x(gsl_matrix *rot_matrix, double angle);
void update_rotation_matrix_y(gsl_matrix *rot_matrix, double angle);
void update_rotation_matrix_z(gsl_matrix *rot_matrix, double angle);

/**
 gcc -Wall main.c -lgsl -lgslcblas -o "main.exe"
 */

int main()
{
    start();
    return 0;
}

void start()
{
    init();
    loop();
    cleanup();
}

void init()
{
    // init screen
    for (int x = 0, y; x < w; x++)
        for (y = 0; y < h; y++)
            screen[x][y] = ' ';
    screen[0][0] = screen[w - 1][0] = screen[0][h - 1] = screen[w - 1][h - 1] = '#';

    // init rotation matrices
    YRotationMatrix = gsl_matrix_alloc(3, 3);
    XRotationMatrix = gsl_matrix_alloc(3, 3);
    ZRotationMatrix = gsl_matrix_alloc(3, 3);
    worldMatrix = gsl_matrix_alloc(3, 3);
    identityMatrix = gsl_matrix_alloc(3, 3);

    gsl_matrix_set_identity(XRotationMatrix);
    gsl_matrix_set_identity(YRotationMatrix);
    gsl_matrix_set_identity(ZRotationMatrix);
    gsl_matrix_set_identity(worldMatrix);
    gsl_matrix_set_identity(identityMatrix);
    /*****init torus******/

    // make circle
    int i = 0;
    for (float t = 0, inc = 2 * M_PI / num_circle_points, x, y; t < 2 * M_PI; i++, t += inc)
    {
        x = circle_radius * cos(t);
        y = circle_radius * sin(t);

        circlepoints[i] = gsl_vector_alloc(3);
        gsl_vector_set(circlepoints[i], 0, x);
        gsl_vector_set(circlepoints[i], 1, y);
        gsl_vector_set(circlepoints[i], 2, 0); // setting z component to 0
    }

    // shift circle to the right
    gsl_vector *first_ring_midpoint = gsl_vector_alloc(3);
    memcpy(first_ring_midpoint->data, (double[]){circle_radius * 2, 0, 0}, sizeof(double) * 3);
    for (i = 0; i < num_circle_points; gsl_vector_add(circlepoints[i], first_ring_midpoint), i++)
        ;

    double ring_angle_inc = 2 * M_PI / num_torus_rings;
    for (int ring_number = 0; ring_number < num_torus_rings; ring_number++)
    {
        update_rotation_matrix_y(YRotationMatrix, ring_number * ring_angle_inc);

        for (i = 0; i < num_circle_points; i++) // we multiple each position vector of the circle by the rotation matrix, to "rotate" it about the origin
        {
            toruspoints[(ring_number * num_circle_points) + i] = gsl_vector_alloc(3);
            gsl_blas_dgemv(CblasNoTrans,                                        // no transpose
                           1.0,                                                 // alpha=1
                           YRotationMatrix,                                     // matrix
                           circlepoints[i],                                     // vector
                           0.0,                                                 // beta,
                           toruspoints[(ring_number * num_circle_points) + i]); // result
        }
    }

    // gsl_vector *torus_mid_point = gsl_vector_alloc(3);
    // memcpy(torus_mid_point->data, (double[]){w / 2, h / 2, 0}, sizeof(double) * 3);
    // for (i = 0; i < num_torus_points; gsl_vector_add(toruspoints[i], torus_mid_point), i++)
    //     ;
    // gsl_vector_free(torus_mid_point);

    gsl_vector_free(first_ring_midpoint);

    // setting up spinning
    update_rotation_matrix_x(XRotationMatrix, 4 * M_PI / 1500);
    update_rotation_matrix_y(YRotationMatrix, 4 * M_PI / 1500);
    update_rotation_matrix_z(ZRotationMatrix, 2 * M_PI / 1500);

    {
        gsl_matrix_set_identity(worldMatrix);
        gsl_matrix *original = gsl_matrix_alloc(3, 3);

        gsl_matrix_memcpy(original, worldMatrix);
        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, original, XRotationMatrix, 0, worldMatrix);

        gsl_matrix_memcpy(original, worldMatrix);
        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, original, YRotationMatrix, 0, worldMatrix);

        gsl_matrix_memcpy(original, worldMatrix);
        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, original, ZRotationMatrix, 0, worldMatrix);

        gsl_matrix_free(original);
    }
    // gsl_matrix_mul_elements(worldMatrix, YRotationMatrix);
    // gsl_matrix_mul_elements(worldMatrix, ZRotationMatrix);

    //  // where we want the middle of the torus
    //  torus_mid_point = gsl_vector_alloc(3);
    //  memcpy(torus_mid_point->data, (double[]){w / 2, h / 2, 0}, sizeof(double) * 3);
}

void loop()
{
    for (int i = 0; i < frames; i++)
    {
        update();
        display();
        // delay_millis(500);
    }
    printf("stopped");
}

int updateCounter = 0;
void update()
{
    // update_rotation_matrix_x(XRotationMatrix, 0);
    // update_rotation_matrix_y(YRotationMatrix, 0);
    // update_rotation_matrix_z(ZRotationMatrix, 0);

    for (int i = 0; i < num_torus_points; i++)
    {
        gsl_vector *original = gsl_vector_alloc(3);
        gsl_vector_memcpy(original, toruspoints[i]); // original <-- toruspoints[i]
        gsl_blas_dgemv(CblasNoTrans,                 // no transpose
                       1.0,                          // alpha=1
                       worldMatrix,                  // matrix
                       original,                     // vector
                       0.0,                          // beta,
                       toruspoints[i]);              // result
    }

    updateCounter++;
}

void display()
{
    // clearing the actual console
    system("clear");

    // clearing the screen characters array
    for (int y = 0, x; y < h; y++)
        for (x = 0; x < w; x++)
            screen[x][y] = ' ';

    // transfer the torus pixels onto the screen array
    // NOTE: We assume the middle of the screen is the origin
    for (int i = 0, x, y; i < num_torus_points; i++)
    {
        x = (gsl_vector_get(toruspoints[i], 0)) + w / 2;
        y = (gsl_vector_get(toruspoints[i], 1)) + h / 2;
        if (x >= 0 && y >= 0 && x < w && y < h)
            screen[x][y] = '#';
    }

    for (int y = 0, x; y < h; printf("\n"), y++)
        for (x = 0; x < w; x++)
            printf("%c ", screen[x][y]);
}

void cleanup()
{
    for (int i = 0; i < num_circle_points; gsl_vector_free(circlepoints[i]), i++)
        ;
    for (int i = 0; i < num_torus_points; gsl_vector_free(toruspoints[i]), i++)
        ;

    gsl_matrix_free(YRotationMatrix);
    gsl_matrix_free(XRotationMatrix);
    gsl_matrix_free(ZRotationMatrix);
    gsl_matrix_free(worldMatrix);
    gsl_matrix_free(identityMatrix);
    // gsl_vector_free(torus_mid_point);
}

void delay_millis(int milli_seconds)
{
    // Converting time into milli_seconds

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

/******************* MATH FUNCTIONS ******************/

void update_rotation_matrix_y(gsl_matrix *rot_mat, double angle)
{
    double c = cos(angle);
    double s = sin(angle);

    gsl_matrix_set(rot_mat, 0, 0, c);
    gsl_matrix_set(rot_mat, 0, 1, 0.0);
    gsl_matrix_set(rot_mat, 0, 2, s);

    gsl_matrix_set(rot_mat, 1, 0, 0.0);
    gsl_matrix_set(rot_mat, 1, 1, 1.0);
    gsl_matrix_set(rot_mat, 1, 2, 0.0);

    gsl_matrix_set(rot_mat, 2, 0, -s);
    gsl_matrix_set(rot_mat, 2, 1, 0.0);
    gsl_matrix_set(rot_mat, 2, 2, c);
}

void update_rotation_matrix_z(gsl_matrix *rot_matrix, double angle)
{
    double c = cos(angle);
    double s = sin(angle);

    gsl_matrix_set(rot_matrix, 0, 0, c);
    gsl_matrix_set(rot_matrix, 0, 1, -s);
    gsl_matrix_set(rot_matrix, 0, 2, 0.0);
    gsl_matrix_set(rot_matrix, 1, 0, s);
    gsl_matrix_set(rot_matrix, 1, 1, c);
    gsl_matrix_set(rot_matrix, 1, 2, 0.0);
    gsl_matrix_set(rot_matrix, 2, 0, 0.0);
    gsl_matrix_set(rot_matrix, 2, 1, 0.0);
    gsl_matrix_set(rot_matrix, 2, 2, 1.0);
}

void update_rotation_matrix_x(gsl_matrix *rot_matrix, double angle)
{
    double c = cos(angle);
    double s = sin(angle);

    gsl_matrix_set(rot_matrix, 0, 0, 1.0);
    gsl_matrix_set(rot_matrix, 0, 1, 0.0);
    gsl_matrix_set(rot_matrix, 0, 2, 0.0);
    gsl_matrix_set(rot_matrix, 1, 0, 0.0);
    gsl_matrix_set(rot_matrix, 1, 1, c);
    gsl_matrix_set(rot_matrix, 1, 2, -s);
    gsl_matrix_set(rot_matrix, 2, 0, 0.0);
    gsl_matrix_set(rot_matrix, 2, 1, s);
    gsl_matrix_set(rot_matrix, 2, 2, c);
}