#include <stdio.h>
#include <stdbool.h>   
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
char memory[100][50];

typedef struct {
    char name[20]; //[Parking], [Essential], [Reservation], [Event]
    char member; //[X] e.g A
    int date; //[XXXXXXXX] e.g 20250510
    int startTime; // [XXXX] e.g 1000
    double timeDuration;// [X] e.g 3
    int endTime;// [XXXX] e.g 1300
    char priority; // [1],[2],[3],[4],[5]
    char essential1[30]; //[No], [Battery], [Lockers], [Inflation]
    char essential2[30]; //[No], [Cable], [Umbrella], [Valet]
    char essential3[30];
  } record;

char GetPriority(char command[]){
    if (strstr(command, "addParking") != NULL) return '3';
    else if (strstr(command, "addReservation") != NULL) return '2';
    else if (strstr(command, "addEvent") != NULL)  return '1';
    else if (strstr(command, "bookEssentials") != NULL)  return '4';
    else if (strstr(command, "printBookings") != NULL)  return '5';
    else return '0';
}

record command2Record (char command[], int pipefd[2]) {
    record returnRecord;

    char *separating_token;
    char arguments[8][15];
    int argIndex = 0;

    int tempCounter;
    int tempIndexCounter = 0;
    char tempDate[9];
    char tempStartTime[4];

    // re-initialize the returnRecord 
    memset(&returnRecord, 0, sizeof(returnRecord));
    strcpy(returnRecord.essential1, "NO");
    strcpy(returnRecord.essential2, "NO");
    strcpy(returnRecord.essential3, "NO");

    separating_token = strtok(command, " ");
    while (separating_token != NULL) {
        strcpy(arguments[argIndex++], separating_token);
        //printf("argument[%d] is %s\n", argIndex-1, arguments[argIndex-1]);
        separating_token = strtok(NULL, " ");
    }

    // return the name of command 
    strcpy(returnRecord.name, arguments[0]);

    if (strcmp(returnRecord.name, "addBatch") == 0) {
        for (tempCounter = 0; arguments[1][tempCounter] != '\0'; tempCounter++) {
            if (arguments[1][tempCounter] == '-') continue;
            returnRecord.essential1[tempIndexCounter++] = arguments[1][tempCounter]; /*the name of batch file is stored in essential1*/
        }
        return returnRecord;
    }

    // return the member name 
    returnRecord.member = arguments[1][strlen(arguments[1]) - 1];

    // return required date 
    memset(tempDate, 0, sizeof(tempDate)); // Clear tempDate
    tempIndexCounter = 0;
    for (tempCounter = 0; tempCounter < 10; tempCounter++) {
        if (arguments[2][tempCounter] == '-') continue;
        tempDate[tempIndexCounter++] = arguments[2][tempCounter];
    }
    returnRecord.date = atoi(tempDate);

    // return the start time
    memset(tempStartTime, 0, sizeof(tempStartTime)); // Clear tempStartTime
    tempIndexCounter = 0;
    for (tempCounter = 0; tempCounter < 5; tempCounter++) {
        if (arguments[3][tempCounter] == ':') continue;
        tempStartTime[tempIndexCounter++] = arguments[3][tempCounter];
    }
    returnRecord.startTime = atoi(tempStartTime);

    // cal the end time
    int endTimeHour = returnRecord.startTime / 100;
    int endTimeMin = returnRecord.startTime % 100;

    returnRecord.timeDuration = atof(arguments[4]);
    endTimeMin += returnRecord.timeDuration * 60;

    endTimeHour += endTimeMin / 60;
    endTimeMin %= 60;

    endTimeHour %= 24;

    // return the end time
    returnRecord.endTime = endTimeHour * 100 + endTimeMin;

    // return command priority
    returnRecord.priority = GetPriority(returnRecord.name);

    // retunrn the esstentials
    if (strcmp(arguments[5], "\0") != 0) strcpy(returnRecord.essential1, arguments[5]);
    if (strcmp(arguments[6], "\0") != 0) strcpy(returnRecord.essential2, arguments[6]);
    if (strcmp(arguments[7], "\0") != 0) strcpy(returnRecord.essential3, arguments[7]);

