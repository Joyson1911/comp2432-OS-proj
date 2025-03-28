#include <stdio.h>
#include <stdbool.h>   
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

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
    char essential4[30];
    char essential5[30];
    char essential6[30];
  } record;

  typedef struct {
    int timeslot[6][7][24];
    int rejectCounter;
    int acceptCounter;
    int invalidCounter;
} analysisBlock;

          typedef struct {
            record result[2][2000]; //0 is reject, 1 is accept
            int rejectCounter;
            int acceptCounter;
            char method[100];
          } outputBlock;

//Input function
char GetPriority(char command[]);
record command2Record (char command[], int pipefd[2]);
void readCommand (char *command);
void readBatch(const char *batchName, int pipefd[2]);
//Scheduling funciton
void Scheduling(record data[],int sizeOfRecord,int method,record result[2][2000],int* acceptCounter,int* rejectCounter,int analysisInfo[6][7][24]);
void FCFS(record rawData[], int sizeOfRecord, record result[2][2000], int* acceptCounter, int* rejectCounter, int analysisInfo[6][7][24]);
void Priority(record rawData[], int sizeOfRecord, record result[2][2000],int* acceptCounter, int* rejectCounter, int analysisInfo[6][7][24]);
bool isTimeCrashed(record* rawData, int timeSlot[7][24][7], int startDay);
//Output function
//Schdeuling function
void printSummary(analysisBlock data[2]);



