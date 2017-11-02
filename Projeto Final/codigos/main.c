#include <msp430g2553.h>
#define SERVO1 BIT1 //P2.1 as TA1.1
#define SERVO2 BIT4 //P2.4 as TA1.2
#define ADC_CHANNELS 4
#define PARAMETRO_MEDIA 100
unsigned int adc[ADC_CHANNELS]={0};
unsigned int media=0;

void Setup_Servos(void);
void Atraso(volatile unsigned int x);
void Servos_Write(int azimute,int elevation);
void Setup_ADC(void);
void Rotate_Servos(int rotate);
void Read_LDRS(void);
void Read_ADC(void);

int main(void)
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  Setup_Servos();
  Servos_Write(0,0); //initializing at 0 degrees
  Atraso(500); //Delay for servos stability
  Setup_ADC();
  for(;;)
  {
    Read_ADC();
  }

  return 0;
}

void Setup_Servos(void)
{
  P2DIR |= SERVO1+SERVO2;
  P2SEL |= SERVO1+SERVO2;
  P2SEL2 &= ~(SERVO1+SERVO2);
}

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

void Servos_Write(int azimute,int elevation)
{
  /*
  600 - 0º
  2600 - 180º

  2000 ms --- 180º
  9º = 100 !
  */
  
  TA1CTL |= TACLR;
  TA1CCR0 = 20000;

  if((azimute==0) && (elevation==0))
  {
    TA1CCR1 = 1600;
    TA1CCR2 = 1600;
  }
  else
  {
    if((TA1CCR1 > 700) && (TA1CCR1 < 2600))
    {
      TA1CCR1 += azimute;
    }
    if((TA1CCR2 > 700) && (TA1CCR2 < 2600))
    {
      TA1CCR2 += elevation;
    }
  }
  TA1CCTL1 = OUTMOD_7;
  TA1CCTL2 = OUTMOD_7;
  TA1CTL = TASSEL_2 + ID_0 + MC_1;
}

void Setup_ADC(void)
{
  ADC10CTL0 |=  SREF_0 +  ADC10SHT_0 + MSC + ADC10ON + ADC10IE; //Reference from 
  //Vcc and Vss, sampling time of 16×ADC10CLKs, ADC10ON
  ADC10CTL1 |= INCH_1 + CONSEQ_3 + ADC10SSEL_3 + SHS_0; //Input channel A1 and A0; repeated sequence 
  ADC10AE0 |= BIT0+BIT1+BIT2+BIT3; // Analog Input in P1.0, P1.1, P1.2 and P1.3; 
  ADC10DTC1 = ADC_CHANNELS; // 4 conversions
  ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
}


void Rotate_Servos(int rotate)
{
  //1 - Rotate Up
  //2 - Rotate Down
  //3 - Rotate Left
  //4 - Rotate Right
  if(rotate==1)
  {
     Servos_Write(0,-50);
     Atraso(300);
  }
  else if(rotate==2)
  {
    Servos_Write(0,50);
    Atraso(300);
  }
  else if(rotate==3)
  {
    Servos_Write(50,0);
    Atraso(300);
  }
  else
  {
    Servos_Write(-50,0);
    Atraso(300);
  }
}

void Read_LDRS(void)
{
  if(media!=0)
  {
      if((adc[0] > (media+PARAMETRO_MEDIA)) || (adc[1] > (media+PARAMETRO_MEDIA)))
      {
        if(adc[2] > (media+PARAMETRO_MEDIA))
        {
          //rotate up
          Rotate_Servos(1);
        }
        else if (adc[3]> (media+PARAMETRO_MEDIA))
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
      else if((adc[2] > (media+PARAMETRO_MEDIA)) || (adc[3] > (media+PARAMETRO_MEDIA)))
      {
        if(adc[0] > (media+PARAMETRO_MEDIA))
        {
          //rotate up
          Rotate_Servos(1);
        }
        else if((adc[0] > (media+PARAMETRO_MEDIA)))
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
  __bis_SR_register(CPUOFF + GIE);
  Read_LDRS();
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

