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


//
// C++ Implementation: integrator_omelyan
//
// Description: This integrator is based on the paper by Igor P. Omelyan,
// On the numerical integration of rigid polyatomics: The modified quaternion approach,
// Computers in Physics, Vol. 12, No. 1, p. 97 (1998).
// Careful, I use the notation Q=(q_0,\vect{q}) here as in Allen/Tidesley, Computer Simulation 
// of Liquids (Clarendon, Oxford, 1987)
//

#include "gen_f.h"
#include "phase.h"
#include "threads.h"
#include "controller.h"
#include "simulation.h"
#include "integrator_omelyan.h"
#include "cell.h"

using namespace std;


#define M_CONTROLLER ((Controller*) m_parent)
#define M_SIMULATION ((Simulation*) M_CONTROLLER->parent())
#define M_PHASE M_SIMULATION->phase()

//
#define M_MANAGER M_PHASE->manager()
const Integrator_Register<IntegratorOmelyan> integrator_omelyan("IntegratorOmelyan");

//---- Constructors/Destructor ----

IntegratorOmelyan::IntegratorOmelyan(Controller *controller)
:Integrator(controller),m_first_sweep(true)
{
  init();
}


IntegratorOmelyan::~IntegratorOmelyan()
{
}


//---- Methods ----

void IntegratorOmelyan::init()
{
  m_properties.setClassName("IntegratorOmelyan");
  m_properties.setName("IntegratorOmelyan");

  m_properties.setDescription("Integrates the orientation and angular velocity of each rigid body attached to a particle according to the algorithm described by Omelyan");
  DOUBLEPC
  	    (J1, m_J1,0,
  	     "1st principle component of the inertial tensor of the species this integrator is intended for. Default J1 = 1.");
  m_J1 = 1;
  DOUBLEPC
  	    (J2, m_J2,0,
  	     "2nd principle component of the inertial tensor of the species this integrator is intended for. Default J2 = 1.");
  m_J2 = 1;
  DOUBLEPC
  	    (J3, m_J3,0,
  	     "3rd principle component of the inertial tensor of the species this integrator is intended for. Default J3 = 1.");
  m_J3 = 1;

  STRINGPC
      (tensrotmat, m_rotmat_name,
       "Full name of the rotation matrix usable as attribute in other modules");
  STRINGPC
      (syrotmat, m_rotmat_symbol,
       "Symbol assigned to the rotation matrix, usable in algebraic expressions");
  m_rotmat_name = "rotmat";
  m_rotmat_symbol = "rotmat";

  STRINGPC
    (scq0, m_q0_name,
          "Full name of q0, usable as attribute in other modules. Default q0.");
  STRINGPC
      (syq0, m_q0_symbol,
              "Symbol assigned to q0, usable in algebraic expressions. Default q0.");
  m_q0_name = "q0";
  m_q0_symbol = "q0";

  STRINGPC
    (scq1, m_q1_name,
          "Full name of q1, usable as attribute in other modules. Default q1.");
  STRINGPC
      (syq1, m_q1_symbol,
              "Symbol assigned to q1, usable in algebraic expressions. Default q1.");
  m_q1_name = "q1";
  m_q1_symbol = "q1";

  STRINGPC
    (scq2, m_q2_name,
          "Full name of q2, usable as attribute in other modules. Default q2.");
  STRINGPC
      (syq2, m_q2_symbol,
              "Symbol assigned to q2, usable in algebraic expressions. Default q2.");
  m_q2_name = "q2";
  m_q2_symbol = "q2";

  STRINGPC
    (scq3, m_q3_name,
          "Full name of q3, usable as attribute in other modules. Default q3.");
  STRINGPC
      (syq3, m_q3_symbol,
              "Symbol assigned to q3, usable in algebraic expressions. Default q3.");
  m_q3_name = "q3";
  m_q3_symbol = "q3";

  STRINGPC
    (scomega, m_omega_name,
     "Full name of omega, usable as attribute in other modules. Defaults omega.");
  STRINGPC
    (syomega, m_omega_symbol,
     "Symbol assigned to omega, usable in algebraic expressions. Defaults omega.");
  m_omega_name = "omega";
  m_omega_symbol = "omega";
}

