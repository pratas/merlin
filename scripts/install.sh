#!/bin/bash
###############################################################################
#
INSTALL_MERLIN=1;
INSTALL_FQZCOMP=1;
INSTALL_QUIP=1;
INSTALL_DSRC=1;
INSTALL_SCALCE=1;
DOWNLOAD_FILES=1;
RUN_MERLIN=1;
RUN_BZIP2=1;
#
###############################################################################
# PROGRAMS INSTALLATION
#
# GET MERLIN ==================================================================
if [[ "$INSTALL_MERLIN" -eq "1" ]]; then
  rm -fr merlin*
  git clone https://github.com/pratas/merlin.git
  cd merlin/src/
  make
  cp MERLIN ../../
  cd ../../
fi
# GET FQZ_COMP ================================================================
if [[ "$INSTALL_FQZCOMP" -eq "1" ]]; then
  rm -fr fqzcomp*
  SFURL="https://downloads.sourceforge.net/project/";
  wget $SFURL/fqzcomp/fqzcomp-4.6.tar.gz
  tar -xzf fqzcomp-4.6.tar.gz
  mv fqzcomp-4.6/ fqzcomp/
  cd fqzcomp/
  make
  cp fqz_comp ../
  cd ..
fi
# GET QUIP ====================================================================
if [[ "$INSTALL_QUIP" -eq "1" ]]; then
  rm -fr quip*
  wget http://homes.cs.washington.edu/~dcjones/quip/quip-1.1.8.tar.gz
  tar -xzf quip-1.1.8.tar.gz
  mv quip-1.1.8/ quip/
  cd quip/
  ./configure
  cd src/
  make
  cp quip ../
  cd ../
  rm -f quip-1.1.8.tar.gz
fi
# GET SCALCE ==================================================================
if [[ "$INSTALL_SCALCE" -eq "1" ]]; then
  rm -fr scalce*
  git clone https://github.com/sfu-compbio/scalce.git
  cd scalce
  make download
  make
  cp scalce ../SCALCE
  cd ../
fi
#
###############################################################################
# FASTQ FILES DOWNLOAD
#
if [[ "$DOWNLOAD_FILES" -eq "1" ]]; then
  wget $EBI/fastq/ERR194/ERR194146/ERR194146_1.fastq.gz # 51x
  gunzip ERR194146_1.fastq.gz;
  wget $EBI/fastq/ERR194/ERR194146/ERR194146_2.fastq.gz # 51x
  gunzip ERR194146_2.fastq.gz;
  wget $EBI/fastq/ERR174/ERR174310/ERR174310_1.fastq.gz # 7x [MPEG standard]
  gunzip ERR174310_1.fastq.gz;
  wget $EBI/fastq/ERR174/ERR174310/ERR174310_2.fastq.gz # 7x [MPEG standard]
  gunzip ERR174310_2.fastq.gz;
fi
#
# OUTPUT:
#
# ERR194146_1.fastq 
# ERR194146_2.fastq 
# ERR174310_1.fastq 
# ERR174310_2.fastq
#
###############################################################################
# RUN MERLIN
#
if [[ "$RUN_MERLIN" -eq "1" ]]; then
MP=" -v -m 4096 "; # 4GB OF MAX MEMORY
  (time ./MERLIN $MP ERR174310_1.fastq > M-ERR174310_1.fq ) &> R_ML_ERR174310_1
  (time ./MERLIN $MP ERR174310_2.fastq > M-ERR174310_2.fq ) &> R_ML_ERR174310_2
  (time ./MERLIN $MP ERR194146_1.fastq > M-ERR194146_1.fq ) &> R_ML_ERR194146_1
  (time ./MERLIN $MP ERR194146_2.fastq > M-ERR194146_2.fq ) &> R_ML_ERR194146_2
fi
#
###############################################################################
# RUN BZIP2
#
if [[ "$RUN_BZIP2" -eq "1" ]]; then
  (time bzip2 -9 ERR174310_1.fq.mindex ) &> R_BZIP2_ERR174310_1
  (time bzip2 -9 ERR174310_2.fq.mindex ) &> R_BZIP2_ERR174310_2
  (time bzip2 -9 ERR194146_1.fq.mindex ) &> R_BZIP2_ERR194146_1
  (time bzip2 -9 ERR194146_2.fq.mindex ) &> R_BZIP2_ERR194146_2
fi
#
###############################################################################

(time ./fqz_comp -n2 -s5+ < OUTPUT.fastq > MINIMIZED.fqz ) &> REPORT_FQZ_COMPq



#
# DECOMPRESSION ===============================================================


