/*****************************************************************************

File:     rubymain.cpp
Date:     06Apr06

Copyright (C) 2007 by David M Michael for Unnature, LLC . All Rights Reserved.

*****************************************************************************/

#include <ruby.h>
#include "shared.h"
#include "Greenfield.h"

VALUE GreenfieldClass;

static VALUE Greenfield_alloc(VALUE self)
{
  Greenfield *ugen = new Greenfield;
  VALUE obj = Data_Wrap_Struct(self, 0, rb_ugen_dealloc, ugen);
	return obj;
}

static VALUE Greenfield_name(VALUE self) 
{
  Greenfield *ugen;
  Data_Get_Struct(self, Greenfield, ugen);
	return rb_str_new2(ugen->name());
}

static VALUE Greenfield_get_host_data(VALUE self) 
{
  Greenfield *greenfield;
  Data_Get_Struct(self, Greenfield, greenfield);
  // this should return a Hash
	return LONG2FIX(greenfield->host_data["x"]);
}

static VALUE Greenfield_set_host_data(VALUE self, VALUE options) 
{
  VALUE x = rb_hash_aref(options, ID2SYM(rb_intern("x")));
  VALUE y = rb_hash_aref(options, ID2SYM(rb_intern("y")));
  VALUE spl = rb_hash_aref(options, ID2SYM(rb_intern("spl")));
  VALUE call_duration = rb_hash_aref(options, ID2SYM(rb_intern("call_duration")));
  
  Greenfield *ugen;
  Data_Get_Struct(self, Greenfield, ugen);
  
  if(x != Qnil) ugen->host_data["x"] = FIX2LONG(x);
  if(y != Qnil) ugen->host_data["y"] = FIX2LONG(y);
  if(spl != Qnil) ugen->host_data["spl"] = FIX2LONG(spl);
  if(call_duration != Qnil) ugen->host_data["call_duration"] = FIX2LONG(call_duration);
    
	return self;
}

static VALUE Greenfield_get_actuator(VALUE self) 
{
  // this should return a Hash
	return self;
}

static VALUE Greenfield_set_actuator(VALUE self, VALUE actuator) 
{
  // make sure the type being passing in is the one that we want
  if(TYPE(actuator) != T_DATA || RDATA(actuator)->dfree != (RUBY_DATA_FUNC)rb_ugen_dealloc){
    rb_raise(rb_eTypeError, "Wrong argument type");
  }
  // retrieve the Unit...
  Greenfield *ugen;
  Data_Get_Struct(self, Greenfield, ugen);
  
  // retrieve the Unit...
  Unit *actuator_ptr;
  Data_Get_Struct(actuator, Unit, actuator_ptr);
  
  // ... and add it to "out"
  ugen->setActuator((SoundFile*)actuator_ptr);
    
	return self;
}

static VALUE Greenfield_set_acceleration(VALUE self, VALUE acceleration) 
{
  if(TYPE(acceleration) == T_FLOAT || TYPE(acceleration) == T_FIXNUM){
    Greenfield *ugen;
    Data_Get_Struct(self, Greenfield, ugen);
    ugen->setAcceleration( (float) NUM2DBL(acceleration) );    
  }
  else{
    rb_raise(rb_eTypeError, "Wrong argument type");
  }
	return self;
}

static VALUE Greenfield_set_threshold(VALUE self, VALUE threshold) 
{
  if(TYPE(threshold) == T_FLOAT || TYPE(threshold) == T_FIXNUM){
    // retrieve the Unit...
    Greenfield *ugen;
    Data_Get_Struct(self, Greenfield, ugen);
    // ... and add it to "out"
    ugen->setThreshold( (float) NUM2DBL(threshold) );    
  }
  else{
    rb_raise(rb_eTypeError, "Wrong argument type");
  }
	return self;
}

static VALUE Greenfield_set_call_potential(VALUE self, VALUE potential) 
{
  if(TYPE(potential) == T_FLOAT || TYPE(potential) == T_FIXNUM){
    // retrieve the Unit...
    Greenfield *greenfield;
    Data_Get_Struct(self, Greenfield, greenfield);
    float v = (float) NUM2DBL(potential);
    greenfield->setCallPotential( v );    
  }
  else{
    rb_raise(rb_eTypeError, "Wrong argument type");
  }
	return self;
}

static VALUE Greenfield_get_call_potential(VALUE self) 
{
  Greenfield *greenfield;
  Data_Get_Struct(self, Greenfield, greenfield);
  float potential = greenfield->callPotential();
  printf("!!!:%f\n",greenfield->callPotential());
	return rb_float_new((double)potential);
}

static VALUE Greenfield_set_max_neighbors(VALUE self, VALUE number)
{
  if(TYPE(number) == T_FIXNUM){
    Greenfield *greenfield;
    Data_Get_Struct(self, Greenfield, greenfield);
    float v = (float) NUM2INT(number);
    greenfield->setMaxNeighbors( v );    
  }
  else{
    rb_raise(rb_eTypeError, "Wrong argument type (Please use and INT)");
  }
	return self;
}

static VALUE Greenfield_get_max_neighbors(VALUE self) 
{
  Greenfield *greenfield;
  Data_Get_Struct(self, Greenfield, greenfield);
  int number = greenfield->maxNeighbors();
	return rb_int_new(number);
}