char GetPriority(char command[]){
  if (strstr(command, "addParking") != NULL) return '3';
  else if (strstr(command, "addReservation") != NULL) return '2';
  else if (strstr(command, "addEvent") != NULL)  return '1';
  else if (strstr(command, "bookEssentials") != NULL)  return '4';
  else if (strstr(command, "printBookings") != NULL)  return '5';
  else return '0';
  }  

  record command2Record (char* command, int pipefd[2]) {
    record returnRecord;
    int invalidFlag = 0;
  
    char *separating_token;
    char arguments[8][15];
    int argIndex = 0;
  
    int tempCounter;
    int tempIndexCounter = 0;
    char tempDate[9];
    char tempStartTime[4];
  
    char essential_list[6][30];
  
    // re-initialize the returnRecord 
    memset(&returnRecord, 0, sizeof(returnRecord));
    for (tempCounter = 0; tempCounter < 6; tempCounter++) {
      strcpy(essential_list[tempCounter], "NO");
    }
  
    separating_token = strtok(command, " ");
    while (separating_token != NULL) {
        strcpy(arguments[argIndex++], separating_token);
        separating_token = strtok(NULL, " ");
    }
  
    // return the name of command 
    strcpy(returnRecord.name, arguments[0]);
  
    // return command priority
    returnRecord.priority = GetPriority(returnRecord.name);
  
    if (strcmp(returnRecord.name, "addBatch") == 0) {
      for (tempCounter = 0; arguments[1][tempCounter] != '\0'; tempCounter++) {
        if (arguments[1][tempCounter] == '-') continue;
        returnRecord.essential1[tempIndexCounter++] = arguments[1][tempCounter]; /*the name of batch file is stored in essential1*/
      }
      return returnRecord;
    }
  
    if (strcmp(returnRecord.name, "printBookings") == 0) {
      for (tempCounter = 0; arguments[1][tempCounter] != '\0'; tempCounter++) {
        if (arguments[1][tempCounter] == '-') continue;
        returnRecord.essential1[tempIndexCounter++] = arguments[1][tempCounter]; /*the name of algorithms is stored in essential1*/
      }

      if (strcmp(returnRecord.essential1, "fcfs") == 0 || strcmp(returnRecord.essential1, "prio") == 0 || strcmp(returnRecord.essential1, "ALL") == 0) {
        write(pipefd[1], &returnRecord, sizeof(returnRecord));
        return returnRecord;
      }


      else {
        printf("Invalid algorithm: %s\n", returnRecord.essential1);
        invalidFlag = 1;
      }


    }  
  
    // return the member name 
    returnRecord.member = arguments[1][strlen(arguments[1]) - 1];
    if ((returnRecord.member < 'A' || returnRecord.member > 'E') && returnRecord.member != 0 && invalidFlag == 0) {
      printf("Invalid member: %s\n", arguments[1]);
      invalidFlag = 1;
    }
  
    
    // return required date 
    memset(tempDate, 0, sizeof(tempDate)); // Clear tempDate
    tempIndexCounter = 0;
    for (tempCounter = 0; tempCounter < strlen(arguments[2]); tempCounter++) {
      if (arguments[2][tempCounter] == '-') continue;
      tempDate[tempIndexCounter++] = arguments[2][tempCounter];
    }
    tempDate[tempIndexCounter] = '\0';
    returnRecord.date = atoi(tempDate);
    if ((returnRecord.date < 20250510 || returnRecord.date > 20250516) && returnRecord.date != 0) {
      printf("Invalid date: %s\n", arguments[2]);
      invalidFlag = 1;
    }
  
    // return the start time
    memset(tempStartTime, 0, sizeof(tempStartTime)); // Clear tempStartTime
    tempIndexCounter = 0;
    for (tempCounter = 0; tempCounter < strlen(arguments[3]); tempCounter++) {
      if (arguments[3][tempCounter] == ':') continue;
      tempStartTime[tempIndexCounter++] = arguments[3][tempCounter];
    }
    tempStartTime[tempIndexCounter] = '\0';
    returnRecord.startTime = atoi(tempStartTime);
    if (returnRecord.startTime < 0 || returnRecord.startTime > 2359) {
      printf("Invalid start time: %s\n", arguments[3]);
      invalidFlag = 1;
    }
  
    // cal the end time
    returnRecord.timeDuration = atof(arguments[4]);
    if (returnRecord.timeDuration < 0.0 || returnRecord.timeDuration > 12.0) {
      printf("Invalid time duration: %s\n", arguments[4]);
      invalidFlag = 1;
    }
    int endTimeHour = returnRecord.startTime / 100;
    int endTimeMin = returnRecord.startTime % 100;
    endTimeMin += returnRecord.timeDuration * 60;
    endTimeHour += endTimeMin / 60;
    endTimeMin %= 60;
    endTimeHour %= 24;
  
    // return the end time
    returnRecord.endTime = endTimeHour * 100 + endTimeMin;
  
    tempIndexCounter = 0;
    int z;
    for (tempCounter = 5; tempCounter < argIndex; tempCounter++) {
      int matchFound = 0;
      for (z = 0; z < 6; z++) {
        if (strcmp(essential_list[z], arguments[tempCounter]) == 0) {
          matchFound = 1;
          break; // Stop searching if a match is found
        }
      }
      if (matchFound == 1) continue;
  
      if (strcmp(arguments[tempCounter], "battery") == 0 || strcmp(arguments[tempCounter], "cable") == 0) {
        strcpy(essential_list[tempIndexCounter++], "battery");
        strcpy(essential_list[tempIndexCounter++], "cable");
      }
      else if (strcmp(arguments[tempCounter], "locker") == 0 || strcmp(arguments[tempCounter], "umbrella") == 0) {
        strcpy(essential_list[tempIndexCounter++], "locker");
        strcpy(essential_list[tempIndexCounter++], "umbrella");
      }
      else if (strcmp(arguments[tempCounter], "InflationService") == 0 || strcmp(arguments[tempCounter], "valetpark") == 0) {
        strcpy(essential_list[tempIndexCounter++], "InflationService");
        strcpy(essential_list[tempIndexCounter++], "valetpark");
      }
      else {
        printf("Invalid essential: %s\n", arguments[tempCounter]);
        invalidFlag = 1;
      }
    }
  
    // retunrn the esstentials
    for (tempCounter = 0; tempCounter < 6; tempCounter++) {
      strcpy((&returnRecord.essential1)[tempCounter], essential_list[tempCounter]);
    }
  
    if (invalidFlag == 1) {
      memset(&returnRecord, 0, sizeof(returnRecord));
      strcpy(returnRecord.essential1, "Invalid"); /* if invalid input happened "Invalid" will be reported to parent */
    }
  
    write(pipefd[1], &returnRecord, sizeof(returnRecord));
  
    return returnRecord; 
  }
  
void readCommand (char *command) {
  const char delim []= ";";
  char *tempCommand;
  char inputBuffer[100];

  printf("Please enter booking:\n");
  if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
    inputBuffer[strcspn(inputBuffer, "\n")] = '\0';
  }
  else {
    command[0] = '\0';
    return;
  }
  tempCommand = strtok(inputBuffer, delim);
  if (tempCommand != NULL) {
    strcpy(command, tempCommand);
  } else {
    command[0] = '\0';
    return;
  }
}

