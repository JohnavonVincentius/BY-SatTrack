#ifndef _EQEP_H_
#define _EQEP_H_

enum eqepFunction{
  quadrature,
  pulse_dir,
  increase,
  decrease
};

#ifdef EQEP0_CTR0_DEV
  int eqep0_INIT(enum eqepFunction func);
  int eqep0_setCelling(long ceil);
  long eqep0_getCount();

  void eqep0_disable();
  void eqep0_enable();
#endif

#endif
