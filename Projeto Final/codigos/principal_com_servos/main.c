#include <msp430g2553.h>
#define SERVO1 BIT1 //P2.1 as TA1.1
#define SERVO2 BIT4 //P2.4 as TA1.2
#define LED BIT6
#define ADC_CHANNELS 4
#define PARAMETRO_MEDIA 100
unsigned int adc[ADC_CHANNELS];
volatile unsigned int media=0;
#define L0 adc[3]
#define L1 adc[2]
#define R0 adc[1]
#define R1 adc[0]

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

void Setup_LED()
{
  P1OUT &= ~BIT6; //Inicializar LED desligado
  P1DIR |= BIT6;
}

void Acender_LED(char string[])
{
  if(string == "ON")
  {
    P1OUT |= LED;
  }
  else if(string == "OFF")
  {
    P1OUT &= ~LED;
  }
  else if (string == "PISCAR")
  {
    P1OUT ^= LED;
    Atraso(500);
  }
}
void Setup_ADC(void)
{
  ADC10CTL0 |=  SREF_0 +  ADC10SHT_0 + MSC + ADC10ON + ADC10IE; //Reference from 
  //Vcc and Vss, sampling time of 16×ADC10CLKs, ADC10ON
  ADC10CTL1 |= INCH_3 + CONSEQ_1 + ADC10SSEL_3 + SHS_0; //Input channel A3, A2, A1 and A0; repeated sequence 
  ADC10AE0 |= (BIT0+BIT1+BIT2+BIT3); // Analog Input in P1.0, P1.1, P1.2 and P1.3; 
  ADC10DTC1 = ADC_CHANNELS; // 4 conversions
  ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
}

void Read_LDRS()
{
  //L0 - Sensor da Esquerda 0
  //L1 - Sensor da Esquerda 1
  //R0 - Sensor da Direita 0
  //R1 - Sensor da Direita 1
  if(media!=0)
  {
    if((L0 < (media - PARAMETRO_MEDIA)) || (L1 < (media - PARAMETRO_MEDIA)))
    {
      //um dos sensores da esquerda detectou maior intensidade de luz
      if((L0 < (media - PARAMETRO_MEDIA)) && (R0 < (media - PARAMETRO_MEDIA)))
      {
        //os dois sensores de cima detectaram maior intensidade de luz
        //girar para cima
        Acender_LED("ON");
        media=(L1+R1)/2;
      }
      else if ((L1 < (media - PARAMETRO_MEDIA)) && (R1 < (media - PARAMETRO_MEDIA)))
      {
       //os dois sensores de baixo detectaram maior intensidade de luz
       //girar para baixo
        Acender_LED("ON");
        media = (L0 + R0)/2;
      }
      else if (L0 < (media - PARAMETRO_MEDIA))
      {
        //sensor da esquerda 0 detectou maior intensidade de luz
        //girar para a esquerda
        Acender_LED("PISCAR");
        media = (L1+R0+R1)/3;
      }
      else if (L1 < (media - PARAMETRO_MEDIA))
      {
        //sensor da esquerda 1 detectou maior intensidade de luz
        //girar para a esquerda
        Acender_LED("PISCAR");
        media = (L0+R0+R1)/3;        
      }
    }
    else if((R0 < (media - PARAMETRO_MEDIA)) || (R1 < (media - PARAMETRO_MEDIA)))
    {
      //um dos sensores da direta detectou maior intensidade de luz
      if((L0 < (media - PARAMETRO_MEDIA)) && (R0 < (media - PARAMETRO_MEDIA)))
      {
        //os dois sensores de cima detectaram maior intensidade de luz
        //girar para cima
        Acender_LED("ON");
        media=(L1+R1)/2;
      }
      else if ((L1 < (media - PARAMETRO_MEDIA)) && (R1 < (media - PARAMETRO_MEDIA)))
      {
       //os dois sensores de baixo detectaram maior intensidade de luz
       //girar para baixo
        Acender_LED("ON");
        media = (L0 + R0)/2;
      }
      else if (R0 < (media - PARAMETRO_MEDIA))
      {
        //sensor da direita 0 detectou maior intensidade de luz
        //girar para a direita
        Acender_LED("PISCAR");
        media = (L0+L1+R1)/3;
      }
      else if (R1 < (media - PARAMETRO_MEDIA))
      {
        //sensor da direita 1 detectou maior intensidade de luz
        //girar para a direita
        Acender_LED("PISCAR");
        media = (L0+L1+R0)/3;        
      }
    }
    else
    {
      media = (L0 + L1 + R0 + R1)/4;
      Acender_LED("OFF");
    }
  }
  else
  {
     media = (L0 + L1 + R0 + R1)/4;
  }
}


void Read_ADC(void)
{
  ADC10CTL0 &= ~ENC;
  while (ADC10CTL1 & BUSY);// Wait if ADC10 core is active
  ADC10SA = (unsigned int)adc;// Copies data in ADC10SA to unsigned int adc array
  ADC10CTL0 |= ENC + ADC10SC;
 // __bis_SR_register(CPUOFF + GIE);
  Read_LDRS();
}

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  Setup_ADC();
  Setup_LED();
  for(;;)
  {
    Read_ADC();
  }

  return 0;
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}