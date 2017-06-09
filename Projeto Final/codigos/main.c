
#include <msp430g2553.h>
/*
Servo 1 refers to the azimute
Servo 2 refers to the elevation
*/
int write_angle(int time)
{
  float felevation;
  int elevation;
  if(time>23)
  {
    felevation = -2.53*time + 117.0;
  }
  else
  {
    felevation = 2.41*time + 2.61;
    elevation = (int)felevation;
  }
  return elevation;
}

void servo1_write(int angle)
{
  TA1CCR0 = 20000;
/*
  1 ms - 0º
  2 ms - 180º
  angle = 5,6*t_us  
*/  
  TA1CCR1 = 5*angle + 1000;
  TA1CCTL1 = OUTMOD_7;
  TA1CTL = TASSEL_2 + ID_0 + MC_1;
}

void servo2_write(int angle)
{
  TA1CCR2 = 5*angle + 1000;
  TA1CCTL2 = OUTMOD_7;
}

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  P2DIR |= BIT1 + BIT4; //OUT1 is in P2.1 and OUT2 is in P2.4
  P2SEL |= BIT1 + BIT4;
  P2SEL2 &= ~(BIT0+BIT4);
  servo1_write(50);
  servo2_write(write_angle(22));
  for(;;)
  {
    
  }

  return 0;
}
