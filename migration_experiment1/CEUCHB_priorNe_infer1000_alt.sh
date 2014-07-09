#!/bin/bash
#$ -cwd
#$ -V
#$ -P bsg.prjb -q short.qb
#$ -e CEUCHB_priorNeNp1000alt_ErrFiles
#$ -o CEUCHB_priorNeNp1000alt_OutFiles
#$ -N CEUCHB_priorNeNp1000alt
#$ -t 1-15
#$ -j y

#rep=1
#rep=$(expr $SGE_TASK_ID )

source CEUCHB_priorNe_param.src

data=${fileprefix}".vcf"
echo ${data}
Np=1000
EM=15

prefix=CEUCHB_priorNe_${nsam}sampleNp${Np}
#mkdir ${prefix}



pattern="1*3+25*2+1*4+1*6"
tmax=8

infer_mig_pattern="-eM 0.02599 1 -eM 0.03736 1 -eM 0.05044 1 "

TASK=$(expr ${SGE_TASK_ID} )
outprefix=${prefix}_Splitrep${TASK}
pf-ARG -l 0 ${cmd} ${assign_haploid} 0 ${split} ${infer_mig_pattern} ${pop_struct} -EM ${EM} -Np ${Np} -o ${outprefix} -vcf ${data} -seed ${TASK} ${TASK} ${TASK}

# should add pattern to this!