void IntegratorOmelyan::setup()
{
  M_CONTROLLER->registerForSetupAfterParticleCreation(this);
  Integrator::setup();

  DataFormat::attribute_t tmpAttr;

/*!
 * Setup the quaternion components
 */
  m_q0_offset =
    Particle::s_tag_format[m_colour].addAttribute
      (m_q0_name,
       DataFormat::DOUBLE,
       true,
       m_q0_symbol).offset;
  m_q1_offset =
    Particle::s_tag_format[m_colour].addAttribute
      (m_q1_name,
       DataFormat::DOUBLE,
       true,
       m_q1_symbol).offset;
  m_q2_offset =
    Particle::s_tag_format[m_colour].addAttribute
      (m_q2_name,
       DataFormat::DOUBLE,
       true,
       m_q2_symbol).offset;
  m_q3_offset =
    Particle::s_tag_format[m_colour].addAttribute
      (m_q3_name,
       DataFormat::DOUBLE,
       true,
       m_q3_symbol).offset;
/*!
 * Setup the rotation matrix
 */
  m_rotmat_offset =
      Particle::s_tag_format[m_colour].addAttribute
      (m_rotmat_name,
       DataFormat::TENSOR,
       true,
       m_rotmat_symbol).offset;
/*!
 * Setup the angular velocity components
 */
  m_omega_offset = 
    Particle::s_tag_format[m_colour].addAttribute
      (m_omega_name,
       DataFormat::POINT,
       true,
       m_omega_symbol).offset;
/*!
 * Setup the torque components
 */
  m_torque_offset = 
    Particle::s_tag_format[m_colour].addAttribute
      ("m_torque",
       DataFormat::POINT,
       true,
       "m_torque").offset;
/*!
 * Setup the forces
 */
  for(size_t i = 0; i < FORCE_HIST_SIZE; ++i)
  {
    /*!
     * The angular velocity forces setup
     */
    tmpAttr =
      Particle::s_tag_format[m_colour].addAttribute
       (STR_FORCE + STR_DELIMITER + m_omega_name + STR_DELIMITER + ObjToString(i),
       DataFormat::POINT,
       true);
    m_omega_force_offset[i] = tmpAttr.offset;
    m_omega_fAttr_index[i] = tmpAttr.index;
  }
/*!
 * Fetch the timestep from controller
 */
  m_dt = M_CONTROLLER->dt();
}
/*!
 *  Calculate rotation matrix after particle setup
 */
void IntegratorOmelyan::setupAfterParticleCreation()
{
// Let us define some helpers for writing the formulae
// 1. the 4 components of the quaternion 
//
#define Q0 (i->tag.doubleByOffset(m_q0_offset))
#define Q1 (i->tag.doubleByOffset(m_q1_offset))
#define Q2 (i->tag.doubleByOffset(m_q2_offset))
#define Q3 (i->tag.doubleByOffset(m_q3_offset))
//
//
// 7. We define the rotation matrix here
//
#define R (i->tag.tensorByOffset(m_rotmat_offset))
//
  Phase *phase = M_PHASE;
  FOR_EACH_FREE_PARTICLE_C__PARALLEL(phase, m_colour, this,
      R(0,0) = Q0*Q0+Q1*Q1-Q2*Q2-Q3*Q3;
      R(0,1) = 2*(Q1*Q2+Q0*Q3);
      R(0,2) = 2*(Q1*Q3-Q0*Q2);
      R(1,0) = 2*(Q1*Q2-Q0*Q3);
      R(1,1) = Q0*Q0-Q1*Q1+Q2*Q2-Q3*Q3;
      R(1,2) = 2*(Q2*Q3+Q0*Q1);
      R(2,0) = 2*(Q1*Q3+Q0*Q2);
      R(2,1) = 2*(Q2*Q3-Q0*Q1);
      R(2,2) = Q0*Q0-Q1*Q1-Q2*Q2+Q3*Q3;
MSG_DEBUG("IntegratorOmelyan::integrateStep1()", "Rotation matrix: " << R);
      );
  } 
