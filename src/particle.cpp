/*
 * smcsmc is short for particle filters for ancestral recombination graphs. 
 * This is a free software for demographic inference from genome data with particle filters. 
 * 
 * Copyright (C) 2013, 2014 Sha (Joe) Zhu and Gerton Lunter
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

#include"particle.hpp"
#include <climits> // INT_MAX

void ForestState::making_copies( int number_of_copies ){
    for ( size_t i = 0; i < this->ForestState_copies.size(); i++){
        this->ForestState_copies[i] = NULL;
    }
    this->ForestState_copies.clear(); // clear previous pointers to the new copies
    
    if ( number_of_copies == 0 ) return;
    
    this->ForestState_copies.push_back( this );
    
    if ( number_of_copies == 1 ) return;
    
    for (int ii = 2; ii <= number_of_copies; ii++) { 
        ForestState* new_copy_state = new ForestState( *this );
        // Give the new particle its own random generator (for multithreading)
        size_t new_seed = (size_t) this->random_generator_->sampleInt( INT_MAX );
        new_copy_state->random_generator_ = new MersenneTwister( new_seed , this->random_generator_->ff() ); 
        //new_copy_state->random_generator_ = new MersenneTwister( new_seed ); 
        new_copy_state->model_ = new Model( *this->model_);
        //dout << "new random seed is " << this->particles[old_state_index]->random_generator_->seed() + this->particles.size() << endl;
        this->ForestState_copies.push_back(new_copy_state);
        }
    assert( this->ForestState_copies.size() == (size_t)number_of_copies ); 
    }



/*! \brief Initialize a new ForestState 
 * @ingroup group_pf_init
 * */
ForestState::ForestState( Model* model, RandomGenerator* random_generator )
            :Forest( model, random_generator ) {
    /*! Initialize base of a new ForestState, then do nothing, other members will be initialized at an upper level */
    //this->init_EventContainers( model );    
	//this->buildInitialTree();    
    //cout << "this->TmrcaHistory.size() = "<<this->TmrcaHistory.size()<<endl;
    // Initialize the initial particles at base 0, with equal weights
  	//this->setParticleWeight( 1.0 );
	//this->setSiteWhereWeightWasUpdated(0.0);
    new_forest_counter++; // DEBUG
    }


/*! \brief Create a newly copied ForestState 
    @ingroup group_pf_resample 
*/
ForestState::ForestState( const ForestState & copied_state )
            :Forest( copied_state ) {
    this->setParticleWeight( copied_state.weight() );
	this->setSiteWhereWeightWasUpdated( copied_state.site_where_weight_was_updated() );    
    this->setAncestor ( copied_state.ancestor() );
    this->copyEventContainers ( copied_state );
    this->segment_count_ = copied_state.segment_count_;
    
    for (size_t i = 0 ; i < copied_state.TmrcaHistory.size(); i++ ){
        this->TmrcaHistory.push_back(copied_state.TmrcaHistory[i]);
        }        
	dout << "current particle's weight is " << this->weight()<<endl;
    new_forest_counter++;  // DEBUG
    }
    
    

