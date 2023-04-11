// Including all the external libraries needed
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <utmp.h>
#include<unistd.h>
#include <sys/resource.h>
#include <getopt.h>
#include <math.h>
#include <signal.h>

// Declaring functions
void programInfo(int samples, int tdelay);
void sysInfo();
char *calculateUsers();
int users(int maxUser, char newUserOutput[]);
char *calculateMemory(int curIter, double prevUsedConv, int graphics);
float sysMem(int curIter, int samples, int sequential, char newMemoryOutput[]);
float calculateCpu(char *prevCpu);
void displayCpu(float newCpuOutput, int sequential,  int curIter, int graphics, char prevCpu[]);
void numCpu();
void handle_sigtstp(int sig);
void handle_sigint(int sig);