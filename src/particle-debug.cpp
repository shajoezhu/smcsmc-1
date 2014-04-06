/*
 * pf-ARG is short for particle filters for ancestral recombination graphs. 
 * This is a free software for demographic inference from genome data with particle filters. 
 * 
 * Copyright (C) 2013, 2014 Sha (Joe) Zhu and Gerton Lunter
 * 
 * This file is part of pf-ARG.
 * 
 * pf-ARG is free software: you can redistribute it and/or modify
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

bool ParticleContainer::check_state_orders(){
	dout << "check particle orders, there are " << this->particles.size()<<" particles" <<endl;
	for (size_t i = 0; i < this->particles.size(); i++){
		dout << "Particle " << i << " next genealogy change occurs at position: " << std::setw(14) << this->particles[i]->next_base();
		dout << "  lambda=" << std::setw(10) << particles[i]->local_tree_length();
		dout << " weight =" << std::setw(10) << this->particles[i]->weight() <<std::endl;
        }
    dout << std::endl;
    return true;
    }

void ParticleContainer::print(){
	for (size_t i = 0 ; i < this->particles.size();  i++){
		this->particles[i]->printTree();
		cout << "paritcle " << i<<" has weight " << this->particles[i]->weight()<<endl;
        }
    }

/*! @ingroup group_debug
*/
bool ForestState::print_Coalevent(){
    dout << " ### Coalescent events:" << endl;
	for (size_t i = 0 ; i < CoaleventContainer.size() ; i++ ){
		dout << setw(10) << this->CoaleventContainer[i]->start_height()  << " to " 
             << setw(10) << this->CoaleventContainer[i]->end_height()    << ", " 
             << setw(13) << this->CoaleventContainer[i]->opportunity()   << " opportunity for " 
             << setw(2)  << this->CoaleventContainer[i]->num_event()     << " coalescent, ";
        if ( this->CoaleventContainer[i]->event_state() == NOEVENT ){
            dout<< " potetial coalsecent";
            }
        //switch ( this->CoaleventContainer[i]->event_state() ){
            //case NOEVENT:  dout<< " potetial coalsecent";     break;
            //case EVENT:    dout<< " coalsecent";              break;
            ////case MIGR_NOEVENT:  dout<< "potetial migration";    dout << " from pop " << this->CoaleventContainer[i]->pop_i() << " to pop " << this->CoaleventContainer[i]->mig_pop();   break;
            ////case MIGR_EVENT:    dout<< "                  ";    dout << " from pop " << this->CoaleventContainer[i]->pop_i() << " to pop " << this->CoaleventContainer[i]->mig_pop();   break;
            //default:  break;
            //}  dout << endl;
        dout << endl;
        }
	return true;
    }
    

/*! @ingroup group_debug
*/
bool ForestState::print_Recombevent(){
    dout << " ### Recombination events:" << endl;
	for (size_t i = 0 ; i < RecombeventContainer.size() ; i++ ){
		dout << setw(10) << this->RecombeventContainer[i]->start_height()  << " to " 
             << setw(10) << this->RecombeventContainer[i]->end_height()    << ", " 
             << setw(13) << this->RecombeventContainer[i]->opportunity()   << " opportunity for " 
             << setw(2)  << this->RecombeventContainer[i]->num_event()     << " recombination, ";
        if ( this->RecombeventContainer[i]->event_state() == NOEVENT ){
            dout<< " potetial recombination";
            }
        //switch ( this->CoaleventContainer[i]->event_state() ){
            //case NOEVENT:  dout<< " potetial coalsecent";     break;
            //case EVENT:    dout<< " coalsecent";              break;
            ////case MIGR_NOEVENT:  dout<< "potetial migration";    dout << " from pop " << this->CoaleventContainer[i]->pop_i() << " to pop " << this->CoaleventContainer[i]->mig_pop();   break;
            ////case MIGR_EVENT:    dout<< "                  ";    dout << " from pop " << this->CoaleventContainer[i]->pop_i() << " to pop " << this->CoaleventContainer[i]->mig_pop();   break;
            //default:  break;
            //}  dout << endl;
        dout << endl;
        }
	return true;
    }


///*! @ingroup group_debug
//*/
//bool ForestState::print_Coalevent_out(){
	//for (size_t i = 0 ; i < CoaleventContainer.size() ; i++ ){
		//cout << setw(10) << this->CoaleventContainer[i]->start_height()  << " to " 
             //<< setw(10) << this->CoaleventContainer[i]->end_height()    << ", " 
             //<< setw(2) << this->CoaleventContainer[i]->opportunity()   << " opportunity, " 
             //<< setw(2) << this->CoaleventContainer[i]->num_coal() 	     << " coalescent, " 
             //<< setw(2) << this->CoaleventContainer[i]->num_recomb()     << " recombination. " << endl;
        //}
	//return true;
    //}
