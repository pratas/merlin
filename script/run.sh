#!/bin/bash
rm fqz* REP* merlin* MINIMIZED* OUTPUT* DECOMPRESSED* FINAL* -fr
# GET MERLIN ==================================================================
rm -fr merlin/
git clone https://github.com/pratas/merlin.git
cd merlin/src/
make
cp MERLIN ../../
cd ../../
# GET FQZ_COMP ================================================================
rm -fr fqzcomp/ fqzcomp-4.6/ fqzcomp-4.6.tar.gz
SFURL="https://downloads.sourceforge.net/project/";
wget $SFURL/fqzcomp/fqzcomp-4.6.tar.gz
tar -xzf fqzcomp-4.6.tar.gz
mv fqzcomp-4.6/ fqzcomp/
cd fqzcomp/
make
cp fqz_comp ../
cd ..
#
# COMPRESSION =================================================================
#
# MERLIN
(time ./MERLIN -m 20G -v $1 > OUTPUT.fastq ) &> REP_MERLIN
(time bzip2 $1.mindex ) &> REP_BZIP2
# FQZ_COMP
(time ./fqz_comp -n2 -s5+ < OUTPUT.fastq > MINIMIZED.fqz ) &> REP_FQZ_COMP
(time ./fqz_comp -n2 -s5+ < $1 > MINIMIZED_SOLO.fqz ) &> REP_FQZ_COMP_SOLO
(time ./fqz_comp -d < MINIMIZED_SOLO.fqz > DECOMPRESSED_SOLO.fastq ) &> REP_DEC_SOLO
(time ./fqz_comp -d < MINIMIZED.fqz > DECOMPRESSED.fastq ) &> REP_DEC
#
(time bunzip2 $1.mindex.bz ) &> REP_BZIP2_DEC
(time ./MERLIN -v -m 20G -d $1.mindex DECOMPRESSED.fastq > FINAL.fastq ) &> REP_MERLIN_DEC
#

