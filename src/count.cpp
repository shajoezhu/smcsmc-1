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

#include"count.hpp"

void CountModel::init(){
    this->init_coal_and_recomb();
    this->init_migr();
    this->init_lags();
        
    this->update_param_interval_  = 5e6; // ONLINE EM
    this->update_param_threshold_ = 1e7; // ONLINE EM
    return ;
    }


void CountModel::init_coal_and_recomb(){
    this->total_coal_count.clear();
    this->total_weighted_coal_opportunity.clear();
    this->total_recomb_count.clear();
    this->total_weighted_recomb_opportunity.clear();

    this->resetTime();    
    for (size_t epoch_idx = 0 ; epoch_idx < change_times_.size(); epoch_idx++){
        vector <Two_doubles> tmp_count(this->population_number(), Two_doubles(0));
        this->total_coal_count.push_back(tmp_count);
        this->total_recomb_count.push_back(tmp_count);
        for (size_t pop_i = 0 ; pop_i < this->population_number(); pop_i++ ){
            this->total_coal_count[epoch_idx][pop_i] = 1 / ( 2 * population_size() );
        }
        
        vector <Two_doubles> tmp_opportunity(this->population_number(), Two_doubles(1));
        this->total_weighted_coal_opportunity.push_back(tmp_opportunity);
        this->total_weighted_recomb_opportunity.push_back(tmp_opportunity);
        }
    this->resetTime();        
    }


void CountModel::init_migr(){ /*! \todo This requires more work*/
    this->total_mig_count.clear();
    this->total_weighted_mig_opportunity.clear();    

    this->resetTime();    
    for (size_t epoch_idx = 0 ; epoch_idx < change_times_.size(); epoch_idx++){
        vector < vector < Two_doubles > > tmp_count_Time_i;
        vector < Two_doubles > tmp_opp_Time_i;
        vector < vector < double > > tmp_count_Time_i_double;
        for (size_t pop_i = 0 ; pop_i < this->population_number(); pop_i++ ){
            vector <Two_doubles> tmp_count(this->population_number(), Two_doubles());
            tmp_count_Time_i.push_back(tmp_count);
            vector <double> tmp_count_Time_i_double(this->population_number(), 0);
            tmp_opp_Time_i.push_back( Two_doubles(1) );
            }        
        this->total_mig_count.push_back(tmp_count_Time_i);
        this->total_weighted_mig_opportunity.push_back(tmp_opp_Time_i);
        this->inferred_mig_rate.push_back(tmp_count_Time_i_double);
        }
    }


void CountModel::init_lags(){
    this->counted_to.clear();
    this->lags.clear();        
    this->resetTime();    
    for (size_t epoch_idx = 0 ; epoch_idx < change_times_.size(); epoch_idx++){
        this->counted_to.push_back( (double)0 );
        double top_t = epoch_idx == (change_times_.size() -1) ? change_times_[change_times_.size()-1] : change_times_[epoch_idx+1];
        //double lag_i =  double(4) / this->recombination_rate() / top_t ; 
        double lag_i = this->const_lag_ > 0 ? this->const_lag_ : double(4) / (this->recombination_rate() * top_t) ; 
        this->lags.push_back( lag_i );
        }    
    this->resetTime();
    }


void CountModel::initialize_mig_rate ( vector <vector<double>*> & rates_list ){
    for (size_t i = 0; i < rates_list.size(); i++ ){
        if (rates_list[i]){
            for (size_t j = 0; j < rates_list[i]->size() ; j++){
                rates_list[i]->at(j) = 0;
                }
            }    
        }
    }
    
    
void CountModel::reset_Ne ( Model *model ){
    model->resetTime();
    this ->resetTime();
    for (size_t epoch_idx = 0; epoch_idx < change_times_.size(); epoch_idx++){
        for (size_t pop_j = 0 ; pop_j < this->population_number(); pop_j++ ){
            //model->addPopulationSize(this->change_times_[epoch_idx], pop_j, this->total_weighted_coal_opportunity[epoch_idx][pop_j] / this->total_coal_count[epoch_idx][pop_j] /2 ,false, false);    
            this->total_weighted_coal_opportunity[epoch_idx][pop_j].compute_final_answer();
            this->total_coal_count[epoch_idx][pop_j].compute_final_answer();
            double tmp_pop_size = this->total_weighted_coal_opportunity[epoch_idx][pop_j].final_answer() / this->total_coal_count[epoch_idx][pop_j].final_answer() / (double)2;
            //tmp_pop_size = roundf(tmp_pop_size * (double)1e6)/(double)1e6; // Rounding, Gerton: this is not a good idea
            model->addPopulationSize(this->change_times_[epoch_idx], pop_j, tmp_pop_size ,false, false);    
            cout << " popsize is equal to " << tmp_pop_size << " ( "<<this->total_weighted_coal_opportunity[epoch_idx][pop_j].final_answer()<<" / "<< this->total_coal_count[epoch_idx][pop_j].final_answer() << "/2)" <<endl;
            }
        }
    this->check_model_updated_Ne( model );
    }


void CountModel::reset_recomb_rate ( Model *model ){
    this->compute_recomb_rate();
    //this->inferred_recomb_rate = roundf ( this->inferred_recomb_rate * (double)1e16)/(double)1e16; // Rounding, Gerton: this is not a good idea
    model->setRecombinationRate( this->inferred_recomb_rate , false, false, 0);
    cout << " set recombination rate " << model->recombination_rate(0) << "("<<this->recomb_count_<<" / "<< this->recomb_opportunity_ << ")" <<endl;
    }


void CountModel::reset_mig_rate ( Model *model ) {
    if (this->has_migration() == false) return;
    this->compute_mig_rate();
    
    assert( this->print_mig_rate (model->mig_rates_list_) );
    assert( this->print_mig_rate (model->total_mig_rates_list_) );

    this->initialize_mig_rate ( model->mig_rates_list_ );
    this->initialize_mig_rate ( model->total_mig_rates_list_ );
    assert( this->print_mig_rate (model->mig_rates_list_) );
    assert( this->print_mig_rate (model->total_mig_rates_list_) );

    model->resetTime();
    this ->resetTime();
    for (size_t epoch_idx = 0; epoch_idx < change_times_.size(); epoch_idx++){
        for (size_t pop_j = 0 ; pop_j < this->population_number(); pop_j++ ){
            for (size_t pop_k = 0 ; pop_k < this->population_number(); pop_k++ ) {
                if ( pop_j == pop_k) continue;
                model->addMigrationRate(this->change_times_[epoch_idx], pop_j, pop_k, this->inferred_mig_rate[epoch_idx][pop_j][pop_k], false, false);
                }
            }
        }
    this->check_model_updated_mig (model);
    
    assert( this->print_mig_rate (model->mig_rates_list_) );
    assert( this->print_mig_rate (model->total_mig_rates_list_) );
    }

//void CountModel::reset_single_mig_rate ( Model * model ){
    //this->print_mig_rate (model->single_mig_probs_list_) ;// DEBUG
    //}

void CountModel::reset_model_parameters(double current_base, Model * model, bool online, bool force_update, bool print){
    
    bool update = false;
    if ( online && current_base > this->update_param_threshold_ ){
        update = true;
        this->update_param_threshold_ += this->update_param_interval_;
        }
                    
    if ( update || force_update  ){ 
        cout<<" MODEL IS RESET at base " << current_base <<endl;
        this->reset_recomb_rate ( model );
        this->reset_Ne ( model );
        this->reset_mig_rate ( model );
        //this->reset_single_mig_rate ( model );
        model->finalize();
        } 
    else {
        if (print){
            this->print_coal_count();            
            }
        return;
        }
    }


void CountModel::log_counts( PfParam& param ) {
	// log coalescent counts
	for (size_t epoch_idx = 0; epoch_idx < change_times_.size(); epoch_idx++ ) {
		for (size_t pop_idx = 0; pop_idx < this->population_number(); pop_idx++ ) {
			param.append_to_count_file( epoch_idx, "Coal", pop_idx, -1, this->total_weighted_coal_opportunity[epoch_idx][pop_idx].final_answer(), 
																	    this->total_coal_count[epoch_idx][pop_idx].final_answer() );
		}
	}
	// log recombination counts
	for (size_t epoch_idx = 0; epoch_idx < change_times_.size(); epoch_idx++ ) {
		param.append_to_count_file( epoch_idx, "Recomb", -1, -1, this->total_weighted_recomb_opportunity[epoch_idx][0].final_answer(), 
																 this->total_recomb_count[epoch_idx][0].final_answer() );
	}
	// log migration counts
	for (size_t epoch_idx = 0; epoch_idx < change_times_.size(); epoch_idx++ ) {
		for (size_t from_pop_idx = 0; from_pop_idx < this->population_number(); from_pop_idx++ ) {
			for (size_t to_pop_idx = 0; to_pop_idx < this->population_number(); to_pop_idx++ ) {
				if (from_pop_idx != to_pop_idx) {
					param.append_to_count_file( epoch_idx, "Migr", from_pop_idx, to_pop_idx, this->total_weighted_mig_opportunity[epoch_idx][from_pop_idx].final_answer(),
																						     this->total_mig_count[epoch_idx][from_pop_idx][to_pop_idx].final_answer() );
				}
			}
		}
	}
}

    

