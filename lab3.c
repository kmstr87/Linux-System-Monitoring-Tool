// Header file needed
#include "stats_functions.h"

int main(int argc, char *argv[]) {
    // Integers used as booleans
    int opt;
    int system = 0;
    int user = 0;
    int graphics = 0;
    int samples = 10;
    int tdelay = 1;
    int sequential = 0;
    
    // Declaring and initializing structs for my long flags
    struct option long_options[] = {
        {"system", 0, 0, 's'},
        {"user", 0, 0, 'u'},
        {"graphics", 0, 0, 'g'},
        {"sequential", 0, 0, 'q'},
        {"samples=", 2, 0, 'm'},
        {"tdelay=", 2, 0, 'd'},
        {0, 0, 0, 0}
    };

    // Getting each command line arguments
    while ((opt = getopt_long(argc, argv, "sugqm::d::", long_options, NULL)) != -1) {
        // For each flags, set its corresponding integer to 1 (true)
        switch(opt) {
            case 'g':
                graphics = 1;
                break;
            case 's':
                system = 1;
                break;
            case 'u':
                user = 1;
                break;
            case 'q':
                sequential = 1;
                break;
            case 'm':
                if (optarg) samples = atoi(optarg);
                break;
            case 'd':
                if(optarg) tdelay = atoi(optarg);
                break;
        }
    }
    
    int counter = 0;
    // Considering case for positional arguments for samples and tdelay
    for (int i = optind; i < argc; i++) {
        if (counter == 0) {
            samples = atoi(argv[i]);
            counter++;
        } else if (counter == 1) {
            tdelay = atoi(argv[i]);
        }
    }
    
    // Initializing previous user counts and previous CPU usage report
    int prevUserCount = 0;
    double prevMemUsage = 0;
    char prevCpu[1024];
    FILE *fp = fopen("/proc/stat", "r");
    fgets(prevCpu, sizeof(prevCpu), fp);
    fclose(fp);


    // Declaring needed string literals and buffers
    char *newMemoryOutput;
    char *newUserOutput;
    char memBuf[1024];
    char userBuf[4096];
    float newCpuOutput;
    int result;

    // Refactoring user and system integers
    if (user + system == 0) {
        user = 1;
        system = 1;
    }

    // Run this one if sequential is not called
    if (sequential == 0) {
        programInfo(samples, tdelay);
    }


    
    for (int i = 0; i < samples; i++) {
        // save cursor position
        if (sequential == 0) printf("\033[s");
        // Print every iteration if sequential is called
        else {
            programInfo(samples, tdelay);
        }


        // Initializing pipe for communication
        int pipes[2];
        // If pipe() fails, print to stderr
        if (pipe(pipes) == -1) {
          perror("pipe");
          exit(1);
        }
        // First fork for memory process
        if ((result = fork()) < 0) {
          perror("Fork Memory");
          exit(1);
        // Child process
        } else if (result == 0) {
          // If system flag is passed, get the value needed, pass it to parent, and exit
          if (system == 1) {
            close(pipes[0]);
            newMemoryOutput = calculateMemory(i, prevMemUsage, graphics);
            write(pipes[1], newMemoryOutput, strlen(newMemoryOutput));
            close(pipes[1]);
          }
          exit(0);
        // Parent process
        } else {
          // If system flag is passed, get the value needed from child and print it
          if (system == 1) {
            close(pipes[1]);
            read(pipes[0], memBuf, 1024);
          }
          close(pipes[0]);
        }

        // If pipe() fails, print to stderr
        if (pipe(pipes) == -1) {
          perror("pipe");
        }
        // Second fork for user process
        if ((result = fork()) < 0) {
          perror("Fork User");
        // Child process
        } else if (result == 0) {
          // If user flag is passed, get the value needed, pass it to parent, and exit
          if (user == 1) {
            close(pipes[0]);
            newUserOutput = calculateUsers();
            write(pipes[1], newUserOutput, strlen(newUserOutput));
            close(pipes[1]);
          }
          exit(0);
        // Parent process
        } else {
          // If user flag is passed, get the value needed from child and print it
          if (user == 1) {
            close(pipes[1]);
            read(pipes[0], userBuf, 4096);
          }
          close(pipes[0]);
        }

        // If pipe() fails, print to stderr
        if (pipe(pipes) == -1) {
          perror("pipe");
          exit(1);
        }
        // Third fork for cpu process
        if ((result = fork()) < 0) {
          perror("Fork CPU");
          exit(1);
        // Child process
        } else if (result == 0) {
          // If system flag is passed, get the value needed, pass it to parent, and exit
          if (system == 1) {
            close(pipes[0]);
            newCpuOutput = calculateCpu(prevCpu);
            write(pipes[1], &newCpuOutput, sizeof(float));
            close(pipes[1]);
          }
          exit(0);
        // Parent process
        } else {
          // If system flag is passed, get the value needed from child and print it
          if (system == 1) {
            close(pipes[1]);
            read(pipes[0], &newCpuOutput, sizeof(float));
          }
          close(pipes[0]);
        }

        
        if (system == 1) {
          prevMemUsage = sysMem(i, samples, sequential, memBuf);
        }
        if (user == 1) {
          prevUserCount = users(prevUserCount, userBuf);
        }
        if (system == 1) {
          numCpu();
          displayCpu(newCpuOutput, sequential, i, graphics, prevCpu);
        }

        // Set up signal handlers
        struct sigaction sa_int, sa_tstp;
        sa_int.sa_handler = handle_sigint;
        sa_tstp.sa_handler = handle_sigtstp;
        sigemptyset(&sa_int.sa_mask);
        sigemptyset(&sa_tstp.sa_mask);
        sa_int.sa_flags = 0;
        sa_tstp.sa_flags = 0;
        sigaction(SIGINT, &sa_int, NULL);
        sigaction(SIGTSTP, &sa_tstp, NULL);


        // If sequential is called, print the system info.
        if (sequential == 1) {
          sysInfo();
          printf(">>> iteration %d\n", i + 1);
        } else {
          //Restoring cursor position
          printf("\033[u");
        }

        // Wait tdelay seconds before iterating again
        sleep(tdelay);
    }
    
    // Calculating # of lines to skip over
    prevUserCount += 2;
    if (graphics == 1) samples += samples;
    samples += 5;
    int numLines = 0;

    numLines = user * prevUserCount + system * samples;
    // If --sequential is entered add one more to numLines
    if (sequential) numLines++;
    // Skipping to bottom of the outputs
    printf("\033[%dB", numLines);

    // Run this one if sequential is not called
    if (sequential == 0) {
      sysInfo();
    }
    
    return 0;
}