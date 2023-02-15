#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void compress(char* src, size_t size) {     // compress function
    int counter;                            // counter initialized
    char count[size];                       // count array to keep counter for later initialized
    int len = strlen(src);               // length of src
    int char_counter = 0;    // keep track of position for later
 
    // If all characters are different, size = twice of line in file
    char* encode = (char*)malloc(sizeof(char) * (len*2 + 1));
 
    for (int i = 0; i < len; i++) {     // for loop through string
        counter = 1;                    // initiate count of new char
        while (i + 1 < len && src[i] == src[i + 1]) { // checking every occurance of char from this point
            counter++;      // increment
            i++;            // increment
        }

        if (counter > 1) {                                  // more than one occurence of letter
            sprintf(count, "%d", counter);        // Store counter in count[] 
            for (int j = 0; *(count + j); j++) {           // for loop 
                encode[char_counter] = count[j];           // add counter to encode
                char_counter++;                            // increment char counter
            }           
        }
        encode[char_counter++] = src[i];                   // add char to encode
    }
 
    encode[char_counter] = '\0';                 // terminate
    printf("%s", encode);     // printing encode
    free(encode);                // free encode
    encode = NULL;                   // clear encode
}

void rle(int fd, char* filename){ // run length encoding function
    char *comp = NULL;              // compressed buffer
    size_t comp_buf_size = 0;       // compressed buffer size
    size_t comp_size;               // compressed size

    if (fd == 0) { // piped
        comp_size = getline(&comp, &comp_buf_size, stdin); // get info from stdin
    }
    else { // NOT PIPED
        FILE *fp = fopen(filename, "r"); // opening file

        if (!fp) {  // checking file
            perror("Error: could not open file "); // error MSG 
            return; // exit
        }

        // getting the first and only line in the file, comp_size = size of line in file
        comp_size = getline(&comp, &comp_buf_size, fp);
        fclose(fp);                    // closing file
    }

    compress(comp, comp_size);    // compressing
    free(comp);                        // free the compressed buffer
    comp = NULL;                           // comp now empty
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {                    // piping
        rle(0, "");
    }

    for (int i = 1; i < argc; i++) {    // ./rle + filename
        rle(i, argv[i]);
    }
    return 0;
}
