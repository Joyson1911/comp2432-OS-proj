#include <stdio.h>
#include <stdbool.h>   // what is the use of this lib boolean
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
char* validInput(char command[]);
void readKeyboard();
char memory[100][50];
char buffer[50];
void addParking(char buffer[]);

void readCommand(){
    int i = 0;
    printf("Please enter booking: \n");
    int pid;

    do {
        readKeyboard();
        if (strstr(command, "addParking") != NULL) {
            addParking(char[buffer]);
        }



        else if (strstr(command, "addReservation") != NULL) {}
        else if (strstr(command, "addEvent" != NULL))  {}
        else if (strstr(command, "bookEssentials" != NULL)) {}  
        else if (strstr(command, "printBookings" != NULL)) {}
    } while (validInput(buffer) != -1);






}



//read keyboard
void readKeyboard(){
    int bytesread;
    if ((bytesread = read(0, buffer, sizeof(buffer)-1)) == -1){ //read the user input from keyboard
        perror("read");
        exit(1);
    }
    buffer[bytesread] = '\0';
}




int validCommand(char command[]){
    if (strstr(command, "addParking") != NULL) return 1;
    else if (strstr(command, "addReservation") != NULL) return 2;
    else if (strstr(command, "addEvent" != NULL))  return 3;
    else if (strstr(command, "bookEssentials" != NULL))  return 4;
    else if (strstr(command, "printBookings" != NULL))  return 5;
    else if (strstr(command, "endProgram" != NULL))  return 6;
    else return -1;
}





int main(){
    int returnpid;
    int childpid[4]; //4 module: Input, Scheduling, Output, Analysis
    int childID;
    int fd[4][2]; // array of pipeline for parent to children
    int pfd[4][2]; //array of pipeline for children to parent
    int i;
    int j;
     //create pipeline
     for (i = 0;i < 4; i++){
        if (pipe(fd[i]) < 0) { //parent to children
                printf("Pipeline creation failed\n");
                exit(1);
        }
        if (pipe(pfd[i]) < 0) { //children to parent
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
                                close(pfd[j][0]);
                                close(pfd[j][1]);
                        }
                }
                close(fd[i][1]); //child close the sender side
                close(pfd[i][0]); //child close the receiever side
                childID = i;
                break;
        }
        else{ //parent process
                close(fd[i][0]);
                close(pfd[i][1]);
                childpid[i] = returnpid;
        }
     }








    if (returnpid == 0){ //child
        
    }
    else{ //parent
        
    }







    printf("~~ WELCOME TO POLYU ~~\n");
    readCommand();

}
