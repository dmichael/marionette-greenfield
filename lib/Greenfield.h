/*!
    \class Greenfield
    \brief STK sample-based ugen implementing the Greenfield call-timing algorithm
    
    This is, in essence, an obervable observing oscillator which triggers an event at a chosen frequency.
    When observables that the Greenfield is listening to post an event (send a signal), 
    the internal oscillator's phase is reset to the basal position.
    
    This reset follows this equation:
    
    T' = s[(d + l/v) − (r − t)] + (T + e) + (y − x)
    
    where:
    T  = free running call period (seconds)
    T' = modified call period after receipt of stimulus (seconds) 
    d  = time elapsed since onset of last call (seconds)
    l  = distance of the stimulus (meters)
    v  = the speed of sound (344 m/s)
    r  = length of the decending slope
    t  = effector delay
    s  = phase response curve
    e  = stochastic element
    y  = length of the stimulus call
    x  = length of the call

    Usage ::
    You can specify the maximum polyphony (maximum
    number of simultaneous voices) in Greenfield.h.
    
    by David M. Michael 7.4.2007 for Unnature
    Adapted from VoicDrum by Perry R. Cook and Gary P. Scavone, 1995 - 2005.
    
*/


#ifndef STK_GREENFIELDGEN_H
#define STK_GREENFIELDGEN_H

#include <cmath>
#include <iostream>
#include <sstream>
#include <map>
#include "Rand.h"
#include "FileWvIn.h"
#include "Unit.h"
#include "SoundFile.h"
using namespace std;
  
class Greenfield : public Unit
{
public:

  //! Class constructor.
  Greenfield();
  
  //! Overloaded class constructor.
  Greenfield(float, float, float, float);
  
  //! Class destructor.
  ~Greenfield();

  char * name( void ){ return "Greenfield"; }

  void initialize(void);
  
  static float calculateAttenuation(float spl, float distance);

  static float calculateDistance(float x1, float y1, float x2, float y2);
  
  map<string,float> host_data;
  
  /* 
    GETTERS AND SETTERS 
    ---------------------------------------------------- 
  */
  //! Get ID
  int id( );
  string id_str( );
  
  //! Set ID
  void setId( int);
  
  void set_id( );
  
  //! get period
  double period(){ return PERIOD_; };
  
  //! set period and related params
  void setPeriod(float);
  
  //! Get decending length
  float decending_length() { return r_; };
  
  //! Set decending length
  void setDecendingLength( float );
  
  //! Get effector delay
  float effector_delay() { return t_; };
  
  //! Set effector delay
  void setEffectorDelay( float delay ) {
    t_ = delay;
  };
  
  //! Get PRC slope
  float prc_slope() { return s_; };
  
  //! Set PRC slope
  void setPrcSlope( float prc_slope ) {
    s_ = prc_slope;
  };
      
  float spl(int);

  void setSpl(float);


  float rebound( void );

  void setRebound( float );
  
  float acceleration( void ){ return acceleration_; };

 //! Set acceleration
  void setAcceleration( float acceleration ) { acceleration_ = acceleration; };

  void setActuator(SoundFile*);
    
  int maxNeighbors();
  
  void setMaxNeighbors(int);
  
  float costOfSignaling();
  
  void setCostOfSignaling( float );
  
  static bool sortNeighbors(float, float);
  
  bool attendToNeighbor( float );

  void forgetNeighbors( void );

  void expendEnergy( float );
  
  void regainEnergy( void );
  
  float callPotential(void);

  void adjustCallPotential(float value);
  
  void setCallPotential(float value);
  
  float threshold(void);
  
  void setThreshold(float);
  
  void adjustThreshold(float value);
  
  void update( void * );
  
  bool triggerable();
  
  void triggerSignal();
  
  void incrementFrame( void );
  
  //! The computation method - very important
  StkFloat computeSample( void );
  
  //! Class constructor.
  StkFloat getTime( void ) const;

  //! Class constructor.
  StkFloat getElapsedTime( void ) const;
  
  //! Class constructor.
  unsigned long getFrameCount( void ) const;





//private:

  StkFloat elapsed_time;          //! Elapsed time since object creation (seconds)
  double d;  //! (d) Time elapsed since onset of last call (seconds) 

  double pulse_duration;
  float e;                      //! Stochastic element
  float correction;
  float correction2;
  float temp;
  unsigned long elapsed_time_in_frames;
  unsigned long last_call_received_time;
    
  // These variables could be managed outside the object (environment? dispatcher?)
  float stimulus_distance;   //! (l) Distance of the stimulus (meters)

    
private:
  int call_count;
  // TODO: this could be more flexible
  SoundFile *actuator_;

  float acceleration_; // period acceleration  

  // important internal fields
  //! Fields for use in the Greenfield call-timing algorithm
  double PERIOD_;               //! (T) Free running call period (seconds)
  double period_;               //! (T') Modified call period after receipt of stimulus (seconds)       
  double period_overflow_;      //! Storage variable for edge case (seconds)
  unsigned long period_in_frames_;
  unsigned long frameCounter_;
  
  float  r_;     //! (r) Length of the decending slope        
  float  t_;       //! (t) Effector delay  
  float  s_;            //! (s) Phase Response Curve  (PRC)
  double x_;        //! (x) Length of the call  
  float  rebound_; // this is the rebound length of the oscillator 

  // sound pressure level (SPL) of the call at the source (dB)
  float  spl_;            
  float  gain_;
  
  // Energy for calling... this should aid in provoking unison bout singing
  float energy_;
  float call_potential_;
  float threshold_;
  float cost_of_signaling_;

  // Keep a list of neighbors.
  // neighbors are evaluated and sorted by sound pressure levels (SPLs)
  // as percieved by the agent
  vector<float> neighbors_;
  unsigned int max_neighbors_; // 2 is usually good
  
  bool has_energy_to_signal;
};

#endif
