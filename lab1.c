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

// Declaring constants
const char *BARRIER = "---------------------------------------";
const float GIGABYTE = (float)1073741824;

// Declaring functions
void programInfo(int samples, int tdelay);
void sysInfo();
int users(int maxUser);
float sysMem(int graphics, int curIter, int samples, int sequential, double prevMemUsage);
void displayCpu(char prevCpu[], int graphics, int sequential,  int curIter, int samples);
void numCpu();

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

    // Refactoring user and system integers
    if (user + system == 0) {
        user = 1;
        system = 1;
    }

    // Run this one if sequential is not called
    if (sequential == 0) {
        programInfo(samples, tdelay);
        sysInfo();
    }
    
    for (int i = 0; i < samples; i++) {
        // save cursor position
        if (sequential == 0) printf("\033[s");
        // Print every iteration if sequential is called
        else {
            programInfo(samples, tdelay);
            sysInfo();
        }
        if (user == 1) {
            prevUserCount = users(prevUserCount);                
        }
        if (system == 1) {
            // Printing memory usage, number of CPUs, and CPU usage in order
            prevMemUsage = sysMem(graphics, i, samples, sequential, prevMemUsage);
            numCpu();
            displayCpu(prevCpu, graphics, sequential, i, samples);
        }
        if (sequential == 1) printf(">>> iteration %d\n", i + 1);
        //Restoring cursor position
        if (sequential == 0)printf("\033[u");
           
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
    
    return 0;
}

void programInfo(int samples, int tdelay) {
    /* This function takes in two integers, # of samples and the delay between each samples (in secs) 
    *  and prints out the the info. about the two mentioned above and the memory
    *  the current program is using. It does not return anything
    *
    *  Prereq: samples > 0, tdelay > 0 
    *
    *  Ex: programInfo(5, 2)
    * >> Nbr of samples: 5 -- every 2 secs
    * >>  Memory usage: 3198 kilobytes
    * >> --------------------------------
    */
    // Initializing rusage and calling getusage() to retrieve data about program RAM usage
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    // Printing out outputs
    printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);
    printf(" Memory usage: %d kilobytes\n", (int)usage.ru_maxrss);
    printf("%s\n", BARRIER);
}

void sysInfo() {
    /* This function doesn't take in any parameters, and prints out
    * information about the machine. It does not return anything.
    *
    * Prereq: none.
    *
    * Ex: sysInfo()
    * >> ### System Information ###
    * >>  System Name = Linux
    * >>  Machine Name = iits-b473-01
    * >>  Version = #99-Ubuntu SMP Thu Sep 23 17:29:00 UTC 2021
    * >>  Release = 5.4.0-88-generic
    * >>  Architecture = x86_64
    * >> ----------------------------------
    */
    // Initializing utsname and calling uname() to get values about the machines
    struct utsname un;
    uname (&un);
    // Printing out infos about the machine
    printf("### System Information ###\n");
    printf(" System Name = %s\n", un.sysname);
    printf(" Machine Name = %s\n", un.nodename);
    printf(" Version = %s\n", un.version);
    printf(" Release = %s\n", un.release);
    printf(" Architecture = %s\n", un.machine);
    printf("%s\n", BARRIER);
}

int users(int maxUser) {
    /* This function takes in the previous number of users currently
    *  on this machine last iteration and prints out informations about
    *  the users currently on. This function returns the number of users
    *  currently on.
    *
    *  Pre: maxUser >= 0
    * Ex: users(3) // currently there is four users on machine
    * >> marcelo       pts/0 (138.51.12.217)
    * >> marcelo       pts/1 (tmux(3773782).%0)
    * >> alberto       tty7 (:0)
    * >> marcelo       pts/2 (tmux(3773782).%1)
    */
    // Initializing utmp to get informations about each users
    struct utmp *user;
    int currentUserCount = 0;
    // Calling setutent() to rewind the file pointer to beginning of utmp file
    setutent();

    printf("### Sessions/users ###\n");
    
    // Loop if the line currently being read is not null
    while ((user = getutent()) != NULL) {
        // If the ut_type of the current line read is a normal process, print out Info about user
        if (user->ut_type == USER_PROCESS) {
            printf("%s       %s  (%s)\n", user->ut_user, user->ut_line, user->ut_host);
            currentUserCount++;
        }
    }

    // If there are less users now than during maximum user # height, delete the old information printed out
    if (currentUserCount > maxUser) maxUser = currentUserCount;
    if (maxUser > currentUserCount) {
        for (int i = 0; i < maxUser - currentUserCount; i++) {
            printf("\033[2K");
            printf("\n");
        }
    }
    
    // Close utmp file
    endutent();
    printf("%s\n", BARRIER);
    
    // Return maximum # of users on during the iterations.
    return maxUser;
}

