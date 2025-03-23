#include <stdio.h>
#include <stdbool.h>   
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>

typedef struct {
    char name[20]; //[Parking], [Essential], [Reservation], [Event]
    char member; //[X] e.g A
    int date; //[XXXXXXXX] e.g 20250510
    int startTime; // [XXXX] e.g 1000
    int timeDuration;// [X] e.g 3
    int endTime;// [XXXX] e.g 1300
    char priority; // [1],[2],[3],[4],[5]
    char essential1[30]; //[No], [Battery], [Lockers], [Inflation]
    char essential2[30]; //[No], [Cable], [Umbrella], [Valet]
  } record;

//record* FCFS(record* data, int sizeOfRecord, int method);
//bool isTimeCrashed(record* data, int timeSlot[7][24][4]);
//record* Scheduling(record* data, int sizeOfRecord, int method);

//timeslot[numberOfDay][numberOfTimeslot][typeOfEssential+1]
  bool isTimeCrashed(record* rawData, int timeSlot[7][24][7], int startDay){ 
    int time = rawData->startTime/100;
    int day = rawData->date - startDay;
    int duration = rawData -> timeDuration;
    bool onlyEssential = false;
    if (strcmp(rawData->name, "Essential") == 0) onlyEssential = true;
    if (rawData->startTime % 100 != 0) duration++; // 1030 to 1130 is 2 timeslot, 1000 to 1100 is 1 timeslot
    printf("%s: duration is %d\n", rawData->name, duration);
    
    int i;
    int essential1 = -1; //-1 means no essential booking
    int essential2 = -1;
    if (strstr(rawData->essential1, "No") != NULL); 
    else if (strstr(rawData->essential1, "Battery") != NULL) essential1 = 1;
    else if (strstr(rawData->essential1, "Lockers") != NULL) essential1 = 2;
    else if (strstr(rawData->essential1, "Inflation") != NULL) essential1 = 3;

    if (strstr(rawData->essential2, "No") != NULL); 
    else if (strstr(rawData->essential2, "Cable") != NULL) essential2 = 4;
    else if (strstr(rawData->essential2,"Umbrella") != NULL) essential2 = 5;
    else if (strstr(rawData->essential2,"Valet") != NULL) essential2 = 6;

    for (i = 0;i< duration;i++){
      printf("%s loop %d %d\n", rawData->name, i, time+i);
      //checking the time (onlyEssential dont have to check the time)
      if (timeSlot[day][time+i][0] == 0 && !onlyEssential){ 
        return true;
      }
      //checking the essential
      if (essential1 != -1 && timeSlot[day][time+i][essential1] == 0){ //checking the essential
        return true;
      }
      if (essential2 != -1 && timeSlot[day][time+i][essential2] == 0){
        return true;
      }
    }

    for(i = 0;i< duration;i++){
      if (!onlyEssential) timeSlot[day][time+i][0] -= 1;
      timeSlot[day][time+i][essential1] -= 1;
      timeSlot[day][time+i][essential2] -= 1;
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
    int timeSlot[day][hour][typesOfEssentials+1]; //[0] = time, [1] = essential(Battery) [2]=lockers [3] = inflation services
                                                  //[4] = Cable, [5] = Umbrella, [6] = Valet
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



void Scheduling(record data[], int sizeOfRecord, int method,record result[2][2000]){
  if (method == 0){ //FCFS
    FCFS(data,sizeOfRecord,result);
    memset(data, 0, sizeof(record)); //clear the FCFSresult array
    return;
  }
  if (method == 1){ //Priority
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

    record y;
    strcpy(y.name,"ParkingY");
    y.member = 'A';
    y.date = 20250510;
    y.startTime = 1000;
    y.timeDuration = 3;
    y.endTime = 1300;
    y.priority = '1';
    strcpy(y.essential1,"Battery");
    strcpy(y.essential2,"Cable");

    array[0] = x;
    array[1] = y;


    Scheduling(array,2,0,FCFSresult);
    printf("%s\n","dadasda");
  
}
