/***************************************************/
/*! \class Bus
    \brief STK basic mixer class.

    This class keeps a vector list of inputs and sums them to compute the output frames

    by Perry R. Cook and Gary P. Scavone, 1995 - 2007.
*/
/***************************************************/

#ifndef STK_SAMPLERUGEN_H
#define STK_SAMPLERUGEN_H

#include "Ugen.h"
#include "Sampler.h"
#include <list>

class SineWaveUgen : public Ugen
{
public:
  //! Default constructor.
  SineWaveUgen( void );

  //! Class destructor.
  virtual ~SineWaveUgen( void );
  
  char * name( void ){ return "SineWave"; }
  
  int add(Ugen*);
  
  int remove(Ugen*);
  
  void clearInputs();

	std::list<Ugen*> frequency_mods;
	std::list<Ugen*> phase_mods;
	std::list<Ugen*> writers;
		  
  SineWave *source;
  
  
protected:

  StkFloat computeSample( void );
  void computeFrame( void );
};

#endif