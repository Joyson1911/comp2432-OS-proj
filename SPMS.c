#include <stdio.h>
#include <stdbool.h>   // what is the use of this lib boolean
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void readCommand(){
    int i =0;
    printf("Please enter booking: \n");
    char command[50][50];
    //scan user input into the input
    while (scanf("%s", command[i]) != EOF) {
        if (strcmp(command[i]), ";" == 0) {
            findOutCommand(command[i]);
            i++;
        }
    }  
}

// The syntax of command is assumed that must correct 
void findOutCommand(char command[]){
    if (strstr(command, "addParking") != NULL) {}
    else if (strstr(command, "addReservation") != NULL) {}
    else if (strstr(command, "addEvent" != NULL) {}
    else if (strstr(command, "bookEssentials" != NULL) {}
    else if (strstr(command, "printBookings" != NULL) {}
    else if (strstr(command, "endProgram" != NULL) {}
}

int main(){
    bool exit = false;
    printf("~~ WELCOME TO POLYU ~~\n");
    readCommand();

}
