#include <stdio.h>
#include <stdbool.h>   
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
char* validInput(char command[]);
void readKeyboard();
char memory[100][50];
void addParking(char buffer[]);

void readCommand(){
    int i = 0;
    printf("Please enter booking: \n");
    int pid;

    // do {
    //     readKeyboard();
    //     if (strstr(command, "addParking") != NULL) {
    //         addParking(char[buffer]);
    //     }



    //     else if (strstr(command, "addReservation") != NULL) {}
    //     else if (strstr(command, "addEvent" != NULL))  {}
    //     else if (strstr(command, "bookEssentials" != NULL)) {}
    //     else if (strstr(command, "printBookings" != NULL)) {}
    // } while (validInput(buffer) != -1);






}








char priority(char command[]){
    if (strstr(command, "addParking") != NULL) return '3';
    else if (strstr(command, "addReservation") != NULL) return '2';
    else if (strstr(command, "addEvent") != NULL)  return '1';
    else if (strstr(command, "bookEssentials") != NULL)  return '4';
    else if (strstr(command, "printBookings") != NULL)  return '5';
        else return '0';
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
            char data[50];
            int j = 2, k = 0, l = 0, n = 0;
            int i;
            int spaceCount;
            char time[3];
            char hour[3];
            char minutes[3];
            int hourInt;
            int minutesInt;
            double timeDec;
            bool flag = false;
            char time_str[6];
            char * ptr;
            while (read(fd[0][0], buffer, sizeof(buffer)-1) > 0){

                data[0] = priority(buffer); //priority
                // member name
                if (strstr(buffer, "member_A") != NULL) data[1] = 'A';
                else if (strstr(buffer, "member_B") != NULL) data[1] = 'B';
                else if (strstr(buffer, "member_C") != NULL) data[1] = 'C';
                else if (strstr(buffer, "member_D") != NULL) data[1] = 'D';
                else if (strstr(buffer, "member_E") != NULL) data[1] = 'E';
                printf("data[1] is %s\n", data[1]);
                // get the data
                for (i = 0;i<sizeof(buffer)-1;i++){
                    if (buffer[i] == ' ') spaceCount++;
                    if (spaceCount < 2) continue;
                    // get the date 
                    if (spaceCount == 2) {
                        data[j++]=buffer[i];
                        continue;
                    }
                    // get the hour 
                    if (spaceCount == 3) {
                        if (buffer[i]==':') {
                            flag=true;
                            data[j++]=buffer[i];
                        }
                        else if (!flag) {
                            data[j++]=buffer[i];
                            hour[k++]=buffer[i];
                        }
                        else {
                            data[j++]=buffer[i];
                            minutes[l++]=buffer[i];
                        }
                        continue;
                    }

                    if (spaceCount == 4) {
                        time[n++]=buffer[i];
                        continue;
                    }
                    if (spaceCount==5) break; // need change 
                }

                //convert start time string to int 
                hourInt=atoi(hour);
                minutesInt=atoi(minutes);
                timeDec=strtod(time, &ptr);
                // cal the finish time
                minutesInt+=(timeDec * 60);
                if (minutesInt >= 60) {
                    hourInt += minutesInt / 60; // Add extra hour(s)
                    minutesInt = minutesInt % 60; // Keep remaining minutes
                }
                //convert finish time int to string 
                sprintf(time_str, "%d:%02d", hourInt, minutesInt);
                strcat(data, time_str);

                switch (data[0]){
                    case '1':
                        strcat(data, " Event ");
                        break;
                    case '2':
                        strcat(data," Reservation ");
                        break;
                    case '3':
                        strcat(data," Parking ");
                        break;
                    case '0':
                        strcat(data," ERROR ");
                }
                
                printf("%s\n",data);

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
        if (read(0, buffer, sizeof(buffer)-1) == -1){
            perror("read");
            exit(1);
        }
        if (strstr(buffer, "endProgram") == NULL) { //call childpid[0] to read
            if (write(fd[0][1], buffer, sizeof(buffer)-1) == -1){ //read keyboard
                perror("write");
                exit(1);
            }
            pause();
        }
        else{
            //collect all child and exit
        }

    }







    printf("~~ WELCOME TO POLYU ~~\n");
    readCommand();

}