void ForestState::copyEventContainers(const ForestState & copied_state ){
    // Copy Coalescent events
    for (size_t i = 0 ; i < copied_state.CoaleventContainer.size() ; i++ ){ 
        deque < EvolutionaryEvent* > CoaleventContainer_i;   
        for (size_t ii = 0 ; ii < copied_state.CoaleventContainer[i].size(); ii++){
            EvolutionaryEvent* new_coalevent = copied_state.CoaleventContainer[i][ii];
            new_coalevent->increase_refcount();
            CoaleventContainer_i.push_back ( new_coalevent ) ;
            }
        CoaleventContainer.push_back( CoaleventContainer_i );        
        }
    // Copy Recombination events
    for (size_t i = 0 ; i < copied_state.RecombeventContainer.size() ; i++ ){ 
        deque < EvolutionaryEvent* > RecombeventContainer_i;  
        for (size_t ii = 0 ; ii < copied_state.RecombeventContainer[i].size(); ii++){
            EvolutionaryEvent* new_recombevent = copied_state.RecombeventContainer[i][ii];
            new_recombevent->increase_refcount();
            RecombeventContainer_i.push_back (new_recombevent) ;
            }
        RecombeventContainer.push_back( RecombeventContainer_i );        
        }
    // Copy Migration events
    for (size_t i = 0 ; i < copied_state.MigreventContainer.size() ; i++ ){ 
        deque < EvolutionaryEvent* > MigreventContainer_i;   /*!< \brief Coalescent events recorder */
        for (size_t ii = 0 ; ii < copied_state.MigreventContainer[i].size(); ii++){
            EvolutionaryEvent* new_migrevent = copied_state.MigreventContainer[i][ii];
            new_migrevent->increase_refcount();
            MigreventContainer_i.push_back (new_migrevent) ;
            }
        MigreventContainer.push_back( MigreventContainer_i );        
        }    
    }



void ForestState::init_EventContainers( Model * model ){
    for (size_t i = 0 ; i < model->change_times_.size() ; i++ ){ 
        deque < EvolutionaryEvent*  > CoaleventContainer_i;   /*!< \brief Coalescent events recorder */
        CoaleventContainer.push_back( CoaleventContainer_i );        
        deque < EvolutionaryEvent* > RecombeventContainer_i; /*!< \brief Recombination events recorder */
        RecombeventContainer.push_back( RecombeventContainer_i );
        deque < EvolutionaryEvent* > MigreventContainer_i;   /*!< \brief Migration events recorder */
        MigreventContainer.push_back( MigreventContainer_i );                
        }
    }



/*! \brief Destructor of ForestState
 * Recursively remove all the previous states, if the pointer counter is zero
 */
ForestState::~ForestState(){
	this->clear_CoaleventContainer();   
	this->clear_RecombeventContainer(); 
    this->clear_MigreventContainer();    
    delete this->random_generator_;     //MULTITRHREADING
    delete this->model_;
    delete_forest_counter++;
	dout << "A Foreststate is deleted" << endl;
    }


void ForestState::record_all_event(TimeInterval const &ti, double &recomb_opp_x_within_scrm){

    dout << endl;
    ForestStatedout << "Start Recording " << endl;

    double recomb_opp_x_within_smcsmc = 0; // DEBUG
	double start_base, end_base;
	double start_height, end_height;

    // find extent of time interval where an event may have occurred
    start_height = ti.start_height();
    if (this->tmp_event_.isNoEvent()) {
		end_height = start_height + ti.length();
	} else {
		end_height = this->tmp_event_.time();
		assert (end_height > start_height);
	}

	// loop over the two nodes
	for (int i=0; i<2; i++) {
		
		if (states_[i] == 2) {
            // node i is tracing an existing non-local branch; opportunities for recombination
            start_base = active_node(i)->last_update();
            end_base = this->current_base();
			if (end_base == start_base) continue;
			double recomb_pos = -1;
			if (tmp_event_.isRecombination())
				recomb_pos = (start_base + end_base)/2;   // we should sample from [start,end], but the data isn't used
			this->record_Recombevent_pos( start_height, end_height, recomb_pos, 1, start_base, end_base );
            recomb_opp_x_within_smcsmc += end_base - start_base;
		} else if (states_[i] == 1) {
            // node i is tracing out a new branch; opportunities for coalescences and migration
            start_base = this->current_base();
            int weight = ti.numberOfContemporaries( active_node(i)->population() );
			// consider only normal (not pairwise) coalescences that occurred on this node
	        bool coal_event = (tmp_event_.isCoalescence() && tmp_event_.active_node_nr() == i);
			this->record_Coalevent( active_node(i)->population(), start_height, end_height, weight, coal_event, start_base );
			// consider migration opportunity and events
			if (this->model().population_number()>1) {
				this->record_Migrevent( active_node(i)->population(), start_height, end_height, tmp_event_.isMigration(), tmp_event_.mig_pop(), start_base );
			}
		}
	}
	
	assert ( recomb_opp_x_within_smcsmc == recomb_opp_x_within_scrm );
    
    // only coalescences into contemporaries were considered; pairwise coalescence between active nodes could also occur
    if ((states_[0] == 1) && (states_[1] == 1) && (active_node(0)->population() == active_node(1)->population() ) ) {
		start_base = this->current_base();
		int weight = 1;
		this->record_Coalevent( active_node(0)->population(), start_height, end_height, weight, tmp_event_.isPwCoalescence(), start_base );
	}

    dout << endl;        
    return;
}

