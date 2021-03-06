//
// 
// Code to assign descendant information to event.
// 
//

#ifndef DESCENDANTS_H
#define DESCENDANTS_H

#include <iostream>

#include "scrm/src/model.h"



typedef uint64_t Descendants_t;

static const Descendants_t NO_DESCENDANTS = 0;



inline Descendants_t get_descendants( const Node* node ) {

    Descendants_t result = 0;
    while (node->label() == 0) {

	/* 02/18/19
	 * When inferring recombination and reporting ARGs
	 * getLocalChild1() occasionally returns a null pointer
	 * as documented, causing a seg fault. Accounting for this possibility.
	 *
	 * Joe indicated that this is perhaps becuase the node is nonlocal
	 * and after checking, this appears to always be the case. 
	 *
	 * To remedy this, I've ensured that the node is local prior
	 * to retrieving children. */ 
       	if(node->local()) {         
              	result |= get_descendants( node->getLocalChild1() );
	}
	node = node->getLocalChild2();  // could be NULL
         
        if (!node) return result;
    }
    assert (node->label() <= 64);
    return result | (((Descendants_t)1) << (node->label() - 1));
}


// updates 'sample' to next 1-based descendant index, or returns 'false'
// initial call should be with sample == 0
inline bool get_next_descendant( Descendants_t& descendants, int& sample ) {

    if (descendants == 0) return false;
    while ( (descendants & 1) == 0 ) {
        ++sample;
        descendants >>= 1;
    }
    ++sample;
    descendants >>= 1;
    return true;
}


inline void print_descendants( std::ostream& str, Descendants_t descendants ) {

    int sample = 0;
    int this_sample = 0;
    if (get_next_descendant( descendants, sample )) {
        do {
            while (++this_sample < sample) {
                str << "0";
            }
            str << "1";
        } while (get_next_descendant( descendants, sample ));
    } else {
        str << "0";
    }
}

#endif
