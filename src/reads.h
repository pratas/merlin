#ifndef READS_H_INCLUDED
#define READS_H_INCLUDED

#include "defs.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct{
  char       *header1;
  char       *bases;
  char       *header2;
  char       *scores;
  uint64_t   position;
  uint64_t   headerMaxSize;
  uint64_t   readMaxSize;
  uint64_t   solidData;
  uint64_t   header2E;
  uint64_t   skipNs;
  uint8_t    lowestScore;
  }
Read;

typedef struct{
  uint64_t   id;
  uint64_t   size;
  Read       *Read;
  }
READS;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint8_t  PseudoDna2Solid  (uint8_t);
uint8_t  Solid2PseudoDna  (uint8_t);
Read     *CreateRead      (void);
void     InitRead         (Read *);
void     FreeRead         (Read *);
Read     *GetRead         (FILE *, Read *);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif
