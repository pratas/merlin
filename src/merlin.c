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

#define ESCAPE 127

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int Compare(const void *a, const void *b){
  return strcmp (*(const char **) a, *(const char **) b); 
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Sort(char *fin, char *fou, char *fdx, int lossy){
  FILE *FI = fopen(fin, "r");
  FILE *FO = fopen(fou, "w");
  FILE *FX = fopen(fdx, "w");
  char **lines = NULL;
  size_t len = 0;
  ssize_t lines_size;
  uint32_t k = 0, x, max_k = 100000;

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
  uint8_t s = 0;
  uint32_t i, k, pos = 0, line = 0;
  BUF  *B = CreateBuffer(BUF_SIZE);
  Read *R = CreateRead(65536 + GUARD, 65535 + GUARD);
  FILE *F = fopen(fpname, "r");

  while((k = fread(B->buf, 1, B->size, F)))
    for(i = 0 ; i < k ; ++i){
      s = B->buf[i];
      switch(line){
        case 1:
          if(s == ESCAPE || s == '\n'){
            R->bases[pos++] = '\n';
            R->bases[pos]   = '\0';
            pos  = 0;
            line = 2;
            break;
            }
          else
            R->bases[pos++] = s;
        break;

        case 0:
          if(s == ESCAPE || s == '\n'){
            R->scores[pos++] = '\n';
            R->scores[pos]   = '\0';
            pos  = 0;
            line = 1;
            break;
            }
          else
            R->scores[pos++] = s;
        break;

        case 2:
          if(s == ESCAPE || s == '\n'){
            R->header1[pos++] = '\n';
            R->header1[pos]   = '\0';
            pos  = 0;
            line = 3;
            break;
            }
          else
            R->header1[pos++] = s;
        break;

        case 3:
          if(s == ESCAPE || s == '\n'){
            R->header2[pos++] = '\n';
            R->header2[pos]   = '\0';
            pos  = 0;
            line = 4;
            fprintf(stdout, "@%s", R->header1);
            fprintf(stdout, "%s",  R->bases);
            fprintf(stdout, "%s",  R->header2);
            fprintf(stdout, "%s",  R->scores);
            break;
            }
          else
            R->header2[pos++] = s;
        break;

        case 4:
          if(s == '\n'){
            pos  = 0;
            line = 0;
            break;
            }
        break;
        }
      }

  fclose(F);
  FreeRead(R);
  RemoveBuffer(B);
  return 1;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintStream(uint8_t *b, uint32_t n, FILE *F){
  int k;
  for(k = 0 ; k < n ; ++k)
    if(b[k] == '\n') 
      fputc(127, F);
    else 
      fputc(b[k], F);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PrintID(uint32_t i, FILE *F){
  fprintf(F, "\t%u\n", i);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Pack(char *fpname, char *ipname){
  uint32_t i = 0;
  FILE *F = fopen(fpname, "w");
  FILE *R = fopen(ipname, "r");
  Read *Read = CreateRead(65536 + GUARD, 65535 + GUARD);
  
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
  uint32_t i = 0;
  FILE *F = fopen(fpname, "w");
  FILE *R = fopen(ipname, "r");
  FILE *I = fopen(fsname, "r");
  Read *Read = CreateRead(65536 + GUARD, 65535 + GUARD);

  while(GetRead(R, Read)){

    char *line = NULL;
    size_t len = 0;
    if((getline(&line, &len, I)) == -1){
      fprintf(stderr, "Error: the index file does not match!\n");
      exit(1);
      }

    fprintf(F, "%s\t", line); // POSITION FOR SORTING INTO ORIGINAL ORDER

    PrintStream(Read->scores,   strlen((char *) Read->scores),  F);
    PrintStream(Read->bases,    strlen((char *) Read->bases ),  F);
    PrintStream(Read->header1,  strlen((char *) Read->header1), F);
    PrintStream(Read->header2,  strlen((char *) Read->header2), F);
    PrintID(++i, F);
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

  if(argc == 1 || argc > 3 || ArgBin(0, argv, argc, "-h")){
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
        iarg = x + 1;
        break;
        }

    if(iarg == argc){
      fprintf(stderr, "[x] Error: invalid parameters!\n");
      exit(1);
      }

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