void CountModel::compute_mig_rate(){
    this->resetTime();
    for (size_t epoch_idx = 0; epoch_idx<change_times_.size(); epoch_idx++){
        for (size_t pop_i = 0 ; pop_i < this->population_number(); pop_i++ ){
			
			this->total_weighted_mig_opportunity[epoch_idx][pop_i].compute_final_answer();
            
            for (size_t pop_j = 0 ; pop_j < this->population_number(); pop_j++ ){
				this->total_mig_count[epoch_idx][pop_i][pop_j].compute_final_answer();
                this->inferred_mig_rate[epoch_idx][pop_i][pop_j] = this->total_mig_count[epoch_idx][pop_i][pop_j].final_answer() / this->total_weighted_mig_opportunity[epoch_idx][pop_i].final_answer();
                //cout<<"this->inferred_mig_rate["<<epoch_idx<<"]["<<pop_i<<"]["<<pop_j<<"] = " << this->inferred_mig_rate[epoch_idx][pop_i][pop_j]<<endl;
            }
        }
    }
}


/*! 
 * \brief Compute the recombination rate once we have sweeped through all the data, and recorded the recomb_opportunity and recomb_counts
 */ 
void CountModel::compute_recomb_rate () {
    this->recomb_opportunity_ = 0;
    this->recomb_count_ = 0;
    for ( size_t epoch_idx = 0; epoch_idx < change_times_.size(); epoch_idx++ ){
		this->total_weighted_recomb_opportunity[epoch_idx][0].compute_final_answer();
		this->total_recomb_count[epoch_idx][0].compute_final_answer();
        this->recomb_opportunity_ += this->total_weighted_recomb_opportunity[epoch_idx][0].final_answer() ;
        this->recomb_count_ += this->total_recomb_count[epoch_idx][0].final_answer();
    }
    
    this->inferred_recomb_rate = this->recomb_count_ / this->recomb_opportunity_;
    //cout << "Recombination rate is " << this->inferred_recomb_rate << " ( " << this->recomb_count_ << " / " << this->recomb_opportunity_ << " )"<<endl;
}

                
                    /*! \verbatim 
                            xstart     
                            .                      xend                         VCFfile->site()
                            .                      .                            .
                            .                      .     3                      .
                            .                      .     x---o              6   .
                            .                  2   .     |   |              x-------o
                            .                  x---------o   |              |   .
                            .                  |   .         |              |   .
                         0  .                  |   .         x---o          |   .
                         x---------o           |   .         4   |          |   .
                            .      |           |   .             x----------o   .
                            .      |           |   .             5              .
                            .      x-----------o   .                            .
                            .      1               .-------------lag------------.
                            .                      .                            .
                     \endverbatim
                     * 
                     * Count the coalescent events between position xstart and xend.
                     * 
                     * At the beginning of this function, the tail ForestState is at 
                     * state 6, whose weight represents the weight for the entire particle
                     * As lagging is applied, we need to skip a few states before start counting. 
                     *  
                     * In this example, only count the coalescent events occured on states 1 and 2.
                     */ 
            
        

//void CountModel::update_star_count( deque < Starevent *> & StareventContainer_i, double weight, size_t x_end, vector<double>& total_star_count, vector<double>& total_star_opportunity ) {
    //// Go through the events, starting from the leftmost and going up to x_end, and add events (weighted by weight) to the appropriate counters
    //// When processed remove the event pointer from the deque; remove the event itself if its reference count becomes 0
    //while (StareventContainer_i.size() > 0 && StareventContainer_i[0]->base() <= x_end) { // DEBUG changed "<" to "<=" ???
        //Starevent * current_Starevent = StareventContainer_i[0];
        //total_star_count[current_Starevent->pop_i()]       += weight * current_Starevent->num_event();
        //total_star_opportunity[current_Starevent->pop_i()] += weight * current_Starevent->opportunity();            
        //current_Starevent->pointer_counter_ --;
        //if (current_Starevent->pointer_counter_ == 0) {
            //delete current_Starevent;
            //}
        //StareventContainer_i.pop_front();
        //}
    //}