/*! \brief Record Coalescent events
* @ingroup group_count_coal
*/
void ForestState::record_Coalevent(
                  size_t pop_i,
                  double start_time, 
                  double end_time, 
                  int weight, 
                  bool event,
                  double end_base ) {
    //cout<<"current_time_index = " << this->writable_model()->current_time_idx_<<endl;
    EvolutionaryEvent * new_event = new EvolutionaryEvent(start_time, end_time, end_base, pop_i, weight);
    if (event) {
		new_event->set_coal_event();
	}
    assert(new_event->print_event());
    this->CoaleventContainer[this->writable_model()->current_time_idx_].push_back(new_event);
}


/*! \brief Record Recombination events
* @ingroup group_count_coal
*/
void ForestState::record_Recombevent_time(
                          double start_time, 
                          double end_time,
                          double recomb_time,   // this is not used for inference, but could be.  <0 = no event
                          int weight, 
                          double start_base, 
                          double end_base){
    
    EvolutionaryEvent* new_event = new EvolutionaryEvent( start_time, end_time, start_base, end_base, weight );
    if (recomb_time >= 0) {
		dout << "Recording event at time-wise recombination opportunity from" << start_time << " to " << end_time << " with event at " << recomb_time << endl;
		new_event->set_recomb_event_time( recomb_time );
		recombination_counter++; // DEBUG
	}
    assert(new_event->print_event());
    this->RecombeventContainer[this->writable_model()->current_time_idx_].push_back(new_event);
}


void ForestState::record_Recombevent_pos(
                          double start_time, 
                          double end_time,
                          double recomb_base,   // this is not used for inference, but could be.  <0 = no event
                          int weight, 
                          double start_base, 
                          double end_base){
    
    EvolutionaryEvent* new_event = new EvolutionaryEvent( start_time, end_time, start_base, end_base, weight );
    if (recomb_base >= 0) {
		new_event->set_recomb_event_pos( recomb_base );
		recombination_counter++; // DEBUG
	}
    assert(new_event->print_event());
    this->RecombeventContainer[this->writable_model()->current_time_idx_].push_back(new_event);
}


void ForestState::record_Recombevent_b4_extension (){ 
    #ifdef _RecombRecordingOff // if the recombination recording off macro is defined, then return without recording the event
        return;
    #endif
    
    dout << endl;
    ForestStatedout << " Build new genealogy, compute the recombination opportunity at each level " << endl;

    // iterate over time intervals (but do NOT prune branches at this stage)
    for (TimeIntervalIterator ti(this, this->nodes_.at(0), false); ti.good(); ++ti) {
        ForestStatedout << " * * Time interval: " << (*ti).start_height() << " - " << (*ti).end_height() << " " ;
        dout << ", with " << ti.numberOfLocalContemporaries() << " local Contemporaries, " << ti.numberOfLocalContemporaries() << " * " << "( " << (*ti).end_height() << " - " << (*ti).start_height() << ")" << std::endl;
        // Create a recombination event for this slice (which may be smaller than an epoch -- but in our case it usually won't be)
        record_Recombevent_time( (*ti).start_height(), (*ti).end_height(), -1, ti.numberOfLocalContemporaries(), this->current_base(), this->next_base_ );
    }
}


