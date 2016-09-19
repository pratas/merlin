#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "misc.h"
#include "mem.h"
#include "reads.h"
#include "args.h"
#include "buffer.h"

#define ESCAPE 9
#define MAX_BLOCK 100000

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int Compare(const void *a, const void *b){
  return strcmp (*(const char **) a, *(const char **) b); 
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int SortByPosition(const void *a, const void *b){
  Read *ia = (Read *) a;
  Read *ib = (Read *) b;
  if     (ia->position < ib->position) return -1;
  else if(ia->position > ib->position) return 1;
  else                                 return 0;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Sort(char *fin, char *fou, char *fdx, int lossy){
  FILE *FI = fopen(fin, "r");
  FILE *FO = fopen(fou, "w");
  FILE *FX = fopen(fdx, "w");
  char **lines = NULL;
  size_t len = 0;
  ssize_t lines_size;
  uint32_t k = 0, x, max_k = MAX_BLOCK;

  lines = (char **) Malloc(max_k * sizeof(char *));
  for(;;){
    while((lines_size = getline(&lines[k], &len, FI)) != -1 && k != max_k){
      qsort(lines, ++k, sizeof(char *), Compare);
      }

    for(x = 0 ; x < k ; ++x){
      fprintf(FO, "%s", lines[x]);
      if(lossy == 0)
        fprintf(FX, "%s", strrchr(lines[x], '\t') + 1);
      }

    if(lines_size == -1)
      break;

    for(x = 0 ; x < k ; ++x)
      free(lines[x]);
 
    k = 0;
    }

  for(x = 0 ; x < k ; ++x)
    free(lines[x]);
  free(lines);

  fclose(FI);
  fclose(FO);
  fclose(FX);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int Unpack(char *fpname){
  FILE *F = fopen(fpname, "r");
  ssize_t ls;
  size_t len;
  char *line = NULL;
  const char delim[1] = { ESCAPE };

  while((ls = getline(&line, &len, F)) != -1){
    //fprintf(stderr, "%s\n", line);
    char *x = strtok(line, delim);
    char *y = strtok(NULL, delim);
    char *z = strtok(NULL, delim);
    char *w = strtok(NULL, delim);
    fprintf(stdout, "@%s\n", z);
    fprintf(stdout, "%s\n",  y);
    fprintf(stdout, "%s\n",  w);
    fprintf(stdout, "%s\n",  x);
    }

  free(line);
  fclose(F);
  return 1;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintStream(char *b, uint32_t n, FILE *F){
  int k;
  for(k = 0 ; k < n ; ++k)
    if(b[k] != '\0' && b[k] != '\n')
      fputc(b[k], F);
  fputc(ESCAPE, F);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintID(uint32_t i, FILE *F){
  fprintf(F, "\t%u\n", i);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Pack(char *fpname, char *ipname){
  uint64_t i = 0;
  FILE *F = fopen(fpname, "w");
  FILE *R = fopen(ipname, "r");
  Read *Read = CreateRead();
  
  while(GetRead(R, Read)){
    PrintStream(Read->scores,   strlen((char *) Read->scores),  F);
    PrintStream(Read->bases,    strlen((char *) Read->bases ),  F);
    PrintStream(Read->header1,  strlen((char *) Read->header1), F);
    PrintStream(Read->header2,  strlen((char *) Read->header2), F);
    PrintID(++i, F);
    }

  FreeRead(Read);
  fclose(F);
  fclose(R);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PackWithIndex(char *fpname, char *fsname, char *ipname){
  FILE *F = fopen(fpname, "w");
  FILE *R = fopen(ipname, "r");
  FILE *I = fopen(fsname, "r");
  Read *Read = CreateRead();

  while(GetRead(R, Read)){

    uint64_t position;
    if(fscanf(I, "%"PRIu64"", &position) != 1){
      fprintf(stderr, "Error: the index file does not match!\n");
      exit(1);
      }

    Read->position = position;

    // TODO: SORT BY POSITION

//    qsort(Read, T->size, sizeof(Read), SortByPosition);

    fprintf(F, "%"PRIu64"\t", position); // POSITION FOR SORTING INTO ORIGINAL ORDER
    PrintStream(Read->scores,   strlen((char *) Read->scores),  F);
    PrintStream(Read->bases,    strlen((char *) Read->bases ),  F);
    PrintStream(Read->header1,  strlen((char *) Read->header1), F);
    PrintStream(Read->header2,  strlen((char *) Read->header2), F);
    }

  FreeRead(Read);
  fclose(F);
  fclose(R);
  fclose(I);
  }


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintMenu(void){ 
  fprintf(stderr,
  "Usage: MERLIN [OPTION]... [FILE] > [STDOUT]                          \n"
  "Compression boost for any FASTQ compressing tool.                    \n"
  "                                                                     \n"
  "Non-mandatory arguments:                                             \n"
  "                                                                     \n"
  "  -h                   give this help,                               \n"
  "  -V                   display version number,                       \n"
  "  -v                   verbose mode (more information),              \n"
  "  -l                   lossy (does not store read order),            \n"
  "  -d <FILE>            unMERLIN (back to the original file),         \n"
  "                                                                     \n"
  "Mandatory arguments:                                                 \n"
  "                                                                     \n"
  "  [FILE]               input filename,                               \n"
  "  > [FILE]             stdout filename,                              \n"
  "                                                                     \n"
  "Report issues to <{pratas,ap}@ua.pt>.                                \n");
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintVersion(void){
  fprintf(stderr,
  "                                                                         \n"
  "                          ==================                             \n"
  "                          |   MERLIN %u.%u   |                           \n"
  "                          ==================                             \n"
  "                                                                         \n"
  "           Compression boost for any FASTQ compressing tool.             \n"
  "                                                                         \n"
  "Copyright (C) 2016-2017 University of Aveiro. This is a Free software.   \n"
  "You may redistribute copies of it under the terms of the GNU - General   \n"
  "Public License v3 <http://www.gnu.org/licenses/gpl.html>. There is NOT   \n"
  "ANY WARRANTY, to the extent permitted by law. Developed and written by   \n"
  "Diogo Pratas & Armando J. Pinho                                        \n\n", 
  VERSION, RELEASE);
  }


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main(int argc, char *argv[]){
  int lossy = 0, verbose = 0;

  if(argc == 1 || argc > 6 || ArgBin(0, argv, argc, "-h")){
    PrintMenu();
    return EXIT_SUCCESS;
    }

  if(ArgBin(0, argv, argc, "-V")){
    PrintVersion();
    return EXIT_SUCCESS;
    }

  if(ArgBin(0, argv, argc, "-v"))
    verbose = 1;
  
  char *f_pack_name  = Cat(argv[argc-1], ".mpack");
  char *f_sort_name  = Cat(argv[argc-1], ".msort");
  char *f_index_name = Cat(argv[argc-1], ".mindex");

  if(verbose) fprintf(stderr, "[>] Running MERLIN ...\n");

  if(ArgBin(0, argv, argc, "-d")){ // PREPARE FOR DECOMPRESSION

    int iarg = 0, x;
    for(x = 1 ; x < argc ; ++x)
      if(strcmp(argv[x], "-d")){
        iarg = x + 2;
        break;
        }

    if(iarg == argc){
      fprintf(stderr, "[x] Error: invalid parameters!\n");
      exit(1);
      }

    if(verbose) fprintf(stderr, "[>] Reading index file: %s\n", argv[iarg]);
    PackWithIndex(f_pack_name, argv[iarg], argv[argc-1]);
    // Sort(f_pack_name, f_sort_name, f_index_name, lossy);
    Unpack(f_sort_name);
    }
  else{ // PREPARE FOR COMPRESSION

    if(ArgBin(0, argv, argc, "-l"))
      lossy = 1;

    Pack(f_pack_name, argv[argc-1]);
    Sort(f_pack_name, f_sort_name, f_index_name, lossy);
    Unpack(f_sort_name);
    remove(f_pack_name);
    // OUTPUT: <FILE>.msort <FILE>.mindex
    }
  
  free(f_pack_name); 
  free(f_sort_name); 
  free(f_index_name); 
  if(verbose) fprintf(stderr, "[>] Done!\n");

  return EXIT_SUCCESS;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
