#include "Greenfield.h"

/* Default Constructor */
Greenfield::Greenfield()
{  
  initialize();
  setPeriod( 1.5 );
  setDecendingLength( 0.1 );
  setEffectorDelay( 0.1 );
  setPrcSlope( 0.1 );
}

/*
  Overloaded Constructor - complete initialization
  Variable that should be set here:
    period
    decending_length
    effector_delay
    prc_slope
    file_name
*/
Greenfield::Greenfield(float period = 1.0, float decending_length = 1.0, float effector_delay = 1.0, float prc_slope = 1.0)
{
  // some really rote initialization
  initialize();
  setPeriod( period );
  setDecendingLength( decending_length );
  setEffectorDelay( effector_delay );
  setPrcSlope( prc_slope );
}

Greenfield::~Greenfield()
{
}

void Greenfield::initialize()
{
  elapsed_time = 0; 
  d = 0; 
  e = 0; 
  correction = 0; 
  correction2 = 0; 
  temp = 0; 
  elapsed_time_in_frames = 0; 
  last_call_received_time = 0;
  frameCounter_ = 0; 
  rebound_ = 0.0;
  // this is a conservative estimate for selective attention
  // some may not want any neighbors at all - so use setter
  max_neighbors_ = 1; 
  acceleration_ = 0.0; 
  // energy related intialization
  energy_     = 1.0;
  call_potential_ = 0.99;
  threshold_  = 0.0;
  cost_of_signaling_ = 0.10;

  // This oscillator has a host
  host_data["x"]   = 0.0;
  host_data["y"]   = 0.0;
  host_data["spl"] = 0.0;
  host_data["call_duration"] = 0.0;
  
  call_count = 0;
}

// Getters and Setters
//-------------------------------------
void Greenfield::setPeriod(float period) 
{
  PERIOD_ =  period_  = period;
  period_in_frames_   = (unsigned long) (period_ * Stk::sampleRate());
  period_overflow_    = 0;
}

void Greenfield::setDecendingLength( float length ) 
{
  r_ = length;
}

float Greenfield::spl( int index )
{ 
  return spl_; 
}

void Greenfield::setSpl( float spl )
{
  spl_  = spl;
  // TODO: find a better equation for extracting gain from SPL
  setGain( spl / 100 );
}

float Greenfield::rebound( void )
{ 
  return rebound_; 
}

void Greenfield::setRebound( float rebound )
{
  rebound_ = rebound;
}

void Greenfield::setActuator(SoundFile* ugen){
  actuator_ = ugen;
}
 
int Greenfield::maxNeighbors( )
{
  return max_neighbors_;
}

void Greenfield::setMaxNeighbors(int max)
{
  max_neighbors_ = max;
}

StkFloat Greenfield::getElapsedTime(void) const
{
  return (StkFloat) frameCounter_ / Stk::sampleRate();
}

unsigned long Greenfield::getFrameCount( void ) const
{
  return frameCounter_;
}

StkFloat Greenfield::getTime( void ) const
{
  return (StkFloat) frameCounter_ / Stk::sampleRate();
}

float Greenfield::costOfSignaling(){
  return cost_of_signaling_;
};

void Greenfield::setCostOfSignaling( float cost ){
  cost_of_signaling_ = cost;  
};



/* 
  here is where we calculate the SPL at the receiver. 
  it is assumed that the SPL that comes in from the message is measured at 1 meter..
  so the equation used here:
  D (difference) = 20 log (d1/d2), where d1 =1, d2 = distance of this agent from the source  
  Thanks to Dimitri Ranos for helping me figure this one out.
*/
float Greenfield::calculateAttenuation(float spl, float distance)
{
  return spl + ( 20 * log10(1/distance) );
}

float Greenfield::calculateDistance(float x1, float y1, float x2, float y2)
{
  return sqrt( pow((x2 - x1), 2) + pow((y2 - y1), 2) );
}

bool Greenfield::sortNeighbors(float i, float j) { return (i > j); }

/*
  This function adds a neighbor's SPL to a list of all neighbors and sorts it in decending order.
  TODO: have neighbors expire after some time so that selective attention can shift to another signaller
*/
bool Greenfield::attendToNeighbor(float spl)
{
  bool attending = false;
  // are we already attending to this guy?
  for(unsigned int i=0; i< neighbors_.size(); i++ ){
    if( neighbors_[i] == spl){
      attending = true;
    } 
  }
  // If the neighbor is not found in the list, add it if we have room.
  if( !attending ) {
    // is there room to just push it into the vector?
    if( neighbors_.size() < max_neighbors_ ) {
      neighbors_.push_back( spl );
      attending = true;
    }
    else{
      // if we have reached our limits, replace the quietest, 
      // but only if it is louder than the queitest (!) ...
      if(spl > neighbors_[max_neighbors_ - 1]) {
        neighbors_[max_neighbors_ - 1] = spl;
        attending = true;  // affirm that you are attending to this one
      }
      else {
        attending = false; // deny that you are attending to this one
      }
    }    
    // sort them in descending order
    sort(neighbors_.begin(), neighbors_.end(), Greenfield::sortNeighbors);
  }
  return attending;
}