void graphicsMem(double prevMemUsage, float phyUsedMem) {
    /* This is a helper function used to print out graphics for memory.
    * It takes in previously used memory in GB and physically used memory in GB.
    * If there is a total relative positive net change (phyUsedMem - prevUsedMem > 0), 
    * it prints out (# x (diff of the two x 10). Vice versa for total relative 
    * negative net change, but with ":". It does not return anything.
    *
    * Prereq: None.
    * 
    * Ex: graphicsMem(9.0, 9.03)
    * >>     |### (9.03)
    */

    // Printing out the setup
    printf("    |");

    // Calculating the value to be printed and the symbols to be used
    prevMemUsage = phyUsedMem - prevMemUsage;
    char sym[2] = "@";
    char repeat[2] = ":";
    if (prevMemUsage > 0) {
        strcpy(sym, "*");
        strcpy(repeat, "#");
    }

    // Calculating # of time for the symbols to be printed
    prevMemUsage = fabs(prevMemUsage);
    int temp = (int)(floor(prevMemUsage * 100));
    printf("%s", sym);
    for (int i = 0; i < temp; i++) {
        printf("%s", repeat);
    }
    // Printing the values
    printf(" %2.2f (%2.2f) \n", prevMemUsage, phyUsedMem);
}

float sysMem(int graphics, int curIter, int samples, int sequential, double prevUsedConv) {
    /* This function prints out the memory currently being used by this machine during "samples" iterations.
    * If sequential = 1, only print out one iteration at a time. If 0, print normally.
    * If graphics = 1, print out graphical interface as well. If 0, print normally.
    * This function returns the currently used memory in GB. If prevUsedConv = 0,
    * it takes the currently used physical memory in GB.
    *
    * Prereq: 0 <= graphics, sequential <= 1, 0 < samples, 0 <= curIter.
    *
    * Note: For details about how graphics work, see graphicsMem().
    *
    * Ex: float prev = 9.75; //currently 9.85 GB of physical memory used
    * prev = sysMem(1, 2, 10, 0, prev);
    * >> returns: 9.85
    * >> ### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)
    * >> 9.75 GB / 15.37 GB  -- 9.75 GB / 16.33 GB   | 0.00 (9.75)
    * >> 9.85 GB / 15.37 GB  -- 9.85 GB / 16.33 GB   |######### 0.09 (9.85)
    */
    // Initializing sysinfo
    struct sysinfo si;
    sysinfo (&si);
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
    
    // Getting infos about memory and converting them into GB
    float phyTotalMem = (float)si.totalram;
    float virTotalMem = phyTotalMem + ((float)si.totalswap);
    float phyTotalConv = phyTotalMem/GIGABYTE;
    float phyUsedConv = (phyTotalMem - (float)si.freeram)/GIGABYTE;
    float virTotalConv = virTotalMem/GIGABYTE;
    float virUsedConv = (virTotalMem - (float)si.freeswap - (float)si.freeram)/GIGABYTE;

    // Setting prev. phys. used mem. as current one if it was given as 0 in param.
    if (curIter == 0) prevUsedConv = phyUsedConv;
    
    // If no sequential
    if (sequential == 0) {
        // Skip curIter lines, and print the memory during current iter.
        if (curIter != 0) printf("\033[%dB", curIter);
        printf("%2.2f GB / %2.2f GB  --  %2.2f GB / %2.2f GB", phyUsedConv, phyTotalConv, virUsedConv, virTotalConv);
        // If no graphics, nothing happens
        if (graphics == 0) printf("\n");
        else {
            // Run graphicsMem() to format the graphics
            graphicsMem(prevUsedConv, phyUsedConv);
        }
        //Print out empty lines to format the display
        for (int i = 1; i < samples - curIter; i++) {
            printf("\n");
        }
    } else {
        // Print current Memory usage
        printf("%2.2f GB / %2.2f GB  --  %2.2f GB / %2.2f GB", phyUsedConv, phyTotalConv, virUsedConv, virTotalConv);
        // If graphics, format the graphics output
        if (graphics == 1) graphicsMem(prevUsedConv, phyUsedConv);
        else printf("\n");
    }
    
    printf("%s\n", BARRIER);
    // Return the currently used physical memory in GB
    return phyUsedConv;
}

