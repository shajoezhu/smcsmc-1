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


#include"coalevent.hpp"
/*!
 * \class Coalevent
 *  another constructor for rec_point, to be called from sample the next genealogy
 */ 


/*! \brief  Initialize Starevent */    
Coalevent::Coalevent( 
           size_t pop_i,
           //double start_time,
           //double end_time, 
           double opportunity_y,
           eventCode event_code, 
           double end_base) {
    this->init();    
    this->set_pop_i(pop_i);
    //this->set_start_height( start_time );
    //this->set_end_height ( end_time );
    this->set_opportunity_y( opportunity_y );
    this->set_event_state(event_code);
    this->set_end_base ( end_base );
    this->set_num_event ( this->event_state() == EVENT ? 1 : 0);
    }


void Coalevent::init(){
    this->set_pop_i(0);
    //this->set_start_height(0);
    //this->set_end_height(0);    
    this->set_num_event(0);
    this->set_opportunity_y(0);
    this->set_event_state( INIT_NULL );
    
    this->set_epoch_index ( 0 );
    this->set_end_base ( 0 );
    this->pointer_counter_ = 1;
    }


bool Coalevent::print_event(){
    EventRecorderdout << "";
    //<< setw(10) << this->start_height()  << " to " 
    //<< setw(10) << this->end_height()    << ", " 
    dout << setw(13) << this->opportunity()   << " opportunity for ";
    dout << ( ( this->event_state() == NOEVENT ) ? "potential" : to_string(this->num_event()) ) << " Coalescent, ";
    dout << " at base "<< this->end_base() ;
    dout << " at epoch "<< this->epoch_index() << endl;
    return true;
    }

bool Recombevent::print_event(){
    EventRecorderdout << "";
    //<< setw(10) << this->start_height()  << " to " 
    //<< setw(10) << this->end_height()    << ", " 
    dout << setw(13) << this->opportunity()   << " opportunity for ";
    dout << ( ( this->event_state() == NOEVENT ) ? "potential" : to_string(this->num_event()) ) << " Recombination, ";
    dout << "from base "<< this->start_base() ;
    dout << " to base "<< this->end_base() ;
    dout << " at epoch "<< this->epoch_index() << endl;
    return true;
    }

bool Migrevent::print_event(){
	EventRecorderdout << "";
        //<< setw(10) << this->start_height()  << " to " 
             //<< setw(10) << this->end_height()    << ", " 
    dout << setw(13) << this->opportunity()   << " opportunity for ";
    dout << ( ( this->event_state() == NOEVENT ) ? "potential": to_string(this->num_event()) ) << " Migration, ";
    dout << "from pop " << this->pop_i() << " to " << ( ( this->event_state() == NOEVENT ) ? "some other population" : to_string ( this->mig_pop() ) );
    //dout << setw(2)  << this->num_event()     << " migration, ";
        //if ( this->event_state() == NOEVENT ){
            //dout << "potetial migration, from pop " << this->pop_i() << " to some other population "; 
            //}
        //else {
            //dout << "from pop " << this->pop_i() << " to pop " << this->mig_pop(); 
            //}
    dout << endl;
    return true;
    }