void Greenfield::forgetNeighbors( )
{
  neighbors_.clear();
}


void Greenfield::expendEnergy(float value){
  Rand r;
  energy_ -= r.randfloat(0.01, 0.03) * value;
  
  if( energy_ <= 0) {
    forgetNeighbors();
    energy_ = 0;
  }
}

void Greenfield::regainEnergy(){
  Rand r;
  energy_ += r.randfloat(0.15, 0.25) * cost_of_signaling_;
}


/*
  "inhibition" governs the willingness of the agent to signal
  in general, this is intended to be raised when calling and lowered when a call is heard.
  Lowering inhibition has the effect of eventually stimulating an agent to call.
  Conversely, raising inhibition prevents a caller from calling interminably and solo. This would be annoying.
  
  High ambient light raises the threshold to signal - perhaps even past the ability of agent inhibition (or lack thereof) to compensate for.
  A very low ambient light or darkness, humidity, or possibly the signalling of others also lowers the threshold
*/



/* inhibition */
float Greenfield::callPotential(void) {
  // lets return the inhibition on an exponential scale
  return call_potential_;//(call_potential_ * call_potential_);
}

/* adjustCallPotential */
void Greenfield::adjustCallPotential(float value) {
  call_potential_ += value;
  if (call_potential_ >= 1.0){
    call_potential_ = 1.0;
  }
  else if (call_potential_ <= -1.0){
    call_potential_ = -1.0;
  }
}

void Greenfield::setCallPotential(float value) 
{
  call_potential_ = value;
}

/* threshold */
float Greenfield::threshold(void) {
  // threshold is in the range [0.0 - 1.0] and can be genetically determined
  return threshold_;//(threshold_ * threshold_);
}

void Greenfield::setThreshold(float value) {
  // inhibition is in the range [0.0 - 1.0]
  threshold_ = value;
  if (threshold_ >= 1.0){
    threshold_ = 1.0;
  }
  else if (threshold_ <= 0.0){
    threshold_ = 0.0;
  }
}

/* adjustThreshold */
void Greenfield::adjustThreshold(float value) {
  // inhibition is in the range [0.0 - 1.0]
  threshold_ += value;
  if (threshold_ >= 1.0){
    threshold_ = 1.0;
  }
  else if (threshold_ <= 0.0){
    threshold_ = 0.0;
  }
}

/* triggerable? */
bool Greenfield::triggerable(){
  if( callPotential() > threshold() ){
    return true;
  } 
  else{
    return false;
  }
}

/*!
  When a message is received from a neighbor, the call period is lengthened or shortened by the timing algorithm.
  The call timing information is specified in the genome in seconds so that its easier to read.
  !!!
  NB: These calculations are done in *SECONDS*, 
      thus there needs to be certain conversions to frames for these variables:
      elapsed_time, 
*/
void Greenfield::update( void *o ) {
  Rand r;
  // Lets cast the messages since we know what they should be
  Greenfield *observable = (Greenfield *) o;
  // get the host data to calculate SPL of the caller
  float senders_x = observable->host_data["x"];
  float senders_y = observable->host_data["y"];
  float src_spl   = observable->host_data["spl"];
  float y = observable->host_data["call_duration"];
  

  // printf("senders_x %f  host_data[x]:%f \n", senders_x, host_data["x"]);
  //   printf("senders_y %f  host_data[y]:%f \n", senders_y, host_data["y"]);
  //   printf("senders_y %f  host_data[y]:%f \n", senders_y, host_data["y"]);
  //       
  // Calculate the distance and SPL with attenuation of the stimulus.
  float l = Greenfield::calculateDistance( senders_x, senders_y, host_data["x"], host_data["y"] );    
  float spl               = Greenfield::calculateAttenuation( src_spl, l );
      
  if( attendToNeighbor(spl) )
  {
    last_call_received_time = clock;
    //printf("\n\n");    
    //cout << "Heard someone " << "at spl: " << spl << " mythresh: " << threshold() << ", mycallpotential: " << callPotential() << endl;
    // upon hearing someone else, your own excitation goes up
    adjustCallPotential(r.randfloat(0.1, 0.3));
    // Make for damn sure these are cleared out
    d = temp = 0;
    float x = host_data["call_duration"];
    // Calculate the trigger time:
    // Since a trigger is sent when the elapsed time >= period, 
    // then presumably the time that the trigger is sent to the trigger at t = "t_" before the period_ length
    float trigger_time = period_ - t_; 
    elapsed_time       = elapsed_time_in_frames / Stk::sampleRate();
    
    // Has the time elapsed just passed the next triggering event (its a very small window)?  
    // If so, set the d negative! - very important, get it?
    // - also if it HAS passed, this period is added onto the next period - NOT the current one
    if ( elapsed_time > trigger_time && elapsed_time < period_) { 
      d = trigger_time - elapsed_time;
    }
    else {
      d = elapsed_time;
    }   
  
    // Add a little noise to the period adjustment
    e = r.randfloat(-0.09, 0.1);
    // Calculate the new call period!
    //printf("s:%f d:%f y:%f x:%f\n", s_, d, y, host_data["call_duration"]);
        // ( s * ( (d+(l/v)) - (r-t) ) ) + (T + E) + (s.y-x) => temp;
    float adjustment = ( s_ * ((d + (l/344)) - (r_ - t_)) );
    temp = adjustment + (PERIOD_ + e) + (y - x) - rebound_;
    //printf("adjustment:%f \n", adjustment);
    //printf("temp:%f \n", temp);
    // If the "d" is positive, set the current "period_" to "temp", thus lengthening it,
    // otherwise, put the value in the overflow to be used for the next period
    if( d > 0.0 ){
      //cout << "_period:"<< period_ <<"\n";
      period_ = temp;
      period_in_frames_ = (unsigned long) (temp * Stk::sampleRate());
      //cout << "!period:"<< period_ <<"\n";    
    }
    else {
      //cout << "_period:"<< period_ <<"\n";
      period_overflow_ = temp;
      //cout << "!period OVERFLOW:"<< period_ <<"\n";
    }
  }
  else
  {
    //cout << "xxxxxxxxxx NOT attending to my neighbor at spl: " << spl << "\n\n";
  }
}

