#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NTHREADS 8
#define INPUT_THRESHOLD NTHREADS * 3

typedef struct { // NEW STRUCT 
    char letter; // GETS LETTER/CHAR
    int length; // GETS LENGTH/COUNTER OF LETTER
} RunEntry;

typedef struct { // buffer struct
  char *base;
  RunEntry *enc; // Array of RunEntry to keep track of each letter and their length/counter
  unsigned int capacity;
  unsigned int bytesRead;
} Buffer;

typedef struct { // prle struct
  Buffer *buf;
  Buffer *result;
  unsigned int offset;
  unsigned int chunk_size;
} prleArg; // RunArg

Buffer allocBuffer(int initCapacity) { // allocating data to buffer
  return (Buffer) {
      .base = malloc(initCapacity),
      .enc = malloc(initCapacity * initCapacity),
      .capacity = initCapacity,
      .bytesRead = 0,
  };
}

void growBuffer(Buffer *buf, int newSize) { // growing buffer whan max buffer size reached
  if (buf->capacity >= newSize) {
    return;
  }
  buf->base = realloc(buf->base, newSize);
  buf->enc = realloc(buf->enc, newSize * newSize);
  buf->capacity = newSize;
}

Buffer readIntoBuffer(FILE *file) { // reading file into buffer
  int n;
  Buffer buf = allocBuffer(10);
  while ((n = fread(buf.base + buf.bytesRead, sizeof(char),
                    buf.capacity - buf.bytesRead, file)) > 0) {
    buf.bytesRead += n;
    if (buf.bytesRead == buf.capacity) {
      growBuffer(&buf, buf.capacity * 2);
    }
  }
  return buf;
}

void freeBuffer(Buffer *buf) { free(buf->base); } // free buffer

void prle(Buffer *buf, Buffer *result, int start, int end) { // Conquer, rle each thread
  *result = allocBuffer(end - start);
  int counter;                            // counter initialized
  int char_counter = 0;    // keep track of position for later
  //int count[end-start];                       // count array to keep counter for later initialized
 
  for (int i = start; i < end; i++) {     // for loop through string
    counter = 1;                    // initiate count of new char
    while (i + 1 < end && buf->base[i] == buf->base[i + 1]) { // checking every occurance of char from this point
        counter++;      // increment
        i++;            // increment
    }
    result->enc[char_counter].length = counter;
    result->enc[char_counter].letter = buf->base[i];                   // add char to result

    result->bytesRead++;                            // increase byteRead
    char_counter++;

    //printf("%d", result->enc[char_counter].length);             
    //printf("%c", result->enc[char_counter].letter); 
  }

  //result->base[char_counter] = '\0';                 // terminate
  result->enc[char_counter].letter = '\0';   
  result->enc[char_counter].length = '\0';

  // DEBUGGING 
  // /*
  //printf(" %d - %d ", start, end);                
  //printf(" %d ", result->bytesRead);
  //printf(" %d ",char_counter); 
  //printf("%d", result->enc[result->bytesRead - 1].length);             
  //printf("%c", result->enc[result->bytesRead-1].letter);             
  //printf(" %s ", buf->base);                
  //printf(" %s ", result->base);                 
  // */ 
}

// The entrypoint of the threads
void *run_prle(void *arg) { // starting function
  prleArg *data = (prleArg *)arg; // data in prleArg
  prle(data->buf, data->result, data->offset, data->offset + data->chunk_size); // run rle function
  pthread_exit(NULL); // exit out of thread
}

