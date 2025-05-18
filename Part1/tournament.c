#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_GLADIATORS 4

void create_glad_process(const char* gladId);

int main()
{
    char* gladiator_names[NUM_GLADIATORS] = {"Maximus", "Lucius", "Commodus", "Spartacus"};
    char* gladiator_files[NUM_GLADIATORS] = {"G1", "G2", "G3", "G4"};
    pid_t pids[NUM_GLADIATORS] = { 0, 0, 0, 0 };
    pid_t last_pid = -1;
    int lastGladIdx = -1;

    for (int i = 0; i < NUM_GLADIATORS; i++)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            exit(1);
        }
        else if (pid == 0)
        {
            execlp("./gladiator", "./gladiator", gladiator_files[i], NULL);
            exit(1);
        }
        else
        {
            pids[i] = pid;
        }
    }

    for (int i = 0; i < NUM_GLADIATORS; i++)
    {
        int status = -1;
        pid_t finishedPID = wait(&status);

        int exit_status = WEXITSTATUS(status);
        if (exit_status == EXIT_SUCCESS)
            last_pid = finishedPID;
        else
        {
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_GLADIATORS; i++)
    {
        if (last_pid == pids[i])
            lastGladIdx = i;
    }

    printf("The gods have spoken, the winner of the tournament is %s!\n", gladiator_names[lastGladIdx]);

    return 0;
}