void ForestState::record_Recombevent_atNewGenealogy ( double event_height ){
    #ifdef _RecombRecordingOff // if the recombination recording off macro is defined, then return without recording the event
        return;
    #endif
    recombination_counter++; // DEBUG
    this->writable_model()->resetTime( event_height );
    size_t epoch_i = this->writable_model()->current_time_idx_;
    dout << "current_time_idx_ =  " << epoch_i << " = [" << this->writable_model()->getCurrentTime() << "," << this->writable_model()->getNextTime() << endl;
    // find the EvolutionaryEvent to add this event to.  This is (now) necessary because the events have been split within an epoch
    int idx = this->RecombeventContainer[epoch_i].size();
    while (true) {
		--idx;
		assert (idx >= 0);
		assert (this->RecombeventContainer[epoch_i][idx]->is_recomb());
		if (this->RecombeventContainer[epoch_i][idx]->start_height > event_height) continue;
		if (this->RecombeventContainer[epoch_i][idx]->end_height < event_height) continue;
		assert (this->RecombeventContainer[epoch_i][idx]->start_base() == this->current_base());
		break;
	}
    // assign the recombination event to the start of this coalescent opportunity segment
    this->RecombeventContainer[epoch_i][idx]->set_recomb_event_time( event_height );
}


/*! \brief Record Migration events
* @ingroup group_count_coal
*/
void ForestState::record_Migrevent(size_t pop_i,
								   double start_time, 
								   double end_time, 
                                   bool event, 
                                   size_t mig_pop, 
                                   double end_base) {

	int weight=1;
    EvolutionaryEvent* new_event = new EvolutionaryEvent(start_time, end_time, end_base, pop_i, weight);
	if (event) {
		new_event->set_migr_event( mig_pop );
	}
    assert(new_event->print_event());
	this->MigreventContainer[this->writable_model()->current_time_idx_].push_back(new_event);
}


/*! Clear coalescent and recombination events recorded between two states.*/
void ForestState::clear_CoaleventContainer(){ 
    for (size_t time_i = 0; time_i < this->CoaleventContainer.size(); time_i++ ){
        //cout << "            this->CoaleventContainer[time_i].size() = " << this->CoaleventContainer[time_i].size() <<endl;
        for (size_t i=0; i < this->CoaleventContainer[time_i].size(); i++){
            if (CoaleventContainer[time_i][i]->decrease_refcount_is_zero()) {
                delete CoaleventContainer[time_i][i];
                }
            }
        }
    }


/*! Clear recombination events recorded between two states.*/
void ForestState::clear_RecombeventContainer(){ 
    for (size_t time_i = 0; time_i < this->RecombeventContainer.size(); time_i++){
        //cout << "            this->RecombeventContainer[time_i].size() = " << this->RecombeventContainer[time_i].size() <<endl;
        for (size_t i=0; i < this->RecombeventContainer[time_i].size(); i++){
            if (RecombeventContainer[time_i][i]->decrease_refcount_is_zero()){
                delete RecombeventContainer[time_i][i];
                }
            }
        }
    }
    
    
/*! Clear migration events recorded between two states.*/
void ForestState::clear_MigreventContainer(){ 
    for (size_t time_i = 0; time_i < this->MigreventContainer.size(); time_i++){
        for (size_t i=0; i < this->MigreventContainer[time_i].size(); i++){
            if ( MigreventContainer[time_i][i]->decrease_refcount_is_zero() ) {
                delete MigreventContainer[time_i][i];
                }
            }
        }
    }
    

void ForestState::include_haplotypes_at_tips(vector <int> &haplotypes_at_tips){
	for (size_t j=0; j < haplotypes_at_tips.size();j++){		
		this->nodes()->at(j)->set_mutation_state(haplotypes_at_tips[j]);
	    }
    }


