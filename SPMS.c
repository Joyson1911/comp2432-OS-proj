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
    float timeDuration;// [X] e.g 3
    int endTime;// [XXXX] e.g 1300
    char priority; // [1],[2],[3],[4],[5]
    char essential1[30]; //[No], [Battery], [Lockers], [Inflation]
    char essential2[30]; //[No], [Cable], [Umbrella], [Valet]
    char essential3[30];
  } record;

//Input function
char GetPriority(char command[]);
record command2Record (char command[], int pipefd[2]);
char* readCommand ();
void readBatch(const char *batchName, int pipefd[2]);
//Scheduling funciton
void Scheduling(record data[], int sizeOfRecord, int method,record result[2][2000], int* acceptCounter, int* rejectCounter);
void FCFS(record rawData[], int sizeOfRecord, record result[2][2000], int* acceptCounter, int* rejectCounter);
void Priority(record rawData[], int sizeOfRecord, record result[2][2000], int* acceptCounter, int* rejectCounter);
bool isTimeCrashed(record* rawData, int timeSlot[7][24][7], int startDay);



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

  if (strcmp(returnRecord.name, "addBatch") == 0 || strcmp(returnRecord.name, "printBooking") == 0) {
      for (tempCounter = 0; arguments[1][tempCounter] != '\0'; tempCounter++) {
          if (arguments[1][tempCounter] == '-') continue;
          returnRecord.essential1[tempIndexCounter++] = arguments[1][tempCounter]; /*the name of batch file and algorithm is stored in essential1*/
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




  

bool isTimeCrashed(record* rawData, int timeSlot[7][24][7], int startDay){ 
  int time = rawData->startTime/100;  //1030 /100 = 10
  int day = rawData->date - startDay; 
  int start = rawData -> startTime /100 * 100; //remove the minutes
  int end = rawData -> endTime /100 * 100; //remove the minutes
  int duration = (end-start) / 100;
  if (rawData -> endTime % 100 != 0) duration++;

  bool onlyEssential = false;
  if (strcmp(rawData->name, "Essential") == 0) onlyEssential = true;
  printf("%s: duration is %d\n", rawData->name, duration);
  
  int i, j;
  const int numberOfEssential = 3;
  char essential[numberOfEssential][30];
  memset(essential,0,sizeof(essential)); //initialize the array
  strcpy(essential[0],rawData -> essential1);
  strcpy(essential[1],rawData -> essential2);
  strcpy(essential[2],rawData -> essential3);
  int types[3]; 
  

  for (i = 0; i<numberOfEssential;i++){
    if (strstr(essential[i], "No") != NULL) types[i] = -1; //-1 means no essential booking
    else if (strstr(essential[i], "Battery") != NULL) types[i] = 1;
    else if (strstr(essential[i], "Lockers") != NULL) types[i] = 2;
    else if (strstr(essential[i], "Inflation") != NULL) types[i] = 3;
    else if (strstr(essential[i], "Cable") != NULL) types[i] = 4;
    else if (strstr(essential[i],"Umbrella") != NULL) types[i] = 5;
    else if (strstr(essential[i],"Valet") != NULL) types[i] = 6;
  }

  for (i = 0;i< duration;i++){
    printf("%s loop %d %d\n", rawData->name, i, time+i);
    //checking the time (onlyEssential dont have to check the time)
    if (!onlyEssential && timeSlot[day][time+i][0] == 0){ 
      return true;
    }
    //checking the essential
    for (j = 0; j<numberOfEssential;j++){
      if (types[j] != -1 && timeSlot[day][time+i][types[j]] == 0){
        return true;
      }
    }
  
  }

    for(i = 0;i< duration;i++){
      if (!onlyEssential) timeSlot[day][time+i][0] -= 1; //minus parking slot by 1
      for (j=0;j<numberOfEssential;j++){
        if(types[j] != -1) timeSlot[day][time+i][types[j]] -= 1; //minus essential by 1
      }
    }
    return false;
  }

void Priority(record rawData[], int sizeOfRecord, record result[2][2000], int* acceptCounter, int* rejectCounter){
  //You can modify the variable here
      const int startDate = 20250510;
      const int endDate = 20250516;
      const int day = endDate - startDate + 1;
      const int hour = 24;
      const int numberOfParkingSlot = 10;
      const int numberOfBattery = 3;
      const int numberOfLockers = 3;
      const int numberOfInflation = 3;
      const int numberOfCable = 3;
      const int numberOfUmbrella = 3;
      const int numberOfValet = 3;
      const int typesOfEssentials = 6;
      const int numberOfPriority = 5;
      int i; 
      int j; 
      int timeSlot[day][hour][typesOfEssentials+1]; 
      /*[0]=time,[1]=essential(Battery) [2]=lockers [3]=inflation, [4]=Cable,[5]=Umbrella,[6]=Valet*/
      for(i = 0;i<day;i++){
        for (j = 0;j<hour;j++){
          timeSlot[i][j][0] = numberOfParkingSlot;
          timeSlot[i][j][1] = numberOfBattery; 
          timeSlot[i][j][2] = numberOfLockers; 
          timeSlot[i][j][3] = numberOfInflation; 
          timeSlot[i][j][4] = numberOfCable;
          timeSlot[i][j][5] = numberOfUmbrella;
          timeSlot[i][j][6] = numberOfValet;
        }
      }
  
      for (j = 0;j < numberOfPriority; j++){
        for (i = 0;i < sizeOfRecord; i++){
          int priority = (int) rawData[i].priority - 48;
          if (priority == j) {
            if (isTimeCrashed(&rawData[i], timeSlot, startDate)){
              result[0][*rejectCounter] = rawData[i]; //store the pointer to the record
              *rejectCounter++;
              printf("REJECT\n");
            }
            else{
              result[1][*acceptCounter] = rawData[i];
              *acceptCounter++;
              printf("ACCEPT\n");
            }
          }
        }
    }
  }

void FCFS(record rawData[], int sizeOfRecord, record result[2][2000],int* acceptCounter, int* rejectCounter){
  //You can modify the variable here
  const int startDate = 20250510;
  const int endDate = 20250516;
  const int day = endDate - startDate + 1;
  const int hour = 24;
  const int numberOfParkingSlot = 10;
  const int numberOfBattery = 3;
  const int numberOfLockers = 3;
  const int numberOfInflation = 3;
  const int numberOfCable = 3;
  const int numberOfUmbrella = 3;
  const int numberOfValet = 3;
  const int typesOfEssentials = 6;
  int i; 
  int j; 
  int timeSlot[day][hour][typesOfEssentials+1];
  /*[0]=time,[1]=essential(Battery) [2]=lockers [3]=inflation, [4]=Cable,[5]=Umbrella,[6]=Valet*/
  for(i = 0;i<day;i++){
    for (j = 0;j<hour;j++){
      timeSlot[i][j][0] = numberOfParkingSlot;
      timeSlot[i][j][1] = numberOfBattery; 
      timeSlot[i][j][2] = numberOfLockers; 
      timeSlot[i][j][3] = numberOfInflation; 
      timeSlot[i][j][4] = numberOfCable;
      timeSlot[i][j][5] = numberOfUmbrella;
      timeSlot[i][j][6] = numberOfValet;
    }
  }


  

  for (i = 0;i < sizeOfRecord; i++){
    if (isTimeCrashed(&rawData[i], timeSlot, startDate)){
      result[0][*rejectCounter] = rawData[i]; //store the pointer to the record
      *rejectCounter++;
      printf("REJECT\n");
    }
    else{
      result[1][*acceptCounter] = rawData[i];
      *acceptCounter++;
      printf("ACCEPT\n");
    }
  }
}

void Scheduling(record data[], int sizeOfRecord, int method,record result[2][2000], int* acceptCounter, int* rejectCounter){
  if (method == 0){ //FCFS
    FCFS(data,sizeOfRecord,result,acceptCounter,rejectCounter);
    return;
  }
  if (method == 1){ //Priority
    Priority(data,sizeOfRecord,result,acceptCounter,rejectCounter);
    return;
  }
  if (method == 2){ //optimal
    return;
  }
  if (method ==3){ //ALL
    return;
  }

}



int main(){
    int returnpid;
    const int numberOfModulue = 4;
    int childpid[numberOfModulue]; //4 module: Input, Scheduling, Output, Analysis
    int childID;
    int fd[numberOfModulue][2]; // array of pipeline for parent to children
    int cfd[numberOfModulue][2]; //array of pipeline for children to parent
    int i;
    int j;
    char buffer[100];
     //create pipeline
     for (i = 0;i < numberOfModulue; i++){
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
     for (i = 0; i < numberOfModulue;i++){
        if ((returnpid = fork()) < 0) {
                printf("Fork failed\n");
                exit(1);
        }
        if (returnpid == 0){   //child process
                for(j = 0; j<numberOfModulue; j++){
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
            record rawData[2000];
            record FCFSresult[2][2000]; //[0] = reject. [1] = accept 
            record Priorityresult[2][2000]; 
            int rejectCounter = 0;
            int acceptCounter = 0;
            int method;
            int sizeOfRecord;
            while (read(fd[childID][0], rawData, sizeof(rawData)) > 0){
                read(fd[childID][0], method, sizeof(method)); //method 0=FCFS, 1=Priority
                read(fd[childID][0], sizeOfRecord, sizeof(rawData));
                if (method == 0){ //FCFS
                  Scheduling(rawData,method,sizeOfRecord,FCFSresult, &acceptCounter, &rejectCounter);
                  write(fd[childID][0], FCFSresult, sizeof(record));
                  rejectCounter = 0;
                  acceptCounter = 0;
                  memset(FCFSresult, 0, sizeof(record));
                } 
                else if (method == 1){ //Priority
                  Scheduling(rawData,method,sizeOfRecord,Priorityresult, &acceptCounter, &rejectCounter);
                  write(fd[childID][0], Priorityresult, sizeof(record));
                  rejectCounter = 0;
                  acceptCounter = 0;
                  memset(Priorityresult, 0, sizeof(record));
                }
                else if (method == 2){

                }
                else if (method == 3){ //ALL
                  Scheduling(rawData,method,sizeOfRecord,FCFSresult, &acceptCounter, &rejectCounter);
                  write(fd[childID][0], FCFSresult, sizeof(record));
                  rejectCounter = 0;
                  acceptCounter = 0;
                  memset(FCFSresult, 0, sizeof(record));
                  Scheduling(rawData,method,sizeOfRecord,Priorityresult, &acceptCounter, &rejectCounter);
                  write(fd[childID][0], Priorityresult, sizeof(record));
                  rejectCounter = 0;
                  acceptCounter = 0;
                  memset(Priorityresult, 0, sizeof(record));
                }


            }
            close(fd[childID][0]);
            close(cfd[childID][1]);
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
              printf("Command name is %s\n", receiveRecord.name);
              printf("date is %d\n", receiveRecord.date);
              printf("member name is %c\n", receiveRecord.member);
              printf("start time is %d\n", receiveRecord.startTime);
              printf("Time duration is %1.1f\n", receiveRecord.timeDuration);
              printf("end time is %d\n", receiveRecord.endTime);
              printf("priority is %c\n", receiveRecord.priority);
              printf("essential1 is %s\n", receiveRecord.essential1);
              printf("essential2 is %s\n", receiveRecord.essential2);
              printf("essential3 is %s\n", receiveRecord.essential3);
              printf("\n");
              break;
          }

          // end the program 
          if (strcmp(receiveRecord.name, "endProgram") == 0) break;
      }
  }
}

