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



#ifndef __THERMOSTAT_PETERS_H
#define __THERMOSTAT_PETERS_H 

#include "thermostat_with_rng.h"
#include "integrator_energy.h"
#include "weighting_function.h"
#include "function_pair.h"


using namespace std;

/* --- ThermostatPeters --- */

/*!
 * Implementation of the Peters thermostat.
 * See: E. A. J. F. Peters, Europhys. Lett. 66, 311-317 (2004)
 */
class ThermostatPeters : public ThermostatWithRng
{
protected:
  /*!
   * The species to thermalize
   */
  pair<string, string> m_species;

  /*!
   * The pair of colors to thermalize
   */
  ColourPair *m_cp;

  /*!
   * The cut-off radius, taken from the weighting function
   */
  double m_cutoff;

  /*!
   * The dissipation constant
   */
//   double m_dissipation;
  FunctionPair m_dissipation;

  /*!
   * Simulation time step
   */
  double m_dt;

  /*!
   * Reciprocal of the cut-off radius
   */
  double m_rc;

  /*!
   * Name of the weighting function to use
   */
  string m_weighting_function;

  /*!
   * Pointer to the actual implementation of the weighting function
   */
  WeightingFunction *m_wf;

  /*!
   * Initialize the property list
   */
  void init();

public:
  /*!
   * Constructor
   * @param sim Pointer to the main simulation object
   */
  ThermostatPeters(Simulation* sim);

  /*!
   * Destructor
   */
  virtual ~ThermostatPeters() {}

  /*!
   * Get the \a ColourPair and initialize variables
   */
  virtual void setup();
};

#endif
