#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "misc.h"
#include "mem.h"
#include "reads.h"
#include "args.h"
#include "buffer.h"

#define ESCAPE 9 // 9 ASCII = TAB
#define MAX_BLOCK 100000

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int Compare(const void *a, const void *b){
  return strcmp (*(const char **) a, *(const char **) b); 
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int SortByPosition(const void *a, const void *b){
  Read *xA = (Read *) a;
  Read *xB = (Read *) b;
  return xB->position-xA->position;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Sort(char *fin, char *fou, char *fdx, int lossy, uint64_t bsize, uint64_t 
n_lines){
  FILE *FI = fopen(fin, "r");
  FILE *FO = fopen(fou, "w");
  FILE *FX = fopen(fdx, "w");
  char **lines = NULL;
  size_t len = 0;
  ssize_t lines_size;
  uint64_t k = 0, x, max_k = bsize;

  lines = (char **) Malloc(max_k * sizeof(char *));

  fprintf(FX, "#MRL%"PRIu64":%"PRIu64"\n", max_k, n_lines);

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

uint64_t Pack(char *fpname, char *ipname){
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

  return i;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PackWithIndex(char *fsname, char *ipname, int verbose){
  FILE *R = fopen(ipname, "r");
  FILE *I = fopen(fsname, "r");
  uint64_t size = 0, idx = 0, lines = 0, k = 0, x = 0;

  if(fgetc(I) != '#' || fgetc(I) != 'M' || fgetc(I) != 'R' || fgetc(I) != 'L' || 
  fscanf(I, "%"PRIu64":%"PRIu64"", &size, &lines) != 2){
    fprintf(stderr, "Error: invalid index file!\n");
    exit(1);
    }

  if(verbose){
    fprintf(stderr, "[>] Block line size: %"PRIu64"\n", size);
    fprintf(stderr, "[>] Number of reads: %"PRIu64"\n", lines);
    }

  Read **Reads = (Read **) Malloc((size+1) * sizeof(Read *));

  for(;;){
    k = 0;
    Reads[k] = CreateRead();

    while(GetRead(R, Reads[k]) && k < size){

      if(fscanf(I, "%"PRIu64"", &Reads[k]->position) != 1){
        fprintf(stderr, "Error: the index file does not match!\n");
        exit(1);
        }
      //qsort(Reads, k+1, sizeof(Read), SortByPosition);
      Reads[++k] = CreateRead();
      ++idx;
      }

    // FIXME: SORT IS NOT WORKING
    qsort(Reads, k, sizeof(Read), SortByPosition);

    for(x = 0 ; x < k ; ++x){
      fprintf(stdout, "@%s", Reads[x]->header1);
      fprintf(stdout, "%s",  Reads[x]->bases);
      fprintf(stdout, "%s",  Reads[x]->header2);
      fprintf(stdout, "%s",  Reads[x]->scores);
      }

    if(idx >= lines)
      break;
    }

  for(x = 0 ; x < size ; ++x)
    free(Reads[x]);
  free(Reads);

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
  "  -b <size>            block size for sorting,                       \n"
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
  int x, lossy = 0, verbose = 0;

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

    int iarg = 0;
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
    PackWithIndex(argv[iarg], argv[argc-1], verbose);
    }
  else{ // PREPARE FOR COMPRESSION

    uint64_t b_size = 0, n_lines = 0;
    for(x = 1 ; x < argc ; ++x)
      if(strcmp(argv[x], "-b")){
        b_size = atol(argv[x + 1]);
        break;
        }

    if(b_size == 0) b_size = MAX_BLOCK;

    if(ArgBin(0, argv, argc, "-l"))
      lossy = 1;

    n_lines = Pack(f_pack_name, argv[argc-1]);
    Sort(f_pack_name, f_sort_name, f_index_name, lossy, b_size, n_lines);
    Unpack(f_sort_name);
    remove(f_sort_name);
    remove(f_pack_name);
    // OUTPUT: <STDOUT> & <FILE>.mindex
    }
  
  free(f_pack_name); 
  free(f_sort_name); 
  free(f_index_name); 
  if(verbose) fprintf(stderr, "[>] Done!\n");

  return EXIT_SUCCESS;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
