// Including the header file needed
#include "stats_functions.h"

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
    char *BARRIER = "---------------------------------------";
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
    char *BARRIER = "---------------------------------------";
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

char *calculateUsers() {
    /* This function calculates informations about
    *  the users currently on. This function returns the result
    *  in string.
    *
    * Ex: calculateUsers() // currently there is four users on machine
    * >> marcelo       pts/0 (138.51.12.217)
    * >> marcelo       pts/1 (tmux(3773782).%0)
    * >> alberto       tty7 (:0)
    * >> marcelo       pts/2 (tmux(3773782).%1)
    */
    // Initializing utmp to get informations about each users
    struct utmp *user;
    // Declaring needed buffers and string literal to be returned
    char *newUserOutput = malloc(4096);
    char tempBuff[4096];
    char lineStore[330];

    // Calling setutent() to rewind the file pointer to beginning of utmp file
    setutent();

    // Loop if the line currently being read is not null
    while ((user = getutent()) != NULL) {
        // If the ut_type of the current line read is a normal process, print out Info about user
        if (user->ut_type == USER_PROCESS) {
            // Store it in a line buffer and concat. that to a temp buffer
            sprintf(lineStore, "%s       %s  (%s)\n", user->ut_user, user->ut_line, user->ut_host);
            strcat(tempBuff, lineStore);
        }
    }

    // Copy temp buffer into a string literal since concat. to string literal is not possible
    strcpy(newUserOutput, tempBuff);
    
    // Close utmp file
    endutent();

    return newUserOutput;
}

int users(int maxUser, char newUserOutput[]) {
    /* This function takes in the previous number of users currently
    *  on this machine last iteration, info. about current users and 
    *  prints out informations about the users currently on. 
    *  This function returns the number of users currently on.
    *
    *  Pre: maxUser >= 0
    * Ex: users(3, newUserOutput) // currently there is four users on machine, and given string
    * >> marcelo       pts/0 (138.51.12.217)
    * >> marcelo       pts/1 (tmux(3773782).%0)
    * >> alberto       tty7 (:0)
    * >> marcelo       pts/2 (tmux(3773782).%1)
    */
    // Initializing needed counter and strings formatters.
    int currentUserCount = 0;
    char *BARRIER = "---------------------------------------";

    // Print the given result
    printf("### Sessions/users ###\n");
    printf("%s", newUserOutput);

    // Use strtok() to check the current user on this iter.
    char* token = strtok(newUserOutput, "\n");
    
    // Loop until reaching EOM
    while (token != NULL) {
        // Since one user is found, increase # of currentUserCount
        currentUserCount++;
        token = strtok(NULL, "\n");
    }
    
    // If there are less users now than during maximum user # height, delete the old information printed out
    if (currentUserCount > maxUser) maxUser = currentUserCount;
    if (maxUser > currentUserCount) {
        for (int i = 0; i < maxUser - currentUserCount; i++) {
            printf("\033[2K");
            printf("\n");
        }
    }
    
    // Printing the barrier and freeing the current iter. user info
    printf("%s\n", BARRIER);
    
    // Return maximum # of users on during the iterations.
    return maxUser;
}

// void graphicsMem(double prevMemUsage, float phyUsedMem) {
//     /* This is a helper function used to print out graphics for memory.
//     * It takes in previously used memory in GB and physically used memory in GB.
//     * If there is a total relative positive net change (phyUsedMem - prevUsedMem > 0), 
//     * it prints out (# x (diff of the two x 10). Vice versa for total relative 
//     * negative net change, but with ":". It does not return anything.
//     *
//     * Prereq: None.
//     * 
//     * Ex: graphicsMem(9.0, 9.03)
//     * >>     |### (9.03)
//     */

//     // Printing out the setup
//     printf("    |");

//     // Calculating the value to be printed and the symbols to be used
//     prevMemUsage = phyUsedMem - prevMemUsage;
//     char sym[2] = "@";
//     char repeat[2] = ":";
//     if (prevMemUsage > 0) {
//         strcpy(sym, "*");
//         strcpy(repeat, "#");
//     }

//     // Calculating # of time for the symbols to be printed
//     prevMemUsage = fabs(prevMemUsage);
//     int temp = (int)(floor(prevMemUsage * 100));
//     printf("%s", sym);
//     for (int i = 0; i < temp; i++) {
//         printf("%s", repeat);
//     }
//     // Printing the values
//     printf(" %2.2f (%2.2f) \n", prevMemUsage, phyUsedMem);
// }

