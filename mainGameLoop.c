#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <gsl/gsl_vector.h>
#include <signal.h>

#define w 20
#define h 10
#define circle_points 10

char screen[w][h];

void start(), init(), loop(), update(), display(), cleanup();
void delay_millis(int milli_seconds);

#define frames 1000
int stopFlag = 0;

void sigint_handler(int signum);

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
    signal(SIGINT, sigint_handler);
    init();
    loop();
    cleanup();
}

void init()
{
    // init screen
    for (int x = 0, y; x < w; x++)
        for (y = 0; y < h; y++)
            screen[x][y] = '#';

    // init torus
    // make circle

    for (float t = 0, inc = M_PI / circle_points; t < M_PI; t += inc)
    {
    }
}

void loop()
{
    for (int i = 0; i < frames && !stopFlag; i++)
    {
        update();
        display();
        delay_millis(500);
    }
    printf("stopped");
}

void update()
{
}

void display()
{
    system("clear");

    for (int y = 0, x; y < h; printf("\n"), y++)
        for (x = 0; x < w; x++)
            printf("%c", screen[x][y]);
}

void cleanup()
{
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

void sigint_handler(int signum)
{
    signal(SIGINT, sigint_handler);
    stopFlag = 1;
    fflush(stdout);
}