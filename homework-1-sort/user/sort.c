#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

char buf[512];

void insertionSort(int arr[], int n) { // insertion sorting method
    int i, j, tmp; // tmp values used to move positions
    for (i = 1; i < n; i++) { // first for loop throuh array
        tmp = arr[i]; // tmp val 
        j = i - 1; 

        while (j >= 0 && arr[j] > tmp) { // Move elements in array greater than tmp val
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = tmp; // move
    }
}
  
void Printing(int arr[], int n) { // print from smallest to biggest
    for (int i = 0; i < n; i++) { // start from beginning of sorted array
        printf(1, "%d ", arr[i]); // printing val
    }
    printf(1,"\n"); // END
}

void Reverseprint(int arr[], int n) { // print from biggest to smallest
    for (int i = n - 1; i >= 0; i--) { // start from end of sorted array
        printf(1, "%d ", arr[i]); // printing val
    }
    printf(1,"\n"); // END
}

void sort(int fd, _Bool rflag) { // sort function
    int n, size, num; // initialize ints 
    int arr[sizeof(buf)]; // initialize array
    
    size = 0; // size of array that has elements
    num = 0; // number taken from file
    
    while ((n = read(fd, buf, sizeof(buf))) > 0) { // reading through file
        for (int i = 0; i < n; i++) { // reading nums in file...
            if (strchr(" \r\t\n\v", buf[i])) { // not a single digit number
                num = 0; // num = 0 for now until its done
            }
            else if (!num) { // got full number
                num = 1; // reset num
                arr[size] = atoi(&buf[i]); // fill up array
                size++; // increment size to keep track of array position and size of array
            }
        }
    }

    if (n < 0) { // nothing in file
        printf(1, "sort: read error\n"); // Error MSG
        exit(); // END OF PROGRAM
    }
    insertionSort(arr, size); // insertion sort numbers
    if (rflag == 1) { Reverseprint(arr, size); } // print in reverse or...
    else { Printing(arr, size);} // print normally
}

int main(int argc, char *argv[]) {
    _Bool rflag = 0; // reverse flag to check if output needs to be reversed
    _Bool oflag = 0; // open flag to check if file has been opened
    int fd; // for opening file

    for (int i = 1; i < argc; i++) { // look through argument vector to see if -r flag
        if (strcmp(argv[i], "-r") == 0) { // checking for -r flag
            rflag = 1; // r flag raised
        }
    }

    if ((argc - rflag) <= 1) { // piping (note the minus rflag since if the flag is up it increases arguement count )
        sort(0, rflag); // sort with fd = 0
        exit(); // END OF PROGRAM
    }

    for (int i = 1; i < argc; i++) { // for loop through argv
        if ((fd = open(argv[i], 0)) > 0) { // checking if file can be opened
            sort(fd, rflag); // openable file needs to be sorted
            close(fd); // closing file now
            oflag = 1; // open file flag raised
        }
    }
    if (oflag == 0) { printf(1, "sort can't open any files\n"); } // if no files been open this is an error
    exit(); // END OF PROGRAM
}