char *calculateMemory(int curIter, double prevUsedConv, int graphics) {
    /* This function calculates memory currently being used by this machine and returns
    * the string which contains its result.
    * If graphics = 1, add graphical interface as well. If 0, return normally.
    * If there is a total relative positive net change (phyUsedMem - prevUsedMem > 0), 
    * it prints out (# x (diff of the two x 10). Vice versa for total relative 
    * negative net change, but with ":".
    * This function returns the currently used memory in GB. If prevUsedConv = 0,
    * it takes the currently used physical memory in GB.
    *
    * Prereq: 0 <= graphics, 0 <= curIter.
    *
    * Note: If net change is > 10, it will only give max. of 10 # or :.
    *
    * Ex: float prev = 9.75; //currently 9.85 GB of physical memory used
    * char *newMemoryOutput = calculateMemory(1, prev, 1);
    * >> 9.75 GB / 15.37 GB  -- 9.75 GB / 16.33 GB   | 0.00 (9.75)
    * >> 9.85 GB / 15.37 GB  -- 9.85 GB / 16.33 GB   |######### 0.09 (9.85)
    */
    // Initializing sysinfo and string literal
    struct sysinfo si;
    float GIGABYTE = (float)1073741824;
    sysinfo (&si);
    char *newMemoryOutput = malloc(1024);

    // Getting infos about memory and converting them into GB
    float phyTotalMem = (float)si.totalram;
    float virTotalMem = phyTotalMem + ((float)si.totalswap);
    float phyTotalConv = phyTotalMem/GIGABYTE;
    float phyUsedConv = (phyTotalMem - (float)si.freeram)/GIGABYTE;
    float virTotalConv = virTotalMem/GIGABYTE;
    float virUsedConv = (virTotalMem - (float)si.freeswap - (float)si.freeram)/GIGABYTE;

    // Setting prev. phys. used mem. as current one if it was given as 0 in param.
    if (curIter == 0) prevUsedConv = phyUsedConv;

    // If no graphics, nothing happens
    if (graphics == 0) {
        sprintf(newMemoryOutput, "%.2f GB / %.2f GB  --  %.2f GB / %.2f GB\n", phyUsedConv, phyTotalConv, virUsedConv, virTotalConv);
    } else {
         // Calculating the value to be printed and the symbols to be used
        float diffMem = phyUsedConv - prevUsedConv;
        char sym[11] = "@";
        char repeat[2] = ":";
        if (diffMem > 0) {
            strcpy(sym, "*");
            strcpy(repeat, "#");
        }

        // Calculating # of time for the symbols to be printed
        diffMem = fabs(diffMem);
        int temp = (int)(floor(diffMem * 100));

        for (int i = 0; i < temp; i++) {
            strcat(sym, repeat);
        }

        // Saving the result into string literal
        sprintf(newMemoryOutput, "%.2f GB / %.2f GB  --  %.2f GB / %.2f GB    |%s %.2f (%.2f)\n",
            phyUsedConv, phyTotalConv, virUsedConv, virTotalConv, sym, diffMem, phyUsedConv);
    }

    return newMemoryOutput;
}

