
#include <msp430g2553.h>
#define SERVO1 BIT1 //P2.1 as TA1.1
#define SERVO2 BIT4 //P2.4 as TA1.2
volatile unsigned int i;
void Servos_Write(void)
{
  TA1CTL |= TACLR;
  TA1CCR0 = 20000;
/*
  1 ms - 0º
  2 ms - 180º
  angle = 5,6*t_us  
*/  
  TA1CCR1 = 2600;
  TA1CCR2 = 1600;
  TA1CCTL1 = OUTMOD_7;
  TA1CCTL2 = OUTMOD_7;
  TA1CTL = TASSEL_2 + ID_0 + MC_1;

  //600 us is the angle 0
  //2600 us is the angle 180
  
  /*
  TA0CTL |= TACLR;
  TA0CCR0 = 20000;
  TA0CCR1 = 600;
  TA0CCTL1 = OUTMOD_7;
  TA0CTL |= TASSEL_2 + ID_0 + MC_1;
  */
}

void Setup_Servos(void)
{
  P2DIR |= SERVO1+SERVO2;
  P2SEL |= SERVO1+SERVO2;
  P2SEL2 &= ~(SERVO1+SERVO2);
  /*P1DIR |= SERVO1;
  P1SEL |= SERVO1;
  P1SEL2 &= ~SERVO1;*/
}


int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  WDTCTL = WDTPW + WDTHOLD;
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  Setup_Servos();
  Servos_Write();
  while(1)
  {
    
  }
  return 0;
}
