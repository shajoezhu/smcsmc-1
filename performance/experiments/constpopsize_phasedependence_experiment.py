import experiment_base
import itertools

###################################################################################
#
# experiment:
#  estimate bias and variance, dependent on known or unknown ancestral information
#
###################################################################################


# parameters for this experiment
inference_reps = 10
seqlen = 50e6
emiters = 4
particles = [100,500,1000,5000]
phased = [True, False]
nsam = [4,8]


# name of this experiment
experiment_name = "constpopsize_4epochs_pahsedependence"


# class defining default population parameters
import test_const_pop_size
experiment_class = test_const_pop_size.TestConstPopSize_FourEpochs


# define the experiments
experiment_pars = [{'length':seqlen, 'simseed':simseed, 'infseed':infseed, 'numparticles':numparticles,
                    'phased':phased, 'nsam':nsam}
                   for (seqlen, simseed, infseed, numparticles, phased, nsam) in (
                           # repetitions with unique data
                           [(seqlen, 100+rep, 100+rep, np, ph, ns)
                            for rep, np, ph, ns in itertools.product(range(inference_reps), particles, phased, nsam)] )]


# run an experiment.  keyword parameters must match those in experiment_pars
def run_experiment( length, simseed, infseed, numparticles, phased, nsam ):
    label = "L{}_S{}_I{}_P{}_Ph{}_NSAM{}".format(int(length),simseed,infseed,numparticles,phased,nsam)
    if experiment_base.have_result( name=experiment_name,
                                    sequence_length=length,
                                    dataseed=simseed,
                                    infseed=infseed,
                                    np=numparticles,
                                    phased=phased,
                                    num_samples=nsam ):
        print "Skipping " + label
        return
    e = experiment_class( 'setUp' )  # fake test fn to keep TestCase.__init__ happy
    e.setUp( experiment_base.datapath + experiment_name )
    # set simulation parameters
    e.pop.num_samples = nsam
    e.pop.sequence_length = length
    e.pop.seed = (simseed,)
    e.pop.scrmpath = experiment_base.scrmpath
    e.phased = phased
    e.filename_disambiguator = label
    # set inference parameters
    e.seqlen = length
    e.em = emiters
    e.seed = (infseed,)
    e.np = numparticles
    e.bias_heights = None
    e.lag = 1
    e.smcsmcpath = experiment_base.smcsmcpath
    # perform inference and store results
    e.infer( case = simseed )
    e.resultsToMySQL( db = experiment_base.db )
    e.success = True   # remove files
    print "Done " + label
    return


    
if __name__ == "__main__":
    experiment_base.run_experiment = run_experiment
    experiment_base.main( experiment_name, experiment_pars )