float sysMem(int curIter, int samples, int sequential, char newMemoryOutput[]) {
    /* This function prints out the calculations in string given by newMemoryOutput.
    * If sequential = 1, only print out one iteration at a time. If 0, print normally.
    * This function returns the currently used memory in GB. If prevUsedConv = 0,
    * it takes the currently used physical memory in GB.
    *
    * Prereq: sequential <= 1, 0 < samples, 0 <= curIter.
    *
    * Ex: float prev = 9.75; //currently 9.85 GB of physical memory used
    * prev = sysMem(1, 2, 0, newMemoryOutput);
    * >> returns: 9.85
    * >> ### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)
    * >> 9.75 GB / 15.37 GB  -- 9.75 GB / 16.33 GB   | 0.00 (9.75)
    * >> 9.85 GB / 15.37 GB  -- 9.85 GB / 16.33 GB   |######### 0.09 (9.85)
    */
    // Initializing needed string formatters
    char *BARRIER = "---------------------------------------";
    float phyUsedConv;
    sscanf(newMemoryOutput, "%f", &phyUsedConv);
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
    
    // If no sequential
    if (sequential == 0) {
        // Skip curIter lines, and print the memory during current iter.
        if (curIter != 0) printf("\033[%dB", curIter);
        // Print current Memory usage
        printf("%s", newMemoryOutput);
       
        //Print out empty lines to format the display
        for (int i = 1; i < samples - curIter; i++) {
            printf("\n");
        }
    } else {
        // Print current Memory usage
        printf("%s", newMemoryOutput);
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

float calculateCpu(char *prevCpu) {
    /* This is a helper function which takes in the prev. and cur. iteration's information 
    *  and calculates the current CPU usage. It returns the usage.
    * 
    * Prereq: prevCpu and curCpu must be read from a /proc/stat file.
    *
    * Ex: sysCpu(prevCpu, curCpu) // two param. contains information read from /proc/stat file
    * >> returns 0.25.
    */
    // Declaring variables needed
    char curCpu[1024];
    
    // Reading the total CPU usage info from /proc/stat file
    FILE *fp = fopen("/proc/stat", "r");

    fgets(curCpu, sizeof(curCpu), fp);
    fclose(fp);

    long int user, nice, system, idle, iowait, irq, softirq;
    long int user2, nice2, system2, idle2, iowait2, irq2, softirq2;
    long int util_prev, util_cur, cur_total, prev_total;

    // Getting the information needed from the prevCpu and curCpu
    sscanf(prevCpu, "cpu %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &system, &idle, &iowait, &irq, &softirq);
    sscanf(curCpu, "cpu %ld %ld %ld %ld %ld %ld %ld", &user2, &nice2, &system2, &idle2, &iowait2, &irq2, &softirq2);

    // Calculating the idle time and total time used by the CPU prev. and currently
    prev_total = user + nice + system + idle + iowait + irq + softirq;
    cur_total = user2 + nice2 + system2 + idle2 + iowait2 + irq2 + softirq2;
    util_prev = prev_total - idle;
    util_cur = cur_total - idle2;

    // Calculating the diff. of total and idle time from previous and current iter.
    double totald = (double) cur_total - (double) prev_total;
    double utild = (double) util_cur - (double) util_prev;
    // If the util. diff is 0, there is no diff. Return 0.
    if (utild == 0) return 0;
    // Calculating the total # of jiffies  system spent (see README for more info)
    double usage = ((utild) / (totald)) * 100;

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
    printf(" %.2f  \n", usage);
}

void displayCpu(float newCpuOutput, int sequential,  int curIter, int graphics, char prevCpu[]) {
    /* This function is responsible for printing out the current CPU usage. The number should
    *  update every single iter. (samples). If graphics = 1, it prints out the graphics from start
    *  to the end of the sample.
    *  If sequential = 1, It prints out only the current iter. and prints out which iter. it's
    *  currently at. This function does not return anything.
    *
    *  Prereq: prevCpu must be read from /proc/stat. graphics, 0 <= sequential, graphics <= 1.
    *          curIter >= 0, samples > 0.
    */

    char *BARRIER = "---------------------------------------";

    int numInc = floor((newCpuOutput / 5)* 10);    

    printf(" Total CPU Usage = %.2f %c", newCpuOutput, '%');
    // If sequential is called
    if (sequential == 1) {
        // Print current iter. info
        printf(",     Current Iter.: %d\n", curIter + 1);
        // If graphics is called, print the graphics of CPU usage
        if (graphics == 1) graphicsCpu(numInc, newCpuOutput);
        
    } else {
        // Only print the total CPU usage
        printf("\n");
        // If only the graphics is called
        if (graphics == 1) {
            // Print the the prev. + cur. iter graphics.
            if (curIter != 0) printf("\033[%dB", curIter);
            graphicsCpu(numInc, newCpuOutput);
        }
    }

    // Declaring variables needed for next iter.
    char curCpu[1024];
    
    // Reading the total CPU usage info from /proc/stat file
    FILE *fp = fopen("/proc/stat", "r");

    // Reading the current cpu data and closing it
    fgets(curCpu, sizeof(curCpu), fp);
    fclose(fp);

    // Changing the prevCpu info with curCpu info
    strcpy(prevCpu, curCpu);

    printf("%s\n", BARRIER);
}

void handle_sigint(int sig) {
    /* This function is responsible for handling the signal for ctrl + c.
    *  When ctrl + c is called during any time of the process, user will
    *  be prompt to ask if one wants to quit the process or continue.
    *  If 'y' is pressed, program will quit. If 'n' is pressed, program will
    *  continue as before.
    */

    char answer;
    printf("\nReceived signal %d (Ctrl+C)\n", sig);
    printf("Do you want to quit? (y/n) ");
    fflush(stdout);  // Make sure the message is printed immediately
    do {
        answer = getchar();
    } while (answer != 'y' && answer != 'n');
    if (answer == 'y') {
        exit(0);
    }
    printf("Continuing...\n");
}

void handle_sigtstp(int sig) {
    /* This function is responsible for handling the signal for ctrl + z.
    *  It ignores the signal and processes the program regularly.
    */
    // Ignore the signal
}