    write(pipefd[1], &returnRecord, sizeof(returnRecord));

    return returnRecord; 
}

char* readCommand () {
    const char delim []= ";";
    char *tempCommand;
    char command[100];
    char inputBuffer[100];

    printf("Please enter booking:\n");
    if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
        inputBuffer[strcspn(inputBuffer, "\n")] = '\0';
    }
    tempCommand= strtok(inputBuffer, delim);
    strcpy(command, tempCommand);

    return command;
}

void readBatch(const char *batchName, int pipefd[2]) {
    char command[100][100];
    const char delim[] = ";";
    char *tempCommand;
    int NumOfCommand = 0;
    char buffer[100];

    int tempCounter;

    FILE *file = fopen(batchName, "r");

    while (fgets(buffer, sizeof(buffer), file) != NULL) { // read the each lines in the batch file 
        buffer[strcspn(buffer, "\n")] = '\0';

        // gain each command
        tempCommand = strtok(buffer, delim);
        strcpy(command[NumOfCommand++], tempCommand);
        
        // turn each command to record and pass these records to parent
        command2Record(command[tempCounter++], pipefd);
    }
}





int main(){
    int returnpid;
    int childpid[4]; //4 module: Input, Scheduling, Output, Analysis
    int childID;
    int fd[4][2]; // array of pipeline for parent to children
    int cfd[4][2]; //array of pipeline for children to parent
    int i;
    int j;
    char buffer[100];
     //create pipeline
     for (i = 0;i < 4; i++){
        if (pipe(fd[i]) < 0) { //parent to children
                printf("Pipeline creation failed\n");
                exit(1);
        }
        if (pipe(cfd[i]) < 0) { //children to parent
                printf("Pipeline creation failed\n");
                exit(1);
        }
    }

    //create child
    for (i = 0; i < 4;i++){
    if ((returnpid = fork()) < 0) {
            printf("Fork failed\n");
            exit(1);
    }
    if (returnpid == 0){   //child process
            for(j = 0; j<4; j++){
                    if (j != i){ //close the unwanted pipeline
                            close(fd[j][0]);
                            close(fd[j][1]);
                            close(cfd[j][0]);
                            close(cfd[j][1]);
                    }
            }
            
            close(fd[i][1]); //child close the sender side
            close(cfd[i][0]); //child close the receiever side
            childID = i;
            break;
    }
    else{ //parent process
            close(fd[i][0]);
            close(cfd[i][1]);
            childpid[i] = returnpid;
    }
    }


    if (returnpid == 0){ //child
        if(childID == 0){ //input
            char receiveCommand[100];
            record newRecord;

            while (1) {
                strcpy(receiveCommand, readCommand());
                newRecord = command2Record(receiveCommand, cfd[childID]);
                if (strcmp(newRecord.name, "addBatch") == 0) readBatch(newRecord.essential1, cfd[childID]);
                
                if (strcmp(newRecord.name, "endProgram") == 0) {
                    close(fd[childID][0]);
                    close(cfd[childID][1]);
                    break;
                }
            }
        }
        if(childID == 1){ //Schdule

        }
        if(childID == 2){ //Output

        }
        if(childID == 3){ //Analysis

        }

        //close pipeline
        close(fd[childID][0]);
        close(cfd[childID][1]);
        exit(1);

    }
    
    else{ //parent
        record receiveRecord;
        printf("~~ WELCOME TO POLYU ~~\n");

        while (1) {
     
            // read the record from child 
            while (read(cfd[0][0], &receiveRecord, sizeof(receiveRecord)) > 0) {
                if (strcmp(receiveRecord.name, "endProgram") == 0) break;
                printf("Command name is %s|", receiveRecord.name);
                printf("date is %d|", receiveRecord.date);
                printf("member name is %c|", receiveRecord.member);
                printf("end time is %d|", receiveRecord.endTime);
                printf("priority is %c|", receiveRecord.priority);\
                printf("esstential1 is %s\n", receiveRecord.essential1);
                break;
            }

            // end the program 
            if (strcmp(receiveRecord.name, "endProgram") == 0) break;
        }
    }
}