static VALUE Greenfield_initialize(int argc, VALUE *argv, VALUE self)
{
  // get the Unit instance
  Greenfield *ugen;
  Data_Get_Struct(self, Greenfield, ugen);
  VALUE options;
  
  /* Zero mandatory, one optional argument */
  /* Here, this optional arg is meant to be a Hash */
  rb_scan_args(argc, argv, "01", &options);
  
  if(options != Qnil)
  {
    // lets make sure its a hash
    Check_Type(options, T_HASH);

    // ACCELERATION
    VALUE acceleration = rb_hash_aref(options, ID2SYM(rb_intern("acceleration")));
    if(NIL_P(acceleration) == 0) Greenfield_set_acceleration(self, acceleration);
  
    // THRESHOLD - this is an alernate method which i fear is not effecient
    VALUE threshold = rb_hash_aref(options, ID2SYM(rb_intern("threshold")));
    if(NIL_P(threshold) == 0) Greenfield_set_threshold(self, threshold);
    
    // PERIOD
    VALUE period = rb_hash_aref(options, ID2SYM(rb_intern("period"))); 
    if(TYPE(period) == T_FLOAT || TYPE(period) == T_FIXNUM){
      float period_f = (float) NUM2DBL(period);  
      ugen->setPeriod(period_f);
    }
    
    // DECENDING LENGTH
    VALUE decending_length = rb_hash_aref(options, ID2SYM(rb_intern("decending_length"))); 
    if(TYPE(decending_length) == T_FLOAT || TYPE(decending_length) == T_FIXNUM){
      float decending_length_f = (float) NUM2DBL(decending_length);  
      ugen->setDecendingLength(decending_length_f);
    }
    
    // EFFECTOR DELAY
    VALUE effector_delay = rb_hash_aref(options, ID2SYM(rb_intern("effector_delay"))); 
    if(TYPE(effector_delay) == T_FLOAT || TYPE(effector_delay) == T_FIXNUM){
       float effector_delay_f = (float) NUM2DBL(effector_delay);
       ugen->setEffectorDelay(effector_delay_f);
     }
    
    // PRC SLOPE
    VALUE prc_slope = rb_hash_aref(options, ID2SYM(rb_intern("prc_slope")));
    if(TYPE(prc_slope) == T_FLOAT || TYPE(prc_slope) == T_FIXNUM){
      float prc_slope_f = (float) NUM2DBL(prc_slope);
      ugen->setPrcSlope(prc_slope_f);
    }  
    
    // REBOUND
    VALUE rebound = rb_hash_aref(options, ID2SYM(rb_intern("rebound")));
    if(TYPE(rebound) == T_FLOAT || TYPE(rebound) == T_FIXNUM){
      float rebound_f = (float) NUM2DBL(rebound);
      ugen->setRebound(rebound_f);
    }

    // CALL DURATION
    VALUE call_duration = rb_hash_aref(options, ID2SYM(rb_intern("call_duration")));
    if(TYPE(call_duration) == T_FLOAT || TYPE(call_duration) == T_FIXNUM){
      float call_duration_f = (float) NUM2DBL(call_duration);
      ugen->host_data["call_duration"] = call_duration_f;
    }  
    
    // X
    VALUE x = rb_hash_aref(options, ID2SYM(rb_intern("x")));
    if(TYPE(x) == T_FLOAT || TYPE(x) == T_FIXNUM){
      float x_f = (float) NUM2DBL(x);
      ugen->host_data["x"] = x_f;
    }
    
    // Y
    VALUE y = rb_hash_aref(options, ID2SYM(rb_intern("y")));
    if(TYPE(y) == T_FLOAT || TYPE(y) == T_FIXNUM){
      float y_f = (float) NUM2DBL(y);
      ugen->host_data["y"] = y_f;
    }

    // SPL
    VALUE spl = rb_hash_aref(options, ID2SYM(rb_intern("spl")));
    if(TYPE(spl) == T_FLOAT || TYPE(spl) == T_FIXNUM){
      float spl_f = (float) NUM2DBL(spl);
      ugen->host_data["spl"] = spl_f;
      ugen->setSpl(spl_f);
    }
  } // end if
  // TODO: the following should accept the incoming coords as array x,y
	return self;
}


extern "C" void Init_greenfield() 
{ 
  // Greenfield
  GreenfieldClass = rb_define_class("Greenfield", rb_cObject);
  rb_define_alloc_func(GreenfieldClass, Greenfield_alloc);
  rb_define_method(GreenfieldClass, "initialize", (VALUE(*)(...)) Greenfield_initialize, -1);
  rb_define_method(GreenfieldClass, "name", (VALUE(*)(...)) Greenfield_name, 0);
  rb_define_method(GreenfieldClass, "host_data", (VALUE(*)(...)) Greenfield_get_host_data, 0);
  rb_define_method(GreenfieldClass, "host_data=", (VALUE(*)(...)) Greenfield_set_host_data, 1);
  rb_define_method(GreenfieldClass, "actuator", (VALUE(*)(...)) Greenfield_get_actuator, 0);
  rb_define_method(GreenfieldClass, "actuator=", (VALUE(*)(...)) Greenfield_set_actuator, 1);
  rb_define_method(GreenfieldClass, "threshold=", (VALUE(*)(...)) Greenfield_set_threshold, 1);
  rb_define_method(GreenfieldClass, "acceleration=", (VALUE(*)(...)) Greenfield_set_acceleration, 1);
  
  rb_define_method(GreenfieldClass, "call_potential", (VALUE(*)(...)) Greenfield_get_call_potential, 0);
  rb_define_method(GreenfieldClass, "call_potential=", (VALUE(*)(...)) Greenfield_set_call_potential, 1);
  
  rb_define_method(GreenfieldClass, "max_neighbors=", (VALUE(*)(...)) Greenfield_set_max_neighbors, 1);
  rb_define_method(GreenfieldClass, "max_neighbors", (VALUE(*)(...)) Greenfield_get_max_neighbors, 0);
  
  rb_include_module(GreenfieldClass, UnitModule);
}