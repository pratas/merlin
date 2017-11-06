#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reads.h"
#include "mem.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// BASE TO SOLID SYMBOL
//
uint8_t PseudoDna2Solid(uint8_t s){
  switch(s){
    case 'A': return '0';
    case 'C': return '1';
    case 'G': return '2';
    case 'T': return '3';
    case 'N': return '.';
    default :  
      fprintf(stderr, "Error: unexpected base value '%c'\n", s);
      exit(1);
    }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// SOLID SYMBOL TO BASE
//
uint8_t Solid2PseudoDna(uint8_t s){
  switch(s){
    case '0': return 'A';
    case '1': return 'C';
    case '2': return 'G';
    case '3': return 'T';
    case '.': return 'N';
    default: 
      fprintf(stderr, "Error: unexpected cs value '%c'\n", s);
      exit(1);
    }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// EOF ERROR WARNING AND PROGRAM TERMINATE
//
static void UEOF(void){
  fprintf(stderr, "Error: unexpected end of file\n");
  exit(1);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CREATE READ
//
Read *CreateRead(void){
  Read *R        = (Read *) Calloc(1, sizeof(Read));
  R->solidData   = 0;
  R->header2E    = 0;
  R->skipNs      = 0;
  R->lowestScore = (uint8_t) 255;
  return R;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void InitRead(Read *R){
  R->solidData   = 0;
  R->header2E    = 0;
  R->skipNs      = 0;
  R->lowestScore = (uint8_t) 255;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CREATE READ
//
void FreeRead(Read *R){
  free(R->header1);
  free(R->bases);
  free(R->header2);
  free(R->scores);
  free(R);
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GET READ
//
Read *GetRead(FILE *F, Read *R){
  int64_t n, c = fgetc(F);
  size_t len = 0;
  ssize_t ls;

  if(c == EOF) return NULL;
  if(c != '@'){
    fprintf(stderr, "Error: failed to get the initial '@' character\n");
    exit(1);
    }
  
  if((ls = getline(&R->header1, &len, F)) == -1) UEOF();
  if((ls = getline(&R->bases,   &len, F)) == -1) UEOF();
  if((ls = getline(&R->header2, &len, F)) == -1) UEOF();
  if((ls = getline(&R->scores,  &len, F)) == -1) UEOF();
   
 
  if(R->solidData){
    n = 1;
    while(R->bases[n] != '\n'){
      R->bases[n] = Solid2PseudoDna(R->bases[n]);
      n++;
      }
    }

  return R;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