void go(Buffer *buf) {
  int n_threads = 1; // number of threads = 1

  if (buf->bytesRead > INPUT_THRESHOLD) { // unless the number of btes is greater than input threshhold
    n_threads = NTHREADS;                 // then use max number of threads we can use (8)
  }

  // Everything is allocated on the heap
  pthread_t *threads = malloc(n_threads * sizeof(pthread_t)); // initialize threads
  prleArg *args = malloc(n_threads * sizeof(prleArg));        // create prle args

  Buffer *results = malloc(n_threads * sizeof(Buffer));       // create result buffer to hold output
  int chunk_size = buf->bytesRead / n_threads;                      // getting chunk size per thread

  for (int i = 0; i < n_threads; i++) {   // for each thread
    int size = chunk_size;
    if (i == n_threads - 1) {
      size = buf->bytesRead - i * chunk_size;
    }

    args[i] = (prleArg){
        .buf = buf,
        .offset = i * chunk_size,
        .chunk_size = size,
        .result = &results[i],
    };

    pthread_create(&threads[i], NULL, run_prle, &args[i]); // threads created
  }

  // MERGE THREADS
  RunEntry prev_char;   // previous letter and its length
  prev_char.letter = ' ';   // initialize prev_char values
  prev_char.length = 0;     // initialize prev_char values
  int new_length = 0;     // new length tracker to keep track when two or more letters repeat between threads

  for (int i = 0; i < n_threads; i++) {
    pthread_join(threads[i], NULL); // joining
    if (i > 0) { // IF NOT THE FIRST THREAD
      if (prev_char.letter == results[i].enc[0].letter && i != n_threads - 1) { // checking if there is match between threads
        if (results[i].bytesRead == 1) { // if only one letter
          results[i].enc[0].length += prev_char.length; // adding previous length to current length
          new_length = results[i].enc[0].length; // save in new_length
          prev_char.length = results[i].enc[0].length; // update pre_char length
          continue; // move on
        }
        else { // if there is more than one letter
          results[i].enc[0].length += prev_char.length; // same thing as before
          new_length = results[i].enc[0].length;
          printf("%d", new_length);
          fputc(prev_char.letter, stdout);

          for (int j = 0; j < results[i].bytesRead; j++) { // execpt we have to update prev_char to be different after matching
            if (prev_char.letter != results[i].enc[j].letter) { // going through letters
              new_length = results[i].enc[j].length; // update new_length
              prev_char.length = results[i].enc[j].length; // update prev_char
              prev_char.letter = results[i].enc[j].letter; // update_prev_char
            }
          }
          continue; 
        }
      } 
    }
    else { // IF FIRST THREAD
      prev_char.letter = results[i].enc[results[i].bytesRead - 1].letter; // set previous char letter to be this letter
      prev_char.length = results[i].enc[results[i].bytesRead - 1].length; // set previous char length to be this length
      continue;
    }

    for (int j = 0; j < results[i].bytesRead; j++) { // loop through result buffers
      if (new_length == 0) { // if there was no need to merge two lengths
        if (results[i].enc[j].length == 1) { 
          fputc(results[i-1].enc[j].letter, stdout); // print out previous letter only if length was one
        }
        else {
          printf("%d", results[i].enc[j].length); // print previous length
          fputc(results[i-1].enc[j].letter, stdout); // & print previous letter
        }
      }
    }
    prev_char.letter = results[i].enc[results[i].bytesRead - 1].letter; // setting new prev_char letter
    prev_char.length = results[i].enc[results[i].bytesRead - 1].length; // setting new prev_char length
    //freeBuffer(&results[i]);
  }

  for (int j = 0; j < results[n_threads - 1].bytesRead; j++) { // last thread values
    if (new_length == 0) { // if no need to merge at end
      if (results[n_threads -1].enc[j].length == 1) {
        fputc(results[n_threads -1].enc[j].letter, stdout); // print letter only
    }
      else {
        printf("%d", results[n_threads -1].enc[j].length); // print length 
        fputc(results[n_threads -1].enc[j].letter, stdout); // print letter
      }
    }
    else {
        new_length += results[n_threads -1].enc[j].length; // update new_length 
        printf("%d", new_length);  // print new length
        fputc(results[n_threads -1].enc[j].letter, stdout); // print letter
        new_length = 0; // reset
    }
  }

  // freeing...
  free(threads);
  free(args);
  free(results);
}

// ./prle filename
// cat filename | ./prle
int main(int argc, char *argv[]) {
  // int fd = 0;
  FILE *file = stdin; // get file from stdin
  if (argc == 2) { // unless argument count is 2 which means a file was supplied
    file = fopen(argv[1], "r"); // open and read the file
  }

  Buffer buf = readIntoBuffer(file); // create a buffer that will hold all the information stored in file
  fclose(file); // close file

  go(&buf); // go function
  freeBuffer(&buf); // freeing buffer
}