/*! 
 * Calculate the marginal likelihood of a node recursively, let X denote the state of Node *, and Y denote the state of the 
 *  first child, and Z denote the state of the second child. Let t1 denote the time from X to Y, and t2 is the time from X to Z.
 *  Let u(t) be the probability function of the no mutation occurs within time t. 
 * 
  \verbatim
      X
   t1/ \ t2
    Y   Z
  \endverbatim
 * 
 * 	Suppose that X, Y and Z only take values 0 or 1.
 *  When X=0, consider the likelihood of the tree in four cases of (y,z) pairs (0,0), (0,1), (1,0) and (1,1)
 *  Similarliy X=1, we have four cases too. 
 * X[0]=y[0] * ut1 * z[0] * ut2 + y[0] * ut1 * z[1] * (1-ut2) + y[1] * (1-ut1) * z[0] * ut2 + y[1] * (1-ut1) * z[1] * (1-ut2);
 * X[1]=y[1] * ut1 * z[1] * ut2 + y[1] * ut1 * z[0] * (1-ut2) + y[0] * (1-ut1) * z[1] * ut2 + y[0] * (1-ut1) * z[0] * (1-ut2);
 * 
 * After simplification, 
 * X[0]=(y[0]*ut1 + y[1]*(1-ut1)) * (z[0]*ut2 + z[1]*(1-ut2)) 
 * X[1]=(y[1]*ut1 + y[0]*(1-ut1)) * (z[1]*ut2 + z[0]*(1-ut2)) 
 * 
 * Note: even though there are only two states, x[0]+x[1] is not 1!!! becasue x[0] is a marginal probability, but still conditional on all the possiblities of the variants 
 *  
 *  If X, Y and Z take values A, T, C and G. The computation would be more complex.  Four equations, with 16 terms in each.
 * 
 * If we consider JC69 model, then 
 * X[0]= y[0] * ut1 * z[0] * ut2  + y[0] * ut1 * sum(z[1,2,3]) * (1-ut2) +  sum(y[1,2,3]) * (1-ut1) * z[0] * ut2 + sum(y[1,2,3]) * (1-ut1) * sum(z[1,2,3]) * (1-ut2) 
 * ...
 * 
 * @ingroup group_resample
 * * */


inline valarray<double> ForestState::cal_marginal_likelihood_infinite(Node * node) {

	valarray<double> x(2);
	//dout << "subtree at " << node << " first child is " << node->first_child() <<" second child is " <<  node->second_child()<<endl;

	// deal with the case that this node is a leaf node
	if ( node->first_child() == NULL ) {
	  assert ( node->second_child() == NULL );     // if a node has no first child, it won't have a second child
	  assert ( node->label() > 0 );                // we only traverse the local tree, therefore the leaf node must be contemporary
	  x[0] = node->mutation_state() == 1 ? 0.0 : 1.0;	// also encode state==-1 (missing data) as 1.0
	  x[1] = node->mutation_state() == 0 ? 0.0 : 1.0;   // also encode state==-1 (missing data) as 1.0
	  //dout << "Marginal probability at " << node->label() << " is " << x[0] << "," << x[1] << endl;
	  return x;
    }

        // Genealogy branch lengths are in number of generations, the mutation rate is unit of per site per generation, often in the magnitude of 10 to the power of negative 8.
	double mutation_rate = this->model().mutation_rate();
	Node *left = trackLocalNode(node->first_child());   // jz_stable
	Node *right = trackLocalNode(node->second_child()); // jz_stable
    //Node *left = node->getLocalChild1();  // jz
    //Node *right = node->getLocalChild2(); // jz
	double t1 = node->height() - left->height();
	double t2 = node->height() - right->height();
    double ut1 = 1 - exp(-t1 * mutation_rate); // assume infinite site
	double ut2 = 1 - exp(-t2 * mutation_rate); // assume infinite site
	assert (ut1>=0 && ut1<=1);
        assert (ut2>=0 && ut2<=1);
	valarray<double> y = cal_marginal_likelihood_infinite(left);
	valarray<double> z = cal_marginal_likelihood_infinite(right);		
	x[0] = (y[0] * ut1 + y[1] * (1-ut1) ) * (z[0] * ut2 + z[1] * (1-ut2) );
	x[1] = (y[0] * (1-ut1) + y[1] * ut1 ) * (z[0] * (1-ut2) + z[1] * ut2 );
	//dout << "Marginal probability at " << node->label() << " is " << x[0] << "," << x[1] << endl;
	return x;
}

	
/*! 
 * \brief Calculate the likelihood of the genealogy at data site i, 
 *  If there is no data given at the site i, return likelihood as 1. Since all particles at this site are equally probable 
 * @ingroup group_pf_resample
 */
