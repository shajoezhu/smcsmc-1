from __future__ import print_function

import unittest
import os
import subprocess

from test_generic import *
import populationmodels



#
# TestTwoSample is a fragile test -- expect the test to fail
# if the random seed is changed.  This is because the test is
# run over very little data (10 Mb)
#

class TestTwoSample(TestGeneric):

    # Called by unittest.main when TestTwoSample is created.
    # Responsible for per-class creation of simulated data.
    # It is implemented as a class method to avoid simulating the same data multiple times
    @classmethod
    def setUpClass(cls):
        cls.prefix = "2sample"
        cls.segfile = cls.prefix + ".seg"
        cls.seqlen = 1e7
        cls.scrmpath = "../scrm"
        cls.pop = populationmodels.Pop1( filename = cls.segfile, sequence_length = cls.seqlen, scrmpath=cls.scrmpath )
        cls.success = True   # set ourselves up for success

        print ("simulating for",cls.prefix,"...")
        cls.pop.simulate()

    # remove simulated data
    @classmethod
    def tearDownClass(cls):
        if cls.segfile != None and cls.success:
            os.unlink( cls.segfile )
            cls.filename = None


    def readResults(self):
        results = {'Coal':[],    # type -> epoch -> {'start' -> # ,'end' -> # ,'estimates' -> iteration -> # }
                   'Recomb':[]}
        for line in open(self.outfile):
            elts = line.strip().split()
            if elts[0] == "Iter": continue
            it, epoch, start, end, typ, ne = elts[0],elts[1],elts[2],elts[3],elts[4],elts[10]
            if int(epoch) == -1:
                # recombination rate is assigned to epoch -1
                epoch = 0
            if len(results[typ]) <= int(epoch):
                results[typ].append( {'start': float(start),
                                      'end': float(end),
                                      'estimates': []} )
            assert( len(results[typ][int(epoch)]['estimates']) == int(it) )
            results[typ][int(epoch)]['estimates'].append( float(ne) )
        return results

    # actual test
    def test_inference(self):
        self.em = 4
        self.np = 1000
        self.seed = (3647837471,)
        self.infer( case = 1 )

        # Ne target ranges for the varous epochs
        #target_min = [0,     1000, 1000, 2000, 4000, 4500, 7000, 6000, 7000, 6000, 5000, 5000, 6000, 7000, 9000, 9500, 11000]
        #target_max = [1e+10, 2000, 2000, 4000, 5000, 5500, 9000, 7000, 9000, 8000, 7000, 7000, 7500, 8500, 11000, 11500, 13000]
        target_min = [0,     1000, 1000, 2000, 4000, 4500, 7000, 6000, 7000, 6000, 5000, 5000, 6000, 7000, 9000, 9500, 9000]
        target_max = [1e+10, 2000, 2000, 4000, 5000, 6000, 9000, 7000, 9000, 8000, 7000, 7000, 7500, 8500, 11500, 11500, 13000]
        results = self.readResults()

        out_of_range = 0
        for epoch, emresults in enumerate(results['Coal']):
            # get the last EM estimate
            result = emresults['estimates'][-1]

            # see if it is within range
            print ("Checking epoch",epoch,result,target_min[epoch],target_max[epoch])
            #self.assertTrue( result >= target_min[epoch] )
            #self.assertTrue( result <= target_max[epoch] )
            if result < target_min[epoch] or result > target_max[epoch]:
                print ("Out of range!")
                out_of_range += 1

        self.assertTrue( out_of_range < 3 )
        self.success = True


if __name__ == "__main__":
    unittest.main()