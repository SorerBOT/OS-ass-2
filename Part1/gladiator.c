#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define FILE_EXTENSION ".txt"
#define LOG_FILE_EXTENSION "_log.txt"

#define MAX_LINE_LENGTH 1024
#define DELIMITERS ", "
#define ENEMIES_COUNT 3

typedef struct
{
    int health;
    int attack;
    int num;
} GladiatorStats;

typedef struct
{
    GladiatorStats stats;
    GladiatorStats* enemies[ENEMIES_COUNT];
    char* id;
} Gladiator;

void validate_ptr(const void* ptr, const char* debug_message);
Gladiator* init_glad(const char* gladId);
char* read_line(const FILE* file);
char* get_file_with_extension(const char* fileName, const char* extension);
char* get_gladiator_file(int gladiatorNum);
GladiatorStats* read_gladiator_stats(int gladiatorNum);
void play_round(Gladiator* glad);
void log_to_file(const char* gladId, const char *__restrict __format, ...);


int main(int argc, char** argv)
{
    char* gladId = "G1";
    Gladiator* glad = NULL;

/*     if (argc < 2)
        exit(1) */;

    // gladId = argv[1];
    glad = init_glad(gladId);

    while (glad->stats.health > 0)
    {
        play_round(glad);
    }

    log_to_file(glad->id, "The gladiator has fallen... Final health: %d\n", glad->stats.health);

    for (int i = 0; i < ENEMIES_COUNT; i++)
        free(glad->enemies[i]);

    free(glad->id);
    free(glad);
    exit(0);
}

Gladiator* init_glad(const char* gladId)
{
    /*
     * READ CURRENT GLADIATOR DATA
     */
    char* fileName = NULL;
    FILE* file = NULL;
    char* line = NULL;
    char* statsToken = NULL;
    Gladiator* gladiator = NULL;
    char* save_ptr = NULL;

    fileName = get_file_with_extension(gladId, FILE_EXTENSION);

    file = fopen(fileName, "r");
    validate_ptr(file, NULL);

    line = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));
    validate_ptr(line, NULL);
    fgets(line, MAX_LINE_LENGTH, file);
    fclose(file);

    gladiator = (Gladiator*)calloc(1, sizeof(Gladiator));
    validate_ptr(gladiator, NULL);

    // GETTING ID
    gladiator->id = strdup(gladId);
    gladiator->stats.num = -1;
    // GETTING HEALTH
    statsToken = strtok_r(line, DELIMITERS, &save_ptr);
    validate_ptr(statsToken, "Could not read gladiator health from file");
    gladiator->stats.health = atoi(statsToken);

    // GETTING ATTACK
    statsToken = strtok_r(NULL, DELIMITERS, &save_ptr);
    validate_ptr(statsToken, "Could not read gladiator attack from file");
    gladiator->stats.attack = atoi(statsToken);

    // GETTING FIRST ENEMY
    statsToken = strtok_r(NULL, DELIMITERS, &save_ptr);
    validate_ptr(statsToken, "Could not read first gladiator num from file");
    gladiator->enemies[0] = read_gladiator_stats( atoi(statsToken) );

    // GETTING SECOND ENEMY
    statsToken = strtok_r(NULL, DELIMITERS, &save_ptr);
    validate_ptr(statsToken, "Could not read second gladiator num from file");
    gladiator->enemies[1] = read_gladiator_stats( atoi(statsToken) );

    // GETTING THIRD ENEMY
    statsToken = strtok_r(NULL, DELIMITERS, &save_ptr);
    validate_ptr(statsToken, "Could not read second gladiator num from file");
    gladiator->enemies[2] = read_gladiator_stats( atoi(statsToken) );

    free(fileName);
    free(line);
    return gladiator;
}

void validate_ptr(const void* ptr, const char* debug_message)
{
    if (ptr != NULL) return;
    if (debug_message != NULL)
        printf("%s\n", debug_message);
    exit(1);
}

GladiatorStats* read_gladiator_stats(int gladiatorNum)
{
    FILE* file = NULL;
    char* line = NULL;
    char* statsToken = NULL;
    char* fileName = NULL;
    GladiatorStats* gladStats = NULL;
    char* save_ptr = NULL;

    fileName = get_gladiator_file(gladiatorNum);
    file = fopen(fileName, "r");
    validate_ptr(file, NULL);

    line = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));
    validate_ptr(line, NULL);
    fgets(line, MAX_LINE_LENGTH, file);
    fclose(file);

    gladStats = (GladiatorStats*)calloc(1, sizeof(GladiatorStats));
    gladStats->num = gladiatorNum;

    // GETTING HEALTH
    statsToken = strtok_r(line, DELIMITERS, &save_ptr);
    validate_ptr(statsToken, "Could not read gladiator health from file");
    gladStats->health = atoi(statsToken);

    // GETTING ATTACK
    statsToken = strtok_r(NULL, DELIMITERS, &save_ptr);
    validate_ptr(statsToken, "Could not read gladiator attack from file");
    gladStats->attack = atoi(statsToken);

    free(fileName);
    free(line);
    return gladStats;
}
char* get_gladiator_file(int gladiatorNum)
{
    char gladiatorId[3] = { 'G', 0, 0 };
    sprintf(&gladiatorId[1], "%d", gladiatorNum);
    return get_file_with_extension(gladiatorId, FILE_EXTENSION);
}

char* get_file_with_extension(const char* fileName, const char* extension)
{
    size_t totalLen = 0;
    char* fileNameWithExtension = NULL;

    totalLen = strlen(fileName) + strlen(extension) + 1;

    fileNameWithExtension = (char*)malloc(totalLen * sizeof(char));
    validate_ptr(fileNameWithExtension, NULL);

    strcpy(fileNameWithExtension, fileName);
    strcat(fileNameWithExtension, extension);

    return fileNameWithExtension;
}

void play_round(Gladiator* glad)
{
    GladiatorStats* enemy = NULL;

    for (int i = 0; i < ENEMIES_COUNT; i++)
    {
        enemy = glad->enemies[i];
        glad->stats.health -= enemy->attack;
        log_to_file(glad->id, "Facing opponent %d... Taking %d damage\n", enemy->num, enemy->attack);
        if (glad->stats.health <= 0) return;
        log_to_file(glad->id, "Are you not entertained? Remaining health: %d\n", glad->stats.health);
    }
}

void log_to_file(const char* gladId, const char *__restrict __format, ...)
{
    char* fileName = NULL;
    FILE* file = NULL;

    // GETTING FILE NAME
    fileName = get_file_with_extension(gladId, LOG_FILE_EXTENSION);
    validate_ptr(fileName, NULL);

    file = fopen(fileName, "a");
    validate_ptr(file, NULL);

    va_list args;
    va_start(args, __format);
    vfprintf(file, __format, args);
    va_end(args);

    fclose(file);
    free(fileName);
}