/*!
 * Unprotect the forces onomega
 */
void IntegratorOmelyan::unprotect(size_t index)
{
  Phase *phase = M_PHASE;

  FOR_EACH_FREE_PARTICLE_C__PARALLEL
      (phase, m_colour, this,
       i->tag.unprotect(m_omega_fAttr_index[index]);
       if(!index) i->tag.protect(m_omega_fAttr_index[FORCE_HIST_SIZE-1]);
       else i->tag.protect(m_omega_fAttr_index[index-1]);

      );
}

/*!
 *
 */
void IntegratorOmelyan::integrateStep1(){
  Phase *phase = M_PHASE;
  size_t force_index = M_CONTROLLER->forceIndex();
  size_t other_force_index = (force_index+1)&(FORCE_HIST_SIZE-1);
  double LAMBDA;
//
// Let us define some helpers for writing the formulae
//
// 2. the 3 components of the angular velocity
//
#define Omega (i->tag.pointByOffset(m_omega_offset))
//
//
// 3. the 3 components of the torque at actual time
//
#define TauOld (i->tag.pointByOffset(m_omega_force_offset[other_force_index]))
#define TauNew (i->tag.pointByOffset(m_omega_force_offset[force_index]))
//
//
// 4. the 3 components of the time derivative of the angular velocity
//
#define Oxdot (Tbff.x/m_J1+(m_J2-m_J3)/m_J1*Omega.y*Omega.z)
#define Oydot (Tbff.y/m_J2+(m_J3-m_J1)/m_J2*Omega.z*Omega.x)
#define Ozdot (Tbff.z/m_J3+(m_J1-m_J2)/m_J3*Omega.x*Omega.y)
//
//
// 5. then we define \dot{q} according to equation (3) in Omelyan
//
#define Q0dot  (-0.5*(Q1*Omega.x+Q2*Omega.y+Q3*Omega.z))
#define Q1dot  (0.5*(Q0*Omega.x-Q3*Omega.y+Q2*Omega.z))
#define Q2dot  (0.5*(Q3*Omega.x+Q0*Omega.y-Q1*Omega.z))
#define Q3dot  (0.5*(-Q2*Omega.x+Q1*Omega.y+Q0*Omega.z))
#define QdotSQ (Q0dot*Q0dot+Q1dot*Q1dot+Q2dot*Q2dot+Q3dot*Q3dot)
// 
//
// 6. then we define \ddot{q} according to equation (4) in Omelyan
//
#define Q0ddot  (-0.5*((Q1dot*Omega.x+Q2dot*Omega.y+Q3dot*Omega.z)+(Q1*Oxdot+Q2*Oydot+Q3*Ozdot)))
#define Q1ddot  (0.5*((Q0dot*Omega.x-Q3dot*Omega.y+Q2dot*Omega.z)+(Q0*Oxdot-Q3*Oydot+Q2*Ozdot)))
#define Q2ddot  (0.5*((Q3dot*Omega.x+Q0dot*Omega.y-Q1dot*Omega.z)+(Q3*Oxdot+Q0*Oydot-Q1*Ozdot)))
#define Q3ddot  (0.5*((-Q2dot*Omega.x+Q1dot*Omega.y+Q0dot*Omega.z)+(-Q2*Oxdot+Q1*Oydot+Q0*Ozdot)))
#define QddotSQ (Q0ddot*Q0ddot+Q1ddot*Q1ddot+Q2ddot*Q2ddot+Q3ddot*Q3ddot)
// 
//
#define QdotQddot (Q0dot*Q0ddot+Q1dot*Q1ddot+Q2dot*Q2ddot+Q3dot*Q3ddot)
//
// 8. The torque saved in the tag
//
#define Tbff (i->tag.pointByOffset(m_torque_offset))
// then the timestep
//
#define DT (m_dt)
//
//
// The Lagrangian multiplier
//
//
  if(m_first_sweep == true){
    FOR_EACH_FREE_PARTICLE_C__PARALLEL(phase, m_colour, this,
      Tbff = R*TauNew;
      );
    m_first_sweep = false;
  }
  FOR_EACH_FREE_PARTICLE_C__PARALLEL(phase, m_colour, this,
      LAMBDA = (1.e0-QdotSQ*DT*DT/2.e0-sqrt(1.e0-QdotSQ*DT*DT-QdotQddot*DT*DT*DT-(QddotSQ-QdotSQ*QdotSQ)*DT*DT*DT*DT/4.e0));
      Q0 = Q0 + Q0dot*DT + Q0ddot*DT*DT/2.0 - LAMBDA*Q0;
      Q1 = Q1 + Q1dot*DT + Q1ddot*DT*DT/2.0 - LAMBDA*Q1;
      Q2 = Q2 + Q2dot*DT + Q2ddot*DT*DT/2.0 - LAMBDA*Q2;
      Q3 = Q3 + Q3dot*DT + Q3ddot*DT*DT/2.0 - LAMBDA*Q3;
//      if(i->mySlot == 13) MSG_DEBUG("IntegratorOmelyan::integrateStep1()", "NormQ " << sqrt(Q0*Q0+Q1*Q1+Q2*Q2+Q3*Q3));
      R(0,0) = Q0*Q0+Q1*Q1-Q2*Q2-Q3*Q3;
      R(0,1) = 2*(Q1*Q2+Q0*Q3);
      R(0,2) = 2*(Q1*Q3-Q0*Q2);
      R(1,0) = 2*(Q1*Q2-Q0*Q3);
      R(1,1) = Q0*Q0-Q1*Q1+Q2*Q2-Q3*Q3;
      R(1,2) = 2*(Q2*Q3+Q0*Q1);
      R(2,0) = 2*(Q1*Q3+Q0*Q2);
      R(2,1) = 2*(Q2*Q3-Q0*Q1);
      R(2,2) = Q0*Q0-Q1*Q1-Q2*Q2+Q3*Q3;
      Tbff = R*TauNew;
      );
//MSG_DEBUG("IntegratorOmelyan::integrateStep1()", "other_force_index " << other_force_index << " force_index " << force_index);
}