void readBatch(const char *batchName, int pipefd[2]) {
  const char delim[] = ";";
  char *tempCommand;
  char buffer[100];

  FILE *file = fopen(batchName, "r");

  while (fgets(buffer, sizeof(buffer), file) != NULL) { // read the each lines in the batch file 
      buffer[strcspn(buffer, "\n")] = '\0';

      // gain each command
      tempCommand = strtok(buffer, delim);
      
      // turn each command to record and pass these records to parent
      command2Record(tempCommand, pipefd);
  }

  fclose(file);
}



  

bool isTimeCrashed(record* rawData, int timeSlot[7][24][7], int startDay){ 
  int time = rawData->startTime/100;  //1030 /100 = 10
  int day = rawData->date - startDay; 

  if (startDay < 20250510 || startDay > 20250616){
    printf("ERROR IN START TIME\n");
    return false;
  }


  int start = rawData -> startTime /100 * 100; //remove the minutes
  int end = rawData -> endTime /100 * 100; //remove the minutes
  int duration = (end-start) / 100;
  if (rawData -> endTime % 100 != 0) duration++;

  bool onlyEssential = false;
  if (strcmp(rawData->name, "Essential") == 0) onlyEssential = true;
  if (strcmp(rawData -> essential1, "NO") == 0) onlyEssential = true;
              //printf("%s: duration is %d\n", rawData->name, duration);
  
  int i, j;
  const int numberOfEssential = 6;
  char essential[numberOfEssential][30];
  memset(essential,0,sizeof(essential)); //initialize the array
  strcpy(essential[0],rawData -> essential1);
  strcpy(essential[1],rawData -> essential2);
  strcpy(essential[2],rawData -> essential3);
  strcpy(essential[3],rawData -> essential4);
  strcpy(essential[4],rawData -> essential5);
  strcpy(essential[5],rawData -> essential6);
  int types[numberOfEssential]; 
  

  for (i = 0; i<numberOfEssential;i++){
    if (strstr(essential[i], "NO") != NULL) types[i] = -1; //-1 means no essential booking
    else if (strstr(essential[i], "battery") != NULL) types[i] = 1;
    else if (strstr(essential[i], "locker") != NULL) types[i] = 2;
    else if (strstr(essential[i], "InflationService") != NULL) types[i] = 3;
    else if (strstr(essential[i], "cable") != NULL) types[i] = 4;
    else if (strstr(essential[i],"umbrella") != NULL) types[i] = 5;
    else if (strstr(essential[i],"valetpark") != NULL) types[i] = 6;
    else {
      printf("ERROR IN ESSENTIAL %s\n", essential[i]);
      types[i] = -1;
    }
  }

  for (i = 0;i< duration;i++){
              //printf("%s loop %d %d\n", rawData->name, i, time+i);
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

  void resetTimeSlot(int timeSlot[7][24][7]){
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
  }

  void Priority(record rawData[], int sizeOfRecord, record result[2][2000],int* acceptCounter, int* rejectCounter, int analysisInfo[6][7][24]){
    int i; 
    int j; 
    int timeSlot[7][24][7];
    int startDate = 20250510;
    const int numberOfPriority = 5;
    resetTimeSlot(timeSlot);
    for (j = 0;j < numberOfPriority; j++){
      for (i = 0;i < sizeOfRecord; i++){
        int priority = (int) rawData[i].priority - 48;
        if (priority == j) {
          if (isTimeCrashed(&rawData[i], timeSlot, startDate)){
            result[0][*rejectCounter] = rawData[i]; //store the pointer to the record
            (*rejectCounter)++;
          }
          else{
            result[1][*acceptCounter] = rawData[i];
            (*acceptCounter)++;
          }
        }
      }
    }
    //write back timeslot essential result
    for (i = 0;i<7;i++){
      for (j = 0;j<24;j++){ // int analysisInfo[6][7][24];
        //printf("%d%d%d: %d\n" ,i,j,k, analysisInfo[i][j][k]);
        analysisInfo[0][i][j] = timeSlot[i][j][1];
        analysisInfo[1][i][j] = timeSlot[i][j][2];
        analysisInfo[2][i][j] = timeSlot[i][j][3];
        analysisInfo[3][i][j] = timeSlot[i][j][4];
        analysisInfo[4][i][j] = timeSlot[i][j][5];
        analysisInfo[5][i][j] = timeSlot[i][j][6];
      }
    }
  }

void FCFS(record rawData[], int sizeOfRecord, record result[2][2000],int* acceptCounter, int* rejectCounter, int analysisInfo[6][7][24]){
  int i; 
  int j; 
  int timeSlot[7][24][7];
  int startDate = 20250510;
  resetTimeSlot(timeSlot);
  for (i = 0;i < sizeOfRecord; i++){
    if (isTimeCrashed(&rawData[i], timeSlot, startDate)){
      result[0][*rejectCounter] = rawData[i]; //store the pointer to the record
      (*rejectCounter)++;
    }
    else{
      result[1][*acceptCounter] = rawData[i];
      (*acceptCounter)++;
    }
  }
  //write back timeslot essential result
  for (i = 0;i<7;i++){
    for (j = 0;j<24;j++){ // int analysisInfo[6][7][24];
      //printf("%d%d%d: %d\n" ,i,j,k, analysisInfo[i][j][k]);
      analysisInfo[0][i][j] = timeSlot[i][j][1];
      analysisInfo[1][i][j] = timeSlot[i][j][2];
      analysisInfo[2][i][j] = timeSlot[i][j][3];
      analysisInfo[3][i][j] = timeSlot[i][j][4];
      analysisInfo[4][i][j] = timeSlot[i][j][5];
      analysisInfo[5][i][j] = timeSlot[i][j][6];
    }
  }
}

void Scheduling(record data[],int sizeOfRecord,int method,record result[2][2000],int* acceptCounter,int* rejectCounter,int analysisInfo[6][7][24]){
  if (method == 0){ //FCFS
    FCFS(data,sizeOfRecord,result,acceptCounter,rejectCounter, analysisInfo);
    return;
  }
  if (method == 1){ //Priority
    Priority(data,sizeOfRecord,result,acceptCounter,rejectCounter, analysisInfo);
    return;
  }
  if (method == 2){ //optimal
    return;
  }
}

void printSummary(analysisBlock data[2]){
  FILE *outputFile = fopen("output.txt","w");

  if (outputFile == NULL) {
      printf("Failed to create the file\n");
      exit(1);
  }

  
  int accept = data[0].acceptCounter;
  int reject = data[0].rejectCounter;
  int total = accept + reject;
  double acceptPercentage = (double) accept * 100 / total;
  double rejectPercentage = (double) reject * 100 / total;
  fprintf(outputFile,"*** Parking Booking Manager – Summary Report ***\n\n");
  fprintf(outputFile,"Performance:\n\n");
  fprintf(outputFile,"  For FCFS:\n");
  fprintf(outputFile,"\t  Total number of Bookings Received: %d (100%%)\n", total);
  fprintf(outputFile,"\t        number of Bookings Assigned: %d (%2.2f%%)\n", accept, acceptPercentage);
  fprintf(outputFile,"\t        number of Bookings Rejected: %d (%2.2f%%)\n", reject, rejectPercentage);
  fprintf(outputFile,"\n");
  fprintf(outputFile,"\t  Utilization of Time Slot:\n\n");

  int totalEssential = 7*24*3;
  int essentialNumber[6];
  memset(essentialNumber,0,sizeof(essentialNumber));
  int i,j,k;
  for (i = 0;i<6;i++){
      for (j = 0;j<7;j++){
          for(k = 0;k<24;k++){
              essentialNumber[i] += data[0].timeslot[i][j][k];
          }
      }
  }
  double essentialPercentage;
  essentialPercentage = (double) (totalEssential - essentialNumber[0]) * 100 / totalEssential;
  fprintf(outputFile,"\t        Battery             - %2.2f%% \n", essentialPercentage);
  essentialPercentage = (double) (totalEssential - essentialNumber[1]) * 100 / totalEssential;
  fprintf(outputFile,"\t        cable               - %2.2f%% \n", essentialPercentage);
  essentialPercentage = (double) (totalEssential - essentialNumber[2]) * 100 / totalEssential;
  fprintf(outputFile,"\t        lockers             - %2.2f%% \n", essentialPercentage);
  essentialPercentage = (double) (totalEssential - essentialNumber[3]) * 100 / totalEssential;
  fprintf(outputFile,"\t        umbrella            - %2.2f%% \n", essentialPercentage);
  essentialPercentage = (double) (totalEssential - essentialNumber[4]) * 100 / totalEssential;
  fprintf(outputFile,"\t        inflation service   - %2.2f%% \n", essentialPercentage);
  essentialPercentage = (double) (totalEssential - essentialNumber[5]) * 100 / totalEssential;
  fprintf(outputFile,"\t        valet parking       - %2.2f%% \n", essentialPercentage);

  fprintf(outputFile,"\n");
  fprintf(outputFile,"Invalid request(s) made: %d\n\n", data[0].invalidCounter);
  fprintf(outputFile,"  For PRIO:\n");

  accept = data[1].acceptCounter;
  reject = data[1].rejectCounter;
  total = accept + reject;
  acceptPercentage = (double) accept * 100 / total;
  rejectPercentage = (double) reject * 100 / total;
  fprintf(outputFile,"\t  Total number of Bookings Received: %d (100%%)\n", total);
  fprintf(outputFile,"\t        number of Bookings Assigned: %d (%2.2f%%)\n", accept, acceptPercentage);
  fprintf(outputFile,"\t        number of Bookings Rejected: %d (%2.2f%%)\n", reject, rejectPercentage);
  fprintf(outputFile,"\n");
  fprintf(outputFile,"\t  Utilization of Time Slot:\n\n");

  totalEssential = 7*24*3;
  memset(essentialNumber,0,sizeof(essentialNumber));
  for (i = 0;i<6;i++){
      for (j = 0;j<7;j++){
          for(k = 0;k<24;k++){
              essentialNumber[i] += data[1].timeslot[i][j][k];
          }
      }
  }
  essentialPercentage = (double) (totalEssential - essentialNumber[0]) * 100 / totalEssential;
  fprintf(outputFile,"\t        Battery             - %2.2f%% \n", essentialPercentage);
  essentialPercentage = (double) (totalEssential - essentialNumber[1]) * 100 / totalEssential;
  fprintf(outputFile,"\t        cable               - %2.2f%% \n", essentialPercentage);
  essentialPercentage = (double) (totalEssential - essentialNumber[2]) * 100 / totalEssential;
  fprintf(outputFile,"\t        lockers             - %2.2f%% \n", essentialPercentage);
  essentialPercentage = (double) (totalEssential - essentialNumber[3]) * 100 / totalEssential;
  fprintf(outputFile,"\t        umbrella            - %2.2f%% \n", essentialPercentage);
  essentialPercentage = (double) (totalEssential - essentialNumber[4]) * 100 / totalEssential;
  fprintf(outputFile,"\t        inflation service   - %2.2f%% \n", essentialPercentage);
  essentialPercentage = (double) (totalEssential - essentialNumber[5]) * 100 / totalEssential;
  fprintf(outputFile,"\t        valet parking       - %2.2f%% \n", essentialPercentage);
  fprintf(outputFile,"\n");
  fprintf(outputFile,"Invalid request(s) made: %d\n\n", data[1].invalidCounter);

  fclose(outputFile);

}


int compare_records(const void *a, const void *b)
{
    record *recordA = (record  *)a;
    record *recordB = (record  *)b;


    if (recordA-> member != recordB-> member)
    {
        return recordA-> member - recordB-> member;
    }

    if (recordA-> date != recordB-> date)
    {
        return recordA->date - recordB-> date;
    }

    return recordA-> startTime - recordB-> startTime;
}
int compare_records_prio(const void *a, const void *b){
    record *recordA = (record *)a;
    record *recordB = (record *)b;

    if (recordA->member != recordB->member) {
        return recordA-> member - recordB->member;
    }
    
    if (recordA->priority != recordB->priority) {
        return recordA->priority - recordB->priority;
    }

    if (recordA->date != recordB->date) {
        return recordA->date - recordB->date;
    }
    
    return recordA->startTime - recordB-> startTime;
}


void copy_record(record buffer[2][2000], outputBlock *store_array, int acceptCounter, int rejectCounter)
{
    int i;
    int j;
    int loop=0;
    for (i = 0; i < 2; i++)
    {
        if (i==0){
            loop=rejectCounter;
        }
        else if (i==1){
            loop=acceptCounter;
        }
        else{
            perror("copy_record");
            exit(-1);
        }
        for (j = 0; j < loop; j++)
        {
            store_array->result[i][j] = buffer[i][j];
        }
    }

}
void print_records(outputBlock records,char *method)
{
    int i;
    int mode;
    int j;
    char current_member = '\0';
    int count=0;
    if (strstr(method,"FCFS")!=NULL){
    mode=0;
    qsort(records.result[0], records.rejectCounter, sizeof(record), compare_records);
    qsort(records.result[1], records.acceptCounter, sizeof(record), compare_records);
    }
    else {
    mode=1;
    qsort(records.result[0], records.rejectCounter, sizeof(record), compare_records_prio);
    qsort(records.result[1], records.acceptCounter, sizeof(record), compare_records_prio);
    }


    for (i=1;i>-1;i--){
        if ( i==1 && mode ==0){
            printf("*** Parking Booking – ACCEPTED / FCFS *** \n");
        }
        else if ( i==0 && mode ==0 ){
            printf("*** Parking Booking – REJECTED / FCFS *** \n");
        }   else if (i==1 && mode == 1){
            printf("*** Parking Booking – ACCEPTED / PRIO *** \n");
        }
        else if (i==0 && mode ==1 ){
            printf("*** Parking Booking – REJECTED / PRIO *** \n");
        }
        else {
            printf("show booking error \n ");
            exit(-1);
        }
        if(i==0){
            count=records.rejectCounter;
        }
        else if (i==1){
            count=records.acceptCounter;
        }
        else{
            count=0;
        }
        for (j = 0; j < count; j++)
        {
    
            if (records.result[i][j].name[0] == '\0')
                continue;

            if (records.result[i][j].member != current_member)
            {
                current_member = records.result[i][j].member;
                printf("\n");
                printf("Member_%c has the following bookings:\n", current_member);
                printf("Date         Start    End       Type                 Device\n");
                printf("===========================================================================\n");
            }

            printf("%d-%02d-%02d   %02d:%02d   %02d:%02d      %-15s      ",
                   records.result[i][j].date / 10000,
                   (records.result[i][j].date % 10000) / 100,
                   records.result[i][j].date % 100,
                   records.result[i][j].startTime / 100,
                   records.result[i][j].startTime % 100,
                   records.result[i][j].endTime / 100,
                   records.result[i][j].endTime % 100,
                   records.result[i][j].name);

                   int essentials_found = 0;
            
                   if (strcmp(records.result[i][j].essential1, "NO") != 0) {
                       printf("%s\n", records.result[i][j].essential1);
                       essentials_found = 1;
                   }
                   if (strcmp(records.result[i][j].essential2, "NO") != 0) {
                       if (essentials_found) {
                           printf("                                                     %s\n", records.result[i][j].essential2);
                       }
                       essentials_found = 1;
                   }
                   if (strcmp(records.result[i][j].essential3, "NO") != 0) {
                       if (essentials_found) {
                           printf("                                                     %s\n", records.result[i][j].essential3);
                       }
                       essentials_found = 1;
                   }
                   if (strcmp(records.result[i][j].essential4, "NO") != 0) {
                       if (essentials_found) {
                           printf("                                                     %s\n", records.result[i][j].essential4);
                       }
                       essentials_found = 1;
                   }
                   if (strcmp(records.result[i][j].essential5, "NO") != 0) {
                       if (essentials_found) {
                           printf("                                                     %s\n", records.result[i][j].essential5);
                       }
                       essentials_found = 1;
                   }
                   if (strcmp(records.result[i][j].essential6, "NO") != 0) {
                       if (essentials_found) {
                           printf("                                                     %s\n", records.result[i][j].essential6);
                       }
                       essentials_found = 1;
                   }
                   
                   if (essentials_found == 0 ) {
                       printf("*\n");
                   }
            printf("\n");
        }
        printf("        - End - \n");
        printf("===========================================================================\n");
        printf("\n"):
    }
}



int main(){
    int returnpid;
    const int numberOfModulue = 4;
    int childpid[numberOfModulue]; //4 module: Input, Scheduling, Output, Analysis
    int fd[numberOfModulue][2]; // array of pipeline for parent to children
    int cfd[numberOfModulue][2]; //array of pipeline for children to parent
    int i;
    int j;
    int childID;
    char buffer[100];
    printf("~~ WELCOME TO POLYU ~~\n");
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
            readCommand(receiveCommand);
            if (receiveCommand[0] == '\0') continue;
            newRecord = command2Record(receiveCommand, cfd[childID]);
            if (strcmp(newRecord.name, "addBatch") == 0) readBatch(newRecord.essential1, cfd[childID]);
            if (strcmp(newRecord.name,"endProgram") == 0) {
              close(fd[childID][0]);
              close(cfd[childID][1]);
              exit(1);
            }
        }
    }

        if(childID == 1){ //Schdule
          typedef struct {
            record rawData[2000];
            int method;
            int sizeOfRecord;
          } receivedData;

          

          typedef struct {
            record result[2][2000];
            int rejectCounter;
            int acceptCounter;
            char method[100];
          } outputBlock;
          
            receivedData data;
            outputBlock FCFSBlock; 
            strcpy(FCFSBlock.method, "FCFS");
            outputBlock PriorityBlock; 
            strcpy(PriorityBlock.method, "PRIO");
            int analysisInfo[6][7][24]; //item, days, hour
            int i,j,k;


            while (read(fd[childID][0], &data, sizeof(data)) != EOF){
              if (strcmp(data.rawData[0].name, "endProgram") == 0){
                close(fd[childID][0]);
                close(cfd[childID][1]);
                exit(1);
              }
              if (data.method == 0){ //FCFS
                Scheduling(data.rawData,data.sizeOfRecord,data.method,FCFSBlock.result, &FCFSBlock.acceptCounter, &FCFSBlock.rejectCounter, analysisInfo);
                write(cfd[childID][1], &FCFSBlock, sizeof(FCFSBlock));
                memset(&FCFSBlock, 0, sizeof(FCFSBlock));
                } 
              else if (data.method == 1 || data.method == 2){ //Priority
                Scheduling(data.rawData,data.sizeOfRecord,data.method,PriorityBlock.result, &PriorityBlock.acceptCounter, &PriorityBlock.rejectCounter, analysisInfo);
                write(cfd[childID][1], &PriorityBlock, sizeof(PriorityBlock));
                memset(&PriorityBlock, 0, sizeof(PriorityBlock));
              }
        // else if (method == 2){
        // }
              else if (data.method == 3){ //ALL
                analysisBlock summaryReport[2];

                Scheduling(data.rawData,data.sizeOfRecord,0,FCFSBlock.result, &FCFSBlock.acceptCounter, &FCFSBlock.rejectCounter, analysisInfo);
                
                for (i = 0;i < 6; i++){
                  for (j = 0;j<7;j++){
                    for (k = 0;k<24;k++){
                      summaryReport[0].timeslot[i][j][k] = analysisInfo[i][j][k];
                    }
                  }
                }
                summaryReport[0].acceptCounter = FCFSBlock.acceptCounter;
                summaryReport[0].rejectCounter = FCFSBlock.rejectCounter;
          

                memset(&FCFSBlock, 0, sizeof(FCFSBlock));

                Scheduling(data.rawData,data.sizeOfRecord,1,PriorityBlock.result, &PriorityBlock.acceptCounter, &PriorityBlock.rejectCounter, analysisInfo);
                
                for (i = 0;i < 6; i++){
                  for (j = 0;j<7;j++){
                    for (k = 0;k<24;k++){
                      summaryReport[1].timeslot[i][j][k] = analysisInfo[i][j][k];
                    }
                  }
                }
                summaryReport[1].acceptCounter = PriorityBlock.acceptCounter;
                summaryReport[1].rejectCounter = PriorityBlock.rejectCounter;

                //send the summary report to parent
                write(cfd[childID][1], summaryReport, sizeof(summaryReport));

                memset(&PriorityBlock, 0, sizeof(PriorityBlock));
              }


            }
        
        }
        if(childID == 2){ //Output

          outputBlock buffer;
          while (read(fd[childID][0], &buffer, sizeof(buffer)) > 0){
            if (buffer.acceptCounter == -1){
              close(fd[childID][0]);
              close(cfd[childID][1]);
              exit(1);
            }
            
        outputBlock store_array;
        buffer.method[sizeof(buffer.method)-1] = '\0'; 
        strcpy(store_array.method,buffer.method);
        store_array.acceptCounter=buffer.acceptCounter;
        store_array.rejectCounter=buffer.rejectCounter;
        copy_record(buffer.result, &store_array, buffer.acceptCounter, buffer.rejectCounter);
        print_records(store_array, store_array.method);
        printf("-> [Done!] \n");
          }
        }
        if(childID == 3){ //Analysis

          analysisBlock analysisData[2];
          while (read(fd[childID][0], &analysisData, sizeof(analysisData)) != EOF){
            if (analysisData[0].acceptCounter == -1){
              close(fd[childID][0]);
              close(cfd[childID][1]);
              exit(1);
            }

            printSummary(analysisData);
          }
        }

        //close pipeline
  

    }
    
    else{ //parent
      record receiveRecord;
      int i;
      typedef struct {
        record memory[2000];
        int method;
        int recordCounter;
      } dataToSend;
      dataToSend data; //send to schedulng

      typedef struct {
        record result[2][2000]; //0 is reject, 1 is accept
        int rejectCounter;
        int acceptCounter;
        char method[100];
      } outputBlock;
      outputBlock FCFSBlock;
      outputBlock PriorityBlock;

      analysisBlock receivedAnalysisData[2]; //0 is FCFS, 1 is priority
      int invalid = 0;

      while (1) {
   
          // read the record from child 
          while (read(cfd[0][0], &receiveRecord, sizeof(receiveRecord)) > 0) {
              // printf("Command name is %s\n", receiveRecord.name);
              // printf("date is %d\n", receiveRecord.date);
              // printf("member name is %c\n", receiveRecord.member);
              // printf("start time is %d\n", receiveRecord.startTime);
              // printf("Time duration is %1.1f\n", receiveRecord.timeDuration);
              // printf("end time is %d\n", receiveRecord.endTime);
              // printf("priority is %c\n", receiveRecord.priority);
              // printf("essential1 is %s\n", receiveRecord.essential1);
              // printf("essential2 is %s\n", receiveRecord.essential2);
              // printf("essential3 is %s\n", receiveRecord.essential3);
              // printf("essential4 is %s\n", receiveRecord.essential4);
              // printf("essential5 is %s\n", receiveRecord.essential5);
              // printf("essential6 is %s\n", receiveRecord.essential6);
              //printf("\n");

              if (strcmp(receiveRecord.essential1, "Invalid") == 0){
                invalid++;
                continue;
              }


              if (strcmp(receiveRecord.name, "endProgram") == 0) {
                  strcpy(data.memory[0].name,"endProgram");
                  write(fd[1][1], &data, sizeof(data)); //send to schduling
/////////////////////
                  FCFSBlock.acceptCounter = -1;
                  write(fd[2][1],&FCFSBlock,sizeof(FCFSBlock)); //send to output
/////////////////////
                  receivedAnalysisData[0].acceptCounter = -1;
                  write(fd[3][1], &receivedAnalysisData, sizeof(receivedAnalysisData)); //send to analysis
///////////////////////
                  int status;
                  for (i = 0; i< numberOfModulue; i++){
                    waitpid(childpid[i], &status, 0);
                  }
                  printf("-> BYE! \n");
                  exit(1);
              }

              if(strcmp(receiveRecord.name,"printBookings") != 0){
                printf("-> [Pending] \n");
                data.memory[data.recordCounter] = receiveRecord;
                data.recordCounter++;
                
              }

              if (strcmp(receiveRecord.name, "printBookings") == 0){
                //select method
                if (strcmp(receiveRecord.essential1,"fcfs") == 0){
                  
                  data.method = 0;
                  write(fd[1][1], &data, sizeof(data));
                  if (read(cfd[1][0], &FCFSBlock, sizeof(FCFSBlock)) != EOF){
                    // SEND TO OUTPUT
                    write(fd[2][1],&FCFSBlock,sizeof(FCFSBlock));
                  }

                }
                else if (strcmp(receiveRecord.essential1,"prio") == 0){
                  data.method = 1;
                  write(fd[1][1], &data, sizeof(data));
                  if (read(cfd[1][0], &PriorityBlock, sizeof(PriorityBlock)) != EOF){
                    // SEND TO OUTPUT
                    write(fd[2][1],&PriorityBlock,sizeof(PriorityBlock));
                  }
                }

                else if (strcmp(receiveRecord.essential1,"opti") == 0){
                  data.method = 2;
                  write(fd[1][1], &data, sizeof(data));
                  if (read(cfd[1][0], &PriorityBlock, sizeof(PriorityBlock)) != EOF){
                    // SEND TO OUTPUT
                    write(fd[2][1],&PriorityBlock,sizeof(PriorityBlock));
                  }
                }
                else if (strcmp(receiveRecord.essential1,"ALL") == 0){
                  data.method = 3;
                  write(fd[1][1], &data, sizeof(data));
                  if (read(cfd[1][0], &receivedAnalysisData, sizeof(receivedAnalysisData)) != EOF){
                    receivedAnalysisData[0].invalidCounter = invalid;
                    receivedAnalysisData[1].invalidCounter = invalid;
                    write(fd[3][1], &receivedAnalysisData, sizeof(receivedAnalysisData));
                  }
                  printf("-> [Done!] \n");
                }
                
              }
          }

          // end the program 
          
      }
  }
}