void numCpu() {
    /* This function returns the # of CPUs in this machine. It doesn't
    * take in any parameters and doesn't return anything.
    *
    * Prereq: None.
    *
    * Ex: numCpu()
    * >> Number of CPUs: 12 
    */
    // Initializing variable and reading a file which contains number of CPUs
    int cpuNum = 0;
    char cpu[1024];
    FILE *fp = fopen("/proc/stat", "r");
    // If the file isn't read, print out the error
    if (fp == NULL) fprintf(stderr, "Error opening file\n");
    else {
        // Read the file line by line and keep track of # of times a line starts with cpu, after skipping first line
        fgets(cpu, sizeof(cpu), fp);
        while (strcmp(fgets(cpu, 4, fp), "cpu") == 0) {
            cpuNum++;
            fgets(cpu, 1000, fp);
        }
        fclose(fp);
        printf("Number of CPUs: %d\n", cpuNum);
    }
}

double sysCpu(char *prevCpu, char *curCpu) {
    /* This is a helper function which takes in the prev. and cur. iteration's information 
    *  and calculates the current CPU usage. It returns the usage.
    * 
    * Prereq: prevCpu and curCpu must be read from a /proc/stat file.
    *
    * Ex: sysCpu(prevCpu, curCpu) // two param. contains information read from /proc/stat file
    * >> returns 0.25.
    */
    // Declaring variables needed
    long int user, nice, system, idle, iowait, irq, softirq;
    long int user2, nice2, system2, idle2, iowait2, irq2, softirq2;
    long int idle_prev, idle_cur, cur_total, prev_total;

    // Getting the information needed from the prevCpu and curCpu
    sscanf(prevCpu, "cpu %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq);
    sscanf(curCpu, "cpu %ld %ld %ld %ld %ld %ld %ld", &user2, &nice2, &system2, &idle2, &iowait2, &irq2, &softirq2);

    // Calculating the idle time and total time used by the CPU prev. and currently
    idle_prev = idle + iowait;
    prev_total = idle_prev + user + nice + system + irq + softirq;
    idle_cur = idle2 + iowait2;
    cur_total = idle_cur + user2 + nice2 + system2 + irq2 + softirq2;
    // Calculating the diff. of total and idle time from previous and current iter.
    double totald = (double) cur_total - (double) prev_total;
    double idled = (double) idle_cur - (double) idle_prev;
    // If the total diff is 0, there is no diff. Return 0.
    if (totald == 0) return 0;
    // Calculating the total # of jiffies  system spent (see README for more info)
    double usage = (1000 * (totald - idled) / totald + 1) / 10;
    return usage;
}

void graphicsCpu(int numInc, double usage) {
    /* This is a helper function which prints out the graphics for current iteration of the
    * of the sample. It prints out "|" for each 0.1 inside usage. It does not return anything.
    *
    * Prereq: numInc = floor(usage * 10).
    *
    * Ex: graphicsCpu(2, 0.25);
    * >>            ||| 0.25
    */
    // Setting up the format
    printf("\033[2K");
    printf("         |");
    // Printing out the # of symbol corresp. to usage
    for (int i = 0; i < numInc; i++) {
        printf("%s", "|");
    }
    // Printing out the usage
    printf(" %2.2f  \n", usage);
}

void displayCpu(char prevCpu[], int graphics, int sequential,  int curIter, int samples) {
    /* This function is responsible for printing out the current CPU usage. The number should
    *  update every single iter. (samples). If graphics = 1, it prints out the graphics from start
    *  to the end of the sample.
    *  If sequential = 1, It prints out only the current iter. and prints out which iter. it's
    *  currently at. This function does not return anything.
    *
    *  Prereq: prevCpu must be read from /proc/stat. graphics, 0 <= sequential, graphics <= 1.
    *          curIter >= 0, samples > 0.
    */
    char curCpu[1024];
    double usage;

    // Reading the total CPU usage info from /proc/stat file
    FILE *fp = fopen("/proc/stat", "r");

    fgets(curCpu, sizeof(curCpu), fp);
    fclose(fp);

    // Calculating the usage using sysCpu(). Check sysCpu() for more info.
    usage = sysCpu(prevCpu, curCpu);      
    
    int numInc = floor(usage * 10);

    printf(" Total CPU Usage = %2.2f %c", usage, '%');
    // If sequential is called
    if (sequential == 1) {
        // Print current iter. info
        printf(",     Current Iter.: %d\n", curIter + 1);
        // If graphics is called, print the graphics of CPU usage
        if (graphics == 1) graphicsCpu(numInc, usage);
        
    } else {
        // Only print the total CPU usage
        printf("\n");
        // If only the graphics is called
        if (graphics == 1) {
            // Print the the prev. + cur. iter graphics.
            if (curIter != 0) printf("\033[%dB", curIter);
            graphicsCpu(numInc, usage);
        }
    }

    printf("%s\n", BARRIER);
    // Changing the prevCpu info with curCpu info
    strcpy(prevCpu, curCpu);
}