//void update_coalescent_count( deque < Coalevent *> & CoaleventContainer_i, double weight, size_t x_end, vector<double>& total_coal_count, vector<double>& total_coal_opportunity ) ;

void CountModel::update_coalescent_count( deque < Coalevent *> & CoaleventContainer_i, double weight, double x_end, vector<Two_doubles>& total_coal_count, vector<Two_doubles>& total_coal_opportunity ){
    // Go through the events, starting from the leftmost and going up to x_end, and add events (weighted by weight) to the appropriate counters
    // When processed remove the event pointer from the deque; remove the event itself if its reference count becomes 0
    while (CoaleventContainer_i.size() > 0 && CoaleventContainer_i[0]->end_base() <= x_end) { // DEBUG changed "<" to "<=" ???
        Coalevent * current_Coalevent = CoaleventContainer_i[0];
        total_coal_count[current_Coalevent->pop_i()].add( weight * current_Coalevent->num_event() );
        total_coal_opportunity[current_Coalevent->pop_i()].add( weight * current_Coalevent->opportunity() );
        current_Coalevent->pointer_counter_ --;
        if (current_Coalevent->pointer_counter_ == 0) {
            delete current_Coalevent;
            }
        CoaleventContainer_i.pop_front();
        }
    }

void CountModel::update_recombination_count( deque < Recombevent *> & RecombeventContainer_i, double weight, double x_start, double x_end, vector<Two_doubles>& total_recomb_count, vector<Two_doubles>& total_recomb_opportunity ){
     
    // Go through the events, starting from the leftmost and going up to x_end, and add events (weighted by weight) to the appropriate counters
    // When processed remove the event pointer from the deque; remove the event itself if its reference count becomes 0

    if ( RecombeventContainer_i.size() == 0 ) cout << "                        finished  RecombeventContainer_i.size() == 0 "<<endl;
    if ( RecombeventContainer_i.size() == 0 ) return;
    // First process all recombination event segments that lie fully to the left of x_end
    
    assert ( RecombeventContainer_i.size() > 0 && RecombeventContainer_i[0]->start_base() <= x_start && RecombeventContainer_i[0]->end_base() > x_start );
    
    while ( RecombeventContainer_i.size() > 0 ) {
        Recombevent * current_Recombevent = RecombeventContainer_i[0];
        // test if the recombination event (at the left of the segment) needs to be included
        if ( current_Recombevent->start_base() >= x_start && current_Recombevent->start_base() < x_end ) {
            total_recomb_count[current_Recombevent->pop_i()].add( weight * current_Recombevent->num_event() );
        }
        // only include the recombination opportunity that overlaps the interval [x_start, x_end]
        double start = max( current_Recombevent->start_base(), (double)x_start );
        double end   = min( current_Recombevent->end_base(),   (double)x_end );
        //cerr.precision(15);
        //cerr << weight << "\t" << current_Recombevent->opportunity_between( start , end ) <<endl;
        total_recomb_opportunity[ current_Recombevent->pop_i() ].add( weight * current_Recombevent->opportunity_between( start , end ) );

        // if this segment extends beyond x_end, we're done
        if ( current_Recombevent->end_base() > (double)x_end ) 
            break;

        // we have completely processed this recombination event / opportunity, so remove it
        current_Recombevent->pointer_counter_ --;
        if (current_Recombevent->pointer_counter_ == 0) {
            delete current_Recombevent;
        }
        RecombeventContainer_i.pop_front();
    }  
}

void CountModel::update_migration_count( deque < Migrevent *> & MigreventContainer_i, double weight, double x_end, size_t epoch_idx ) {
    // Go through the events, starting from the leftmost and going up to x_end, and add events (weighted by weight) to the appropriate counters
    // When processed remove the event pointer from the deque; remove the event itself if its reference count becomes 0
    while (MigreventContainer_i.size() > 0 && MigreventContainer_i[0]->end_base() < x_end) {
        Migrevent * current_Migrevent = MigreventContainer_i[0];
        if (current_Migrevent->event_state() == EVENT) {
            total_mig_count[epoch_idx][current_Migrevent->pop_i()][current_Migrevent->mig_pop()].add( weight * current_Migrevent->num_event() );
        }
        total_weighted_mig_opportunity[epoch_idx][current_Migrevent->pop_i()].add( weight * current_Migrevent->opportunity() );
        current_Migrevent->pointer_counter_ --;
        if (current_Migrevent->pointer_counter_ == 0) {
            delete current_Migrevent;
            }
        MigreventContainer_i.pop_front();
        }
    }

