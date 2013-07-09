/*
 * This file is part of the SYMPLER package.
 * https://github.com/kauzlari/sympler
 *
 * Copyright 2002-2013, 
 * David Kauzlaric <david.kauzlaric@frias.uni-freiburg.de>,
 * and others authors stated in the AUTHORS file in the top-level 
 * source directory.
 *
 * SYMPLER is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SYMPLER is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SYMPLER.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Please cite the research papers on SYMPLER in your own publications. 
 * Check out the PUBLICATIONS file in the top-level source directory.
 *
 * You are very welcome to contribute extensions to the code. Please do 
 * so by making a pull request on https://github.com/kauzlari/sympler
 * 
 */



#ifndef PAIR_VECTOR_H_
#define PAIR_VECTOR_H_


#include "val_calculator.h"
#include "function_pair.h"
#include "general.h"

using namespace std;

/* Module for calculation of pair forces; the result is saved as a pair attribute*/

class PairVector: public ValCalculatorPair {
protected:
	/*!
	 * Cut-off radius for the pair summation
	 */
	double m_cutoff;
	/*!
	 * A string for m_function;
	 */
	string m_expression_string;

	/*!
	 * The function pair computing the user defined pair factor
	 */

	FunctionPair m_function;
    /*!
     * An additional factor for the 1st particle
     */
    FunctionPair m_1stparticleFactor;

    /*!
     * An additional factor for the 2nd particle
     */
    FunctionPair m_2ndparticleFactor;
    /*!
      * The mathematical expression for \a m_1stparticleFactor
     */
     string m_1stPExpression;

    /*!
      * The mathematical expression for \a m_2ndparticleFactor
     */
     string m_2ndPExpression;

	/*!
	 * This string holds the symbols, which are not waited for to be computed beforehand
	 */
	string m_oldSymbols;


	virtual ValCalculatorPair* copyMySelf() {
		return new PairVector(*this);
	}
	/*!
	 * Initialise the property list
	 */
	virtual void init();

public:

	PairVector(/*WeightingFunction *wf,*/string symbol);
	/*!
	 * Constructor for Node hierarchy//
	 */
	PairVector(/*Node*/Simulation* parent);

	/*!
	 * Destructor//
	 */
	virtual ~PairVector();

	/*!
	 * Setup this Calculator
	 */
	virtual void setup();

	/*!
	 * Returns the symbol name as defined in the input file.
	 */
	virtual string myName() {
		return m_symbolName;
	}
	//
	//set the slots for the pairs
	virtual void setSlot(ColourPair* cp, size_t& slot, bool oneProp);

	//Slots for each particle,we need a slot per pair,so this should not be called
	virtual void setSlots(ColourPair* cp, pair<size_t, size_t> &theSlots,
			bool oneProp) {
		throw gError
		("PairVector::setSlots", "(ColourPair*, "
				"pair<size_t, size_t>&, bool ) should not be called! Contact the programmers.");
	}

	void mySlot(size_t& slot) const {
		slot = m_slot;
	}

	virtual void mySlots(pair<size_t, size_t> &theSlots) {
		throw gError("PairVector::mySlots(pair<size_t, size_t>*) should "
				"not be called!");
	}

	/*Compute the user defined expression for pair pD*/
#ifdef _OPENMP
	virtual void compute(Pairdist* dis, int threadNum) {
	  // does the same as in serial mode at the moment; 
	  // it writes into pairs, so should work
	  compute(dis);
	}
#endif
	virtual void compute(Pairdist* dis) {

	  if (dis->abs() < m_cutoff) {
	    
	    point_t temp;
	    temp.x=0;
	    temp.y=0;
	    temp.z=0;
	    
	    // compute the expression for the pair function
	    m_function(&temp, dis);
	    dis->tag.pointByOffset(m_slot)=temp;
	  }
	}




	/*!
	 * Diffenrently to the function in \a Symbol, this class really has
	 * to determine its stage during run-time
	 */
	virtual bool findStage();

	/*!
	 * Diffenrently to the function in \a Symbol, this class really has
	 * to determine its stage during run-time
	 */
	virtual bool findStage_0();
};



#endif /* PAIR_VECTOR_H_ */
