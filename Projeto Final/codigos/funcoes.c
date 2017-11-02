#include <msp430g2553.h>
#define SERVO1 BIT1 //P2.1 as TA1.1
#define SERVO2 BIT4 //P2.4 as TA1.2
#define ADC_CHANNELS 4
unsigned int adc[ADC_CHANNELS]={0};
unsigned int media=0;


void Atraso(volatile unsigned int x)
{
    TA0CCR0 = 1000-1; 
    TA0CTL |= TACLR; //clear timer
    TA0CTL = TASSEL_2 + ID_0 + MC_1; 
    while(x>0)
    {
      x--;
      while((TA0CTL&TAIFG)==0);
              TA0CTL &= ~TAIFG;
    }
    TA0CTL = MC_0; //stop timer
}

void Setup_Servos(void)
{
  P2DIR |= SERVO1+SERVO2;
  P2SEL |= SERVO1+SERVO2;
  P2SEL2 &= ~(SERVO1+SERVO2);
}

void Servos_Write(int azimute,int elevation)
{
  TA1CTL |= TACLR;
  TA1CCR0 = 20000;
/*
  600 - 0º
  2600 - 180º

  2000 ms --- 180º
  9º = 100 !
  
*/
  if((azimute==0) && (elevation==0))
  {
    TA1CCR1 = 1600;
    TA1CCR2 = 1600;
  }
  else
  {
    if((TA1CCR1 > 600) && (TA1CCR1 < 2600))
    {
      TA1CCR1 += azimute;
    }
    if((TA1CCR2 > 600) && (TA1CCR2 < 2600))
    {
      TA1CCR2 += elevation;
    }
  }
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

void Setup_ADC(void)
{
  ADC10CTL0 |=  SREF_0 +  ADC10SHT_0 + MSC + ADC10ON + ADC10IE; //Reference from 
  //Vcc and Vss, sampling time of 16×ADC10CLKs, ADC10ON
  ADC10CTL1 |= INCH_1 + CONSEQ_3 + ADC10SSEL_3 + SHS_0; //Input channel A1 and A0; repeated sequence 
  ADC10AE0 = BIT0+BIT1+BIT2+BIT3; // Analog Input in P1.0, P1.1, P1.2 and P1.3; 
  ADC10DTC1 = ADC_CHANNELS; // 4 conversions
  ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
}

void Rotate_Servos(int rotate)
{
  volatile unsigned int i = 5000;
  //1 - Rotate Up
  //2 - Rotate Down
  //3 - Rotate Left
  //4 - Rotate Right
  if(rotate==1)
  {
     Servos_Write(0,-100);
     while(i--);
  }
  else if(rotate==2)
  {
    Servos_Write(0,100);
    while(i--);
  }
  else if(rotate==3)
  {
    Servos_Write(100,0);
    while(i--);
  }
  else
  {
    Servos_Write(-100,0);
    while(i--);
  }
}

void Read_LDRS(void)
{
  if(media!=0)
  {
      if((adc[0] > (media+100)) || (adc[1] > (media+100)))
      {
        if(adc[2] > (media+100))
        {
          //rotate up
          Rotate_Servos(1);
        }
        else if (adc[3]> (media+100))
        {
          //rotate down
          Rotate_Servos(2);
        }
        else
        {
          //rotate left
          Rotate_Servos(3);
        }
      }
      else if((adc[2] > (media+100)) || (adc[3] > (media+100)))
      {
        if(adc[0] > (media+100))
        {
          //rotate up
          Rotate_Servos(1);
        }
        else if((adc[0] > (media+100)))
        {
          //rotate down
          Rotate_Servos(2);
        }
        else
        {
          //rotate right
          Rotate_Servos(4);
        }
      }
      media = (adc[0]+adc[1]+adc[2]+adc[3])/4;
  }
  else
  {
    media = (adc[0]+adc[1]+adc[2]+adc[3])/4;
  }
}

void Read_ADC(void)
{
  ADC10CTL0 &= ~ENC;
  while (ADC10CTL1 & BUSY);// Wait if ADC10 core is active
  ADC10SA = (unsigned int)adc;// Copies data in ADC10SA to unsigned int adc array
  ADC10CTL0 |= ENC + ADC10SC;
  //__bis_SR_register(CPUOFF + GIE);
  Read_LDRS();
}
