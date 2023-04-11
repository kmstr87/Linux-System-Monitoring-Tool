# Linux-System-Monitoring-Tool

This is a C program that will report different metrics of the utilization of a given system as described below, but implemented concurrently.

The program should accept several command line arguments:

- --system
  - to indicate that only the system usage should be generated

- --user
  - to indicate that only the users usage should be generated

- --graphics
  - to include graphical output in the cases where a graphical outcome is possible as indicated below.

- --sequential
  - to indicate that the information will be output sequentially without needing to "refresh" the screen 

- --samples=N
  - if used the value N will indicate how many times the statistics are going to be collected and results will be average and reported based on the N number of repetitions. (If no value is indicated, the default value will be 10.)

- --tdelay=T
  - to indicate how frequently to sample in seconds.(If no value is indicated the default value will be 1 sec.)

The last two arguments can also be considered as positional arguments if not flag is indicated in the corresponding order: samples tdelay.

The reported "stats" include:
  - **user usage**
    - report how many users are connected in a given time
    - report how many sessions each user is connected to
  - **system usage**
    - report how much utilization of the CPU is being done
    - report how much utilization of memory is being done (report used and free memory)
  - **Memory**  
    - Total memory is the actual physical RAM memory of the computer.
    - Virtual memory accounts for the physical memory and swap space together.
  - **Graphical representations**  
    - if the --graphics flag is used, generate a graphical representation showing the variation of memory used
    1. **For Memory utilization:**
      - :::@  total relative negative change
      - ###*  total relative positive change
    2. **For CPU utilization:**
      - ||||   positive percentage increase
      
The program also intercept signals coming from Ctrl-Z and Ctrl-C.
- For the former, it will just ignore it as the program should not be run in the background while running interactively.
- For the latter, the program will ask the user whether it wants to quit or not the program.

## Documentations of the functions
The overview of the functions are in the actual code itself, as I included the details of the functions
similar to Python docstrings.

## Overcoming Problems
Concurrency: I decided to go with the design where I fork three child processes every iteration to handle the sleep() call.
Every iteration, I create three child processes, and make each child process calculate the values for memory, users, and cpu utilization.
After the processes have finished its job, it writes to the parent process via pipe, and then it exits. When all three child processes have been terminated,
the parent reads the values and prints them out to the screen. Right before the program sleeps for -tdelay seconds, program checks to see if it had been interrupted
by ctrl+c or ctrl+z. It ignores ctrl+z completely, for when ctrl+c signal was given, it asks the user if they want to quit. If -y was given, program quits. If -n was given, it continues its process.

Whenever I ran into a problem, I first tried to find out what the problem was. If it was a simple syntax error,
problem solved! However, when I ran into a complex bug, I tried to find what tools I used, and read the documentations
on the tool. If it was hard to understand, I searched it up online to look for explanations. I played around with the tool
in a scratch c file, and then refactored the code after understanding more about the tool.

Ex: I had a hard time figuring out why my values weren't printing after the child processes writing to the pipe. It was because I
one of the variables I used for get the size of the string was declared in the child process, so it didn't exist in parent process.


## Formula explanations

- **Calculating # of lines to skip at the end (~ line 120):**
Add 2 to prevUserCount to consider barrier and title of user.
If graphics is given, mult. sample by two to consider extra number of lines printed for graphics.
Add 5 to samples to consider title and barrier of memory and CPU usage, and the printed output of CPU usage.
Mult. prevUserCount by user, mult. sample by system, and add the two together. This makes it so that if you
only want to print out user or system, only their section will be considered for skipping. 
Skip by the new sum, which will get you to the bottom of the printed output.

- **Total CPU Usage:**
Total cpu utilization time: Ti = useri + nicei + sysi + idlei + IOwaiti + irqi + softirqi.
Idle time: Ii = idlei., where i represents a sampling time; hence the CPU utilization at time 
is given by U_i = T_i - I_i.
Then, the CPU utilization will be given by, ((U_2 - U_1)/(T_2 - T_1)) * 100


## Running the Program

- First, compile the program by running cmd "make lab3". Then, to clean .o files, run "make clean". This should output lab3 file, where the program can be run
by cmd "./lab3"
- **Note:** Before running the program, make sure to clear the screen before. Otherwise, the formatting
        of the printed statments will be out of order. (In the terminal, run "clear")
        
- *Aside: for next example, the compiled code will be called prog1.*
- ~ prog1 will print the program info, system info, user info, memory info, and cpu info in order.
- ~ prog1 --tdelay=2 8 1 will run the program with 8 samples and 1 sec delay between
- ~ prog1 --user --system is equal to ~prog1