/*
  A signal has been triggered, now process the consequences
*/
void Greenfield::triggerSignal()
{
  Rand r;
  // Emit the signal to the voicebox and any other entities that are listening
  actuator_->source->reset();
  // Tell your fans that something has happened
  notifyObservers( this );
  // Expend a little energy for signaling  
  expendEnergy(cost_of_signaling_);
  // Raise inhibition a little, its embarrasing (and dangerous) to let yourself be known alone
  adjustCallPotential(r.randfloat(-0.009, -0.004));  
  
  call_count++;
  // this may be drastic, but
  if(call_count % 4 == 0){
    //printf("Forgetting neighbors\n");
    forgetNeighbors();
    call_count = 0;
  }

  int num_potential_calls =  clock / (unsigned long) (PERIOD_ * Stk::sampleRate());
  int mod2 =   clock - last_call_received_time;
  //printf("!!! %i %i %i %i %i\n", last_call_received_time, clock, (unsigned long) (PERIOD_ * Stk::sampleRate()), num_potential_calls, mod2);


  //cout << "Triggered a signal: mythresh: " << acceleration() << endl;
  // if is has been a long time since you heard from anyone
  if(acceleration() > 0 && mod2 > (PERIOD_ * Stk::sampleRate()) )
  {
    period_ += acceleration();
    period_in_frames_ = (unsigned long) (period_ * Stk::sampleRate());
    adjustCallPotential(r.randfloat(-0.09, -0.04));  
    //cout << "Triggered a signal: mythresh: " << threshold() << ", mycallpotential: " << callPotential() << " myperiod:" << period_ << endl;
  }
  else{
    // If there is something "stored" for the next period, 
    // it becomes the modified period (?..double check this) --checked
    if( period_overflow_ > 0 ) {
      period_in_frames_ = (unsigned long) (period_overflow_ * Stk::sampleRate());
      period_ = period_overflow_;
    }
    else {
      period_in_frames_ = (unsigned long) (PERIOD_ * Stk::sampleRate());
      period_ = PERIOD_;  // reset the period to its free-running value
    }
    period_overflow_        = 0; // reset the next round modifications
  }
}


/*!
  These calculations *should* be done in frames, as that is the currency of realtime audio.
  This function also determines whether or not to chirp, based on the period
  
  This class is only a generator function and does not return any meaningful sample calculations, 
  it does however trigger an event which can and should be tied to some sort of sound generator
  - this is also a good case to look at some sort of control rate processing, but I am not sure
*/
void Greenfield::incrementFrame()
{
  // If the elapsed time (Telapsed) is greater than or equal to the call period (Tprime)
  // send a trigger to the chirp  
  if( elapsed_time_in_frames >= period_in_frames_ ) {
    // triggerable?  
    if( triggerable() ) {
      // then trigger the signal
      triggerSignal(); 
      
    }
    // not triggerable - maybe regain a little energy here (?)
    else {
      Rand r;
      adjustCallPotential(r.randfloat(0.01, 0.02));
    }
    // reset the elapsed time
    elapsed_time_in_frames = 0;
    elapsed_time           = 0;
  }
  elapsed_time_in_frames++;
}

StkFloat Greenfield::computeSample()
{

  lastOutput_ = 0.0;
  incrementFrame();
  //if ( nSounding_ == 0 ) return lastOutput_;

  lastOutput_ = actuator_->tick();
  clock++;
  return lastOutput_;
}