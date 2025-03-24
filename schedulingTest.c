#include <stdio.h>
#include <stdbool.h>   
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>

typedef struct {
    char name[20]; //[Parking], [Essential], [Reservation], [Event]
    char member; //[X] e.g A
    int date; //[XXXXXXXX] e.g 20250510
    int startTime; // [XXXX] e.g 1000 1030
    float timeDuration;// [X] e.g 3.5     10-11 11-12 12-13 13-14
    int endTime;// [XXXX] e.g 1330  1400
    char priority; // [1],[2],[3],[4],[5]
    char essential1[30]; //[No], [Battery], [Lockers], [Inflation]
    char essential2[30]; //[No], [Cable], [Umbrella], [Valet]
    char essential3[30];
  } record;

//record* FCFS(record* data, int sizeOfRecord, int method);
//bool isTimeCrashed(record* data, int timeSlot[7][24][4]);
//record* Scheduling(record* data, int sizeOfRecord, int method);
//void Priority(record rawData[], int sizeOfRecord, record result[2][2000])

//timeslot[numberOfDay][numberOfTimeslot][typeOfEssential+1]
  bool isTimeCrashed(record* rawData, int timeSlot[7][24][7], int startDay){ 
    int time = rawData->startTime/100; 
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





  void FCFS(record rawData[], int sizeOfRecord, record result[2][2000]){
    //You can modify the variable here
    const int startDate = 20250510;
    const int endDate = 20250516;
    const int day = endDate - startDate + 1;
    const int hour = 24;
    const int numberOfParkingSlot = 2;
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
  
    int rejectCounter = 0;
    int acceptCounter = 0;
    

    for (i = 0;i < sizeOfRecord; i++){
      if (isTimeCrashed(&rawData[i], timeSlot, startDate)){
        result[0][rejectCounter] = rawData[i]; //store the pointer to the record
        rejectCounter++;
        printf("REJECT\n");
      }
      else{
        result[1][acceptCounter] = rawData[i];
        acceptCounter++;
        printf("ACCEPT\n");
      }
    }
}

void Priority(record rawData[], int sizeOfRecord, record result[2][2000]){
//You can modify the variable here
    const int startDate = 20250510;
    const int endDate = 20250516;
    const int day = endDate - startDate + 1;
    const int hour = 24;
    const int numberOfParkingSlot = 2;
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
  
    int rejectCounter = 0;
    int acceptCounter = 0;

    for (j = 0;j < numberOfPriority; j++){
      for (i = 0;i < sizeOfRecord; i++){
        int priority = (int) rawData[i].priority - 48;
        if (priority == j) {
          if (isTimeCrashed(&rawData[i], timeSlot, startDate)){
            result[0][rejectCounter] = rawData[i]; //store the pointer to the record
            rejectCounter++;
            printf("REJECT\n");
          }
          else{
            result[1][acceptCounter] = rawData[i];
            acceptCounter++;
            printf("ACCEPT\n");
          }
        }
      }
  }
}
void Scheduling(record data[], int sizeOfRecord, int method,record result[2][2000]){
  if (method == 0){ //FCFS
    FCFS(data,sizeOfRecord,result);
    return;
  }
  if (method == 1){ //Priority
    Priority(data,sizeOfRecord,result);
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
    
    int i;
    int method = 0; // 0=FCFS 1=Priority 2=optimal 3 = ALL

    record FCFSresult[2][2000]; //[0] = reject. [1] = accept 
    record Priorityresult[2][2000]; 
    record Optimalresult[2][2000]; 


    record array[1000];
    record x;
    strcpy(x.name,"ParkingX");
    x.member = 'A';
    x.date = 20250510;
    x.startTime = 1030;
    x.endTime = 1330;
    x.timeDuration = 3;
    x.priority = '1';
    strcpy(x.essential1,"Battery");
    strcpy(x.essential2,"Cable");
    strcpy(x.essential3,"No");

    record y;
    strcpy(y.name,"ParkingY");
    y.member = 'A';
    y.date = 20250510;
    y.startTime = 1030;
    y.timeDuration = 3.5;
    y.endTime = 1400;
    y.priority = '1';
    strcpy(y.essential1,"Battery");
    strcpy(y.essential2,"Cable");
    strcpy(y.essential3,"No");

    array[0] = x;
    array[1] = y;


    Scheduling(array,2,1,FCFSresult);
    memset(FCFSresult, 0, sizeof(record)); //clear the FCFSresult array
    printf("%s\n","dadasda");
  
}