double ForestState::calculate_likelihood( ) {
    //dout << "calculate_likelihood function, root is " << this->local_root() << endl;
    valarray<double> marginal_likelihood = cal_marginal_likelihood_infinite(this->local_root());
    dout << "marginal likelihood is " << marginal_likelihood[0] <<  "," << marginal_likelihood[1] << endl;
    double prior[2] = {0.5,0.5};
    double likelihood = marginal_likelihood[0]*prior[0] + marginal_likelihood[1]*prior[1];
    dout << "ForestState::calculate_likelihood( ) likelihood is " << likelihood << endl;
    return likelihood;
}


double ForestState::extend_ARG ( double mutation_rate, double extend_to, Segment_State segment_state, bool updateWeight, bool recordEvents ) {

    double updated_to = this->site_where_weight_was_updated();
    dout << "Particle current base is at " << this->current_base() << " weight is updated to " << updated_to <<endl;
    assert (updated_to >= this->current_base());
    double likelihood = 1.0;
    
    while ( updated_to < extend_to ) {
        
        dout << "  Now at " <<this->current_base()<< " updated_to " << updated_to << " and extending to " << extend_to << endl;            
        /*!
         * First, update the likelihood up to either extend_to or the end of this state
         */
        double update_to = min( extend_to, this->next_base() );
        double length_of_local_tree = this->getLocalTreeLength(); // in generations
        double likelihood_of_segment = ( segment_state == SEGMENT_INVARIANT ) ? exp( -mutation_rate * length_of_local_tree * (update_to - updated_to) ) : 1.0 ;// assume infinite site model
        dout << " Likelihood of no mutations in segment of length " << (update_to - updated_to) << " is " << likelihood_of_segment ;
        dout << ( ( segment_state == SEGMENT_INVARIANT ) ? ", as invariant.": ", as missing data" ) << endl;
        likelihood *= likelihood_of_segment;
        updated_to = update_to;  // rescues the invariant
        /*!
         * Next, if we haven't reached extend_to now, add a new state and iterate
         */
        if ( updated_to < extend_to ) {
            this->sampleNextGenealogy( recordEvents );

            //if ( this->heat_bool_ ){
                //TmrcaState tmrca( this->site_where_weight_was_updated(), this->local_root()->height() );
                //this->TmrcaHistory.push_back ( tmrca );
            //}
            
        }
        
    }
    assert (updated_to == extend_to);        
    if (updateWeight) {
        this->setParticleWeight( this->weight() * likelihood );
    }
    this->setSiteWhereWeightWasUpdated( extend_to );
    return likelihood;
}



std::string ForestState::newick(Node *node) {
  if(node->in_sample()){
    std::ostringstream label_strm;
    label_strm<<node->label();
    return label_strm.str();
  }
  else{
    Node *left = this->trackLocalNode(node->first_child());
    double t1 = node->height() - left->height();
    std::ostringstream t1_strm;
    t1_strm << t1 / (4 * this->model().default_pop_size);

    Node *right = this->trackLocalNode(node->second_child());
    double t2 = node->height() - right->height();
    std::ostringstream t2_strm;
    t2_strm << t2 / (4 * this->model().default_pop_size);

    return "("+this->newick(left)+":"+t1_strm.str()+","+ this->newick(right)+":"+t2_strm.str() +")";
  }
}
