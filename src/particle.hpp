/*
 * smcsmc is short for particle filters for ancestral recombination graphs.
 * This is a free software for demographic inference from genome data with particle filters.
 *
 * Copyright (C) 2013-2017 Donna Henderson, Sha (Joe) Zhu and Gerton Lunter
 *
 * This file is part of smcsmc.
 *
 * smcsmc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "forest.h"
#include "coalevent.hpp"
#include <deque>
#include <valarray>
#include "segdata.hpp"
#include <queue>
#include <cmath>

#ifndef NDEBUG
#define ForestStatedout (std::cout << "    ForestState ")
#else
#pragma GCC diagnostic ignored "-Wunused-value"
#define ForestStatedout 0 && (std::cout << "    ForestState ")
#endif

#ifndef NDEBUG
#define newForestdout (std::cout << "    NEW FOREST ")
#else
#pragma GCC diagnostic ignored "-Wunused-value"
#define newForestdout 0 && (std::cout << "    NEW FOREST ")
#endif

#pragma GCC diagnostic ignored "-Wsign-compare"

#ifndef PARTICLE
#define PARTICLE

extern int new_forest_counter;
extern int delete_forest_counter;
extern int recombination_counter; //DEBUG
extern int recombination_event_called; //DEBUG

struct BranchLengthData {
    BranchLengthData (double partialBranchLength = 0, double subtreeBranchLength = -1)
    {
        this->partialBranchLength = partialBranchLength;
        this->subtreeBranchLength = subtreeBranchLength;
    }
    ~BranchLengthData(){};
    // this holds the branch length of the partial tree consisting of all leaf nodes
    // that carry data, up to the current node.
    double partialBranchLength;
    // this holds the branch length of the subtree subtending all leaf nodes that carry
    // data, but not including the branch from the subtree's root to the current node
    // Special case: if no descendants of the current node carry data, this is -1.
    double subtreeBranchLength;
};

// machinery for delayedIS

class DelayedFactor {
    
public:
    DelayedFactor(double pos, double factor) : application_position(pos), importance_factor(factor) {}
    
    double application_position;
    double importance_factor;

    void print_info() const {
        std::clog << "\nThe application position of this DF is " << this->application_position << std::endl;
        std::clog << "The importance factor of this DF is " << this->importance_factor << std::endl;
    }
};

class CompareDFs {
public:
    bool operator() (const DelayedFactor &lhs, const DelayedFactor &rhs) const {
        return lhs.application_position > rhs.application_position;
    }
};


/*!
 * \brief Derived class from Forest.
 * A particle is a special case of a Forest, with special members
 */
class ForestState : public Forest{
#ifdef UNITTEST
    friend class TestForestState;
#endif
    friend class ParticleContainer;
    friend class CountModel;

    // All members and methods are private
private:
    // Constructor, called at initial stage //
    ForestState(Model* model, RandomGenerator* random_generator, const vector<int>& record_event_in_epoch,
                bool own_model_and_random_generator);    /*!< \brief Used to create a ForestState for the first time */
    ForestState(const ForestState &current_state);       /*!< \brief Copy constructor, used when resampling */

    // Destructors //
    ~ForestState();
    void clear_eventContainer();
    
    // Resampling //
    void init_EventContainers( Model * model );
    void copyEventContainers(const ForestState & copied_state );
    void resample_recombination_position(void);
    
    // Update weight
    void include_haplotypes_at_tips(vector <int> &haplotypes_at_tips); /*!< \brief Update data to the particle */
    double calculate_likelihood( ); /*!< \brief Calculate the likelihood of the genealogy */
    valarray<double> cal_partial_likelihood_infinite(Node * node); /*!< Calculate the marginal likelihood of each node */
    double trackLocalTreeBranchLength();
    BranchLengthData trackSubtreeBranchLength ( Node * currentNode );
    
    // Extend
    double extend_ARG ( double mutation_rate, double extend_to, bool updateWeight=true, bool recordEvents=true );
    
    // Record events
    void record_Recombevent_b4_extension ( );
    void record_Recombevent_atNewGenealogy ( double event_height );
    void record_all_event(TimeIntervalIterator const &ti, double & recomb_opp_x_within_scrm);
    void clear_recomb_opp_within_scrm(){ this->recomb_opp_x_within_scrm = 0 ; }
    
    // Setters and getters:
    void setSiteWhereWeightWasUpdated( double site ){ this->site_where_weight_was_updated_=site; }
    double site_where_weight_was_updated() const { return site_where_weight_was_updated_; }
    void setPosteriorWeight(double weight) { assert (weight > 0.0); this->posterior_weight_ = weight; }
    void setPilotWeight(double weight) { this->pilot_weight_ = weight; }
    double posteriorWeight() const { return this->posterior_weight_; }
    double pilotWeight() const { return this->pilot_weight_; }

    // Biased sampling
    void adjustWeights(double adjustment) {
        posterior_weight_ *= adjustment;
        pilot_weight_ *= adjustment;
    }
    void adjustWeightsWithDelay(double adjustment, double delay) {
        posterior_weight_ *= adjustment;
        total_delayed_adjustment_ *= adjustment;
        delayed_adjustments.push( DelayedFactor ( current_base() + delay, adjustment ) );
    }
    void applyDelayedAdjustment() {
        double importance_factor = delayed_adjustments.top().importance_factor;
        delayed_adjustments.pop();
        total_delayed_adjustment_ /= importance_factor;
        pilot_weight_ *= importance_factor;
        assert( std::abs(posterior_weight_ - pilot_weight_ * total_delayed_adjustment_) <= .001 * posterior_weight_ );
    }
    
    //// biased sampling    
    double imp_weight_simulation_to_pilot_dist_over_segment( double previously_updated_to, double update_to,
                                                             bool sampled_recombination);
    void IS_TreePoint_adjustor( const TreePoint & tp );
    
    double compute_positional_component_of_transitional_prob_of_no_recombination( double sequence_distance_without_rec );
    double compute_positional_component_of_proposal_prob_of_no_recombination( double sequence_distance_without_rec );
    double compute_positional_component_of_transitional_prob_of_recombination( double sequence_distance_without_rec );
    double compute_positional_component_of_proposal_prob_of_recombination( double sequence_distance_without_rec );
    
    TreePoint sampleBiasedPoint(Node* node = NULL, double length_left = -1);
    void sampleBiasedRecSeqPosition(bool recordEvents);
    
    double getWeightedLocalTreeLength() const;
    double getWeightedLengthBelow( Node* node ) const;
    double WeightedBranchLengthAbove( Node* node ) const;
    double WeightedToUnweightedHeightAbove( Node* node, double length_left) const;

    // What does this do?
    Node* trackLocalNode(Node *node) const;

    // below are overloaded
    void sampleRecSeqPosition( bool recordEvents = false );
    
    // Debugging tools
    std::string newick(Node *node) ;
        
    // Members
    vector < EvolutionaryEvent* > eventTrees;
    double site_where_weight_was_updated_;
    double posterior_weight_;
    double pilot_weight_;
    bool owning_model_and_random_generator;
    const vector < int >& record_event_in_epoch;
    std::priority_queue<DelayedFactor, std::vector<DelayedFactor>, CompareDFs > delayed_adjustments;
    double total_delayed_adjustment_;
    
};
#endif
