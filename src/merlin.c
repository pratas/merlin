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
#define MAX_LINE_SIZE 4096

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void WriteReadN(char *w, char *x, char *y, char *z){
  fprintf(stdout, "@%s\n", w);
  fprintf(stdout, "%s\n",  x);
  fprintf(stdout, "%s\n",  y);
  fprintf(stdout, "%s\n",  z);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void WriteRead(char *w, char *x, char *y, char *z){
  fprintf(stdout, "@%s", w);
  fprintf(stdout, "%s",  x);
  fprintf(stdout, "%s",  y);
  fprintf(stdout, "%s",  z);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Sort(char *fin, char *fou, char *fdx, int lossy, uint32_t mem, uint64_t 
n_lines){
  char fname[MAX_LINE_SIZE];
  sprintf(fname, "sort -T . -S %uM %s", mem, fin);
  FILE *FI = Popen(fname, "r");
  FILE *FX = Fopen(fdx, "w");
  FILE *FO = Fopen(fou, "w");
  char buffer[MAX_LINE_SIZE];

  fprintf(FX, "#MRL%"PRIu64"\n", n_lines);

  while(fgets(buffer, MAX_LINE_SIZE, FI)){
    fputs(buffer, FO);
    if(lossy == 0)
      fprintf(FX, "%s", strrchr(buffer, '\t') + 1);
    }

  pclose(FI);
  fclose(FO);
  fclose(FX);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void UnpackR(char *input_file_name){
  FILE *F = Fopen(input_file_name, "r");
  const char delim[2] = { ESCAPE, '\0' };

  for(;;){
    char line[MAX_LINE_SIZE * 4];
    if(fgets(line, MAX_LINE_SIZE * 4, F) == NULL)
      break;
    strtok(line, delim);
    char *y = strtok(NULL, delim);
    char *z = strtok(NULL, delim);
    char *w = strtok(NULL, delim);
    char *v = strtok(NULL, delim);
    WriteReadN(w, z, v, y);
    }

  fclose(F);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Unpack(char *fpname){
  FILE *F = Fopen(fpname, "r");
  const char delim[2] = { ESCAPE, '\0' };

  for(;;){
    char line[MAX_LINE_SIZE * 4];
    if(fgets(line, MAX_LINE_SIZE * 4, F) == NULL)
      break;    
    char *x = strtok(line, delim);
    char *y = strtok(NULL, delim);
    char *z = strtok(NULL, delim);
    char *w = strtok(NULL, delim);
    WriteReadN(z, y, w, x);
    }

  fclose(F);
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

void PrintIDR(uint32_t i, FILE *F){
  fprintf(F, "%u\t", i);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint64_t Pack(char *fpname, char *ipname){
  uint64_t i = 0;
  FILE *F = Fopen(fpname, "w");
  FILE *INPUT_FILE = Fopen(ipname, "r");

  for(;;){
    char header[MAX_LINE_SIZE];
    char bases[MAX_LINE_SIZE];
    char header2[MAX_LINE_SIZE];
    char scores[MAX_LINE_SIZE];

    int c = fgetc(INPUT_FILE);
    if(c == EOF) break;
    if(c != '@'){
      fprintf(stderr, "Error: failed to get the initial '@' character\n");
      exit(1);
      }

    if(fgets(header, MAX_LINE_SIZE, INPUT_FILE) == NULL) break;
    if(fgets(bases, MAX_LINE_SIZE, INPUT_FILE) == NULL) break;
    if(fgets(header2, MAX_LINE_SIZE, INPUT_FILE) == NULL) break;
    if(fgets(scores, MAX_LINE_SIZE, INPUT_FILE) == NULL) break;

    PrintStream(scores,  strlen((char *) scores),  F);
    PrintStream(bases,   strlen((char *) bases ),  F);
    PrintStream(header,  strlen((char *) header),  F);
    PrintStream(header2, strlen((char *) header2), F);
    PrintID(++i, F);
    }

  fclose(F);
  fclose(INPUT_FILE);
  return i;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void PackFrontIndex(char *input_file_name, char *index_file_name, char 
*output_file_name, int verbose){
  FILE *INPUT_FILE  = Fopen(input_file_name,  "r");
  FILE *INDEX_FILE  = Fopen(index_file_name,  "r");
  FILE *OUTPUT_FILE = Fopen(output_file_name, "w");
  uint64_t lines = 0;

  if(fgetc(INDEX_FILE) != '#' || fgetc(INDEX_FILE) != 'M' || fgetc(INDEX_FILE) 
  != 'R' || fgetc(INDEX_FILE) != 'L' || fscanf(INDEX_FILE, "%"PRIu64"", &lines) 
  != 1){
    fprintf(stderr, "Error: invalid index file!\n");
    exit(1);
    }

  if(verbose) fprintf(stderr, "[>] Number of reads: %"PRIu64"\n", lines);

  Read *Read = CreateRead();

  while(GetRead(INPUT_FILE, Read)){
    uint64_t num;
    if(fscanf(INDEX_FILE, "%"PRIu64"", &num) != 1){
      fprintf(stderr, "Error: the index file does not match!\n");
      exit(1);
      }
    PrintIDR(num, OUTPUT_FILE);
    PrintStream(Read->scores,   strlen((char *) Read->scores),  OUTPUT_FILE);
    PrintStream(Read->bases,    strlen((char *) Read->bases ),  OUTPUT_FILE);
    PrintStream(Read->header1,  strlen((char *) Read->header1), OUTPUT_FILE);
    PrintStream(Read->header2,  strlen((char *) Read->header2), OUTPUT_FILE);
    fprintf(OUTPUT_FILE, "\n");
    }

  FreeRead(Read);
  fclose(INPUT_FILE);
  fclose(INDEX_FILE);
  fclose(OUTPUT_FILE);
  }


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void SortWithIndex(char *input_file_name, char *output_file_name, uint32_t mem,
int verbose){
  char fname[MAX_LINE_SIZE];
  sprintf(fname, "sort -V -T . -S %uM %s", mem, input_file_name);
  FILE *INPUT_FILE  = Popen(fname,  "r");
  FILE *OUTPUT_FILE = Fopen(output_file_name, "w");
  char buffer[MAX_LINE_SIZE];

  while(fgets(buffer, MAX_LINE_SIZE, INPUT_FILE))
    fputs(buffer, OUTPUT_FILE);

  pclose(INPUT_FILE);
  fclose(OUTPUT_FILE);
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
  "  -m <memory>          maximum memory used in sort (in MB),          \n"
  "  -d <FILE>            unMERLIN (back to the original file),         \n"
  "                       note: <FILE> is <FILE>.mindex.                \n"
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
  uint32_t mem = 4096;

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

  if(verbose) fprintf(stderr, "[>] Running MERLIN ...\n");

  for(x = 1 ; x < argc ; ++x)
    if(!strcmp(argv[x], "-m")){
      mem = atol(argv[x+1]);
      break;
      }

  if(verbose) fprintf(stderr, "[>] Maximum memory used: %u MB\n", mem);

  if(ArgBin(0, argv, argc, "-d")){ // PREPARE FOR DECOMPRESSION

    int iarg = 0;
    for(x = 1 ; x < argc ; ++x)
      if(!strcmp(argv[x], "-d")){
        iarg = x + 1;
        break;
        }

    if(iarg == argc){
      fprintf(stderr, "[x] Error: invalid parameters!\n");
      exit(1);
      }

    if(verbose) fprintf(stderr, "[>] Reading index file: %s\n", argv[iarg]);

    char *f_mdpack_name = Cat(argv[argc-1], ".mdpack");
    char *f_mdsort_name = Cat(argv[argc-1], ".mdsort");

    if(verbose) fprintf(stderr, "[>] Packing ...\n");
    PackFrontIndex(argv[argc-1], argv[iarg], f_mdpack_name, verbose);
    if(verbose) fprintf(stderr, "[>] Sorting ...\n");
    SortWithIndex(f_mdpack_name, f_mdsort_name, mem, verbose);
    remove(f_mdpack_name);
    if(verbose) fprintf(stderr, "[>] Unpacking ...\n");
    UnpackR(f_mdsort_name);
    remove(f_mdsort_name);

    free(f_mdpack_name);
    free(f_mdsort_name);
    }
  else{ // PREPARE FOR TRANSFORMATION FOR IMPROVE COMPRESSION
   
    char *f_pack_name  = Cat(argv[argc-1], ".mpack");
    char *f_sort_name  = Cat(argv[argc-1], ".msort");
    char *f_index_name = Cat(argv[argc-1], ".mindex"); // FILE WITH INDEXES

    uint64_t n_lines = 0;
    if(ArgBin(0, argv, argc, "-l"))
      lossy = 1;

    if(verbose) fprintf(stderr, "[>] Packing ...\n");
    n_lines = Pack(f_pack_name, argv[argc-1]);
    if(verbose) fprintf(stderr, "[>] Sorting ...\n");
    Sort(f_pack_name, f_sort_name, f_index_name, lossy, mem, n_lines);
    remove(f_pack_name);
    if(verbose) fprintf(stderr, "[>] Unpacking ...\n");
    Unpack(f_sort_name);
    remove(f_sort_name);

    free(f_pack_name);
    free(f_sort_name);
    free(f_index_name);
    }
  
  if(verbose) fprintf(stderr, "[>] Done!\n");

  return EXIT_SUCCESS;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
