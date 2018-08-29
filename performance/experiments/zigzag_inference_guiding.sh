# run simulation
# -j 12:   run 12 experiments in parallel
# --force: overwrite any results already in database (but do not overwrite existing data in data-zigzag/)
#
# Add "-c" to submit jobs to a compute cluster using qsub
python zigzag_inference_guiding.py --db zigzag.db --datapath data-zigzag -j 12 --force

# make plot
python zigzag_inference_guiding.py --db zigzag.db --datapath data-zigzag --plot