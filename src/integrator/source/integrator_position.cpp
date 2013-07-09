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



#include "gen_f.h"
#include "phase.h"
#include "threads.h"
#include "controller.h"
#include "simulation.h"
#include "integrator_position.h"
#include "cell.h"

using namespace std;


#define M_CONTROLLER  ((Controller*) m_parent)
#define M_SIMULATION  ((Simulation*) M_CONTROLLER->parent())
#define M_PHASE  M_SIMULATION->phase()
#define TRACK_PARTICLE_183

// const Integrator_Register<IntegratorPosition> integrator_position("IntegratorPosition");

//---- Constructors/Destructor ----

IntegratorPosition::IntegratorPosition(Controller *controller): Integrator(controller)
{
    init();
}


IntegratorPosition::~IntegratorPosition()
{
}



//---- Methods ----

void IntegratorPosition::init()
{
  m_properties.setClassName("IntegratorPosition");

  m_properties.setDescription(
    "Integrates the position and momentum coordinates of each particle."
  );

  DOUBLEPC
    (lambda,
     m_lambda,
     0,
     "Lambda parameter for modified velocity verlet algorithm.");

  m_lambda = 0.5;
}

void IntegratorPosition::isAboutToStart()
{
}


void IntegratorPosition::integrateStep1()
{
//   FOR_EACH_FREE_PARTICLE_C
//       (M_SIMULATION->phase(), 0,
//        if(i->mySlot == 183)
//        {
//          MSG_DEBUG("IntegratorPosition::integrateStep1", i->mySlot << "BEFORE (N=" << M_SIMULATION->phase()->returnNofPart() << "):"
//              << endl << "r=" << i->r << endl << "v=" << i->v << endl << "dt="  << i->dt << endl << "f0="  << i->force[0] << endl << "f1="  << i->force[1]);
//        }
//       );

 M_PHASE->invalidatePositions(this);

//   FOR_EACH_FREE_PARTICLE_C
//       (M_SIMULATION->phase(), 0,
//        if(i->mySlot == 183)
//        {
//          MSG_DEBUG("IntegratorPosition::integrateStep1", i->mySlot << "AFTER (N=" << M_SIMULATION->phase()->returnNofPart() << "):"
//              << endl << "r=" << i->r << endl << "v=" << i->v << endl << "dt="  << i->dt << endl << "f0="  << i->force[0] << endl << "f1="  << i->force[1]);
//        }
//       );
}


void IntegratorPosition::integrateStep2()
{
   Phase *phase = M_PHASE;

   phase->invalidate();
}


void IntegratorPosition::integratePosition(Particle* p, Cell* cell)
{
}


void IntegratorPosition::integrateVelocity(Particle* p)
{
}


void IntegratorPosition::solveHitTimeEquation(WallTriangle* wallTriangle, const Particle* p, const point_t &force, vector<double>* results)
{
}


void IntegratorPosition::hitPos(/*WallTriangle* wallTriangle, */double dt, const Particle* p, point_t &hit_pos, const point_t &force)
{
}


#ifdef _OPENMP
string IntegratorPosition::dofIntegr() {
  return "vel_pos";
}


void IntegratorPosition::mergeCopies(Particle* p, int thread_no, int force_index) {
}

#endif

