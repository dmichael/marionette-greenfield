
#include <list>
#include "SimpleSamplerUgen.h"


SimpleSamplerUgen::SimpleSamplerUgen()
{
  source = new FileWvIn;
  source->openFile("../soundfiles/test.wav");
  source->normalize();
  setChannels(source->getChannels());
}

SimpleSamplerUgen::~SimpleSamplerUgen()
{
  delete source;
}

int SimpleSamplerUgen::add(Ugen *ugen)
{
	
  writers.push_back(ugen);
	
	// expand number of channels in bus, if necessary:
	// return added process
	return 1;
}

int SimpleSamplerUgen::remove(Ugen *ugen)
{	
	// return removed process
	return 1;
}

void SimpleSamplerUgen::clearInputs()
{
	
}

StkFloat SimpleSamplerUgen::computeSample()
{  
 lastOutput_ = source->tick();
 return lastOutput_;
}


// This tick() is also called by the frames version of the tick.
// .. it is in this function that 
// 1) all the instruments are looped through
// 2) in this loop, the instrument is tick()ed
// 3) then, the intrument's gains are looped through, multiplying the ticked value by the gain
// .. assigning the resulting value to the something like _lastOutputs[num_of_channels]
// 4) this value is then assigned to the correct frame.. whatever that maybe :(

void SimpleSamplerUgen::computeFrame( void )
{
  source->tickFrame( lastOutputs_ );
  
  //ugen->tickFrame( lastOutputs_ );
  // make sure that the samples are cleared out - this is super shitty
  // for ( int x=0; x<num_channels; x++ ) lastOutputs_[x] = 0;
  //   lastOutput_ = 0;
  //   //lastOutput_ = lastOutputLeft_ = lastOutputRight_ = 0.0;
  //   // loop through all the voices and tick() them, producing a single sample
  //   std::list<Ugen*>::iterator i;
  //   for(i = writers.begin(); i != writers.end(); i++){
  //     // calculate the output of the instrument
  //     lastOutput_  = (*i)->tick();
  //     for ( int j=0; j<num_channels; j++ ) 
  //     {
  //       lastOutputs_[j] += (lastOutput_ / writers.size());
  //     }
  //   }

}