void IntegratorOmelyan::integrateStep2(){
  Phase *phase = M_PHASE;
  double OxIter=0.0, OxSav=0.0; 
  double OyIter=0.0, OySav=0.0; 
  double OzIter=0.0, OzSav=0.0;
  double INHOMX, INHOMY, INHOMZ;
  double PreFacX, PreFacY, PreFacZ;
  double relaerr = 1.0, residue = 1.0;
  int iter, imax=1;
  point_t TauOldPrime, TauNewPrime;
//
//
// This is to define the index to the new force which is force_index 
// and the index to the old one is given as the modulo function below.
// General form for arbitrary FORCE_HIST_SIZE should read: 
// (force_index+FORCE_HIST_SIZE-1)%(FORCE_HIST_SIZE) NOTE the % operator!!!
// This only holds for calculating the previous index.
// This is sufficient for calculating all history iteratively. I you
// only need every nth historic value -> do it on your own.
// Discussion David&Andreas 14.6.2013
//
  size_t force_index = M_CONTROLLER->forceIndex();
  size_t other_force_index = (force_index+1)&(FORCE_HIST_SIZE-1);
//  MSG_DEBUG("IntegratorOmelyan::integrateStep2()", "force_index: " << force_index << ", other_force_index: " << other_force_index);       
//
//MSG_DEBUG("IntegratorOmelyan::integrateStep2()", "other_force_index " << other_force_index << " force_index " << force_index);
//exit(0);
  FOR_EACH_FREE_PARTICLE_C__PARALLEL(phase, m_colour, this,
        PreFacX = DT*(m_J2-m_J3)/m_J1/2.0;
        PreFacY = DT*(m_J3-m_J1)/m_J2/2.0;
        PreFacZ = DT*(m_J1-m_J2)/m_J3/2.0;
	TauNewPrime=R*TauNew;
//	TauOldPrime=R*TauOld;
//	TauNewPrime=R*TauNew;
//
// We have to solve the system of equations of second order that looks like rhis
//
//       OxNew = OxOld + (TauxOld+TauxNew)*DT/m_J1/2.0 + (m_J2-m_J3)/m_J1 * (Omega.y*Omega.z+OyNew*OzNew);
//       OyNew = OyOld + (TauyOld+TauyNew)*DT/m_J2/2.0 + (m_J3-m_J1)/m_J2 * (Omega.z*Omega.x+OzNew*OxNew);
//       OzNew = OzOld + (TauzOld+TauzNew)*DT/m_J3/2.0 + (m_J1-m_J2)/m_J3 * (Omega.x*Omega.y+OxNew*OyNew);
//
    INHOMX = (Tbff.x+TauNewPrime.x)*DT/m_J1/2.0+PreFacX*Omega.y*Omega.z;
    INHOMY = (Tbff.y+TauNewPrime.y)*DT/m_J2/2.0+PreFacY*Omega.z*Omega.x;
    INHOMZ = (Tbff.z+TauNewPrime.z)*DT/m_J3/2.0+PreFacZ*Omega.x*Omega.y;
    OxIter = Omega.x;
    OyIter = Omega.y;
    OzIter = Omega.z;
    iter=0;
    do{
     iter+=1;
     OxSav = OxIter;
     OySav = OyIter;
     OzSav = OzIter;
     OxIter = Omega.x+INHOMX+PreFacX*OyIter*OzIter;
     OyIter = Omega.y+INHOMY+PreFacY*OzIter*OxIter;
     OzIter = Omega.z+INHOMZ+PreFacZ*OxIter*OyIter;
//
#define resOx (-OxIter + Omega.x+INHOMX+PreFacX*OyIter*OzIter)
#define resOy (-OyIter + Omega.y+INHOMY+PreFacX*OzIter*OxIter)
#define resOz (-OzIter + Omega.z+INHOMZ+PreFacZ*OxIter*OyIter)
#define delOx (OxIter-OxSav)
#define delOy (OyIter-OySav)
#define delOz (OzIter-OzSav)
//
     residue = sqrt(resOx*resOx+resOy*resOy+resOz*resOz);
     relaerr = sqrt(delOx*delOx+delOy*delOy+delOz*delOz)/sqrt(OxIter*OxIter+OyIter*OyIter+OzIter*OzIter);
     if(iter>100){
       MSG_DEBUG("IntegratorOmelyan::integrateStep2()", "iter = " << iter << ", residue = " << residue << ", relaerr = " << relaerr);
       exit(0);
     }
/*     MSG_DEBUG("IntegratorOmelyan::integrateStep(2)", "TauOld.x = " << TauOld.x << ", " << "TauOld.y = " << TauOld.y << ", " << "TauOld.z = " << TauOld.z << endl
	 << "TauNew.x = " << TauNew.x << ", " << "TauyNew = " << TauyNew << ", " << "TauzNew = " << TauzNew << endl);
*/
   }
   while(residue > 1.e-3 && relaerr > 1.e-3);
//  MSG_DEBUG("IntegratorOmelyan::integrateStep(2)", "exit do-while loop");      
//
//
   Omega.x = OxIter;
   Omega.y = OyIter;
   Omega.z = OzIter;
   Tbff = TauNewPrime;
//  MSG_DEBUG("IntegratorOmelyan::integrateStep(2)", "Omega = " << Omega);      
   if(iter > imax) imax=iter;
      );
}

#ifdef _OPENMP
string IntegratorOmelyan::dofIntegr() {
  return m_omega_name;
}

void IntegratorOmelyan::mergeCopies(Particle* p, int thread_no, int force_index) {
  if (m_merge == true) {
    for (int i = 0; i < SPACE_DIMS; ++i) {
      p->tag.pointByOffset(m_omega_force_offset[force_index])[i] += (*p->tag.vectorDoubleByOffset(m_vec_offset[thread_no]))[m_vec_pos + i];
      (*p->tag.vectorDoubleByOffset(m_vec_offset[thread_no]))[m_vec_pos + i] = 0;
    }
  }
}

#endif

