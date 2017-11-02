#include <msp430g2553.h>
#define SERVO1 BIT1 //P2.1 as TA1.1
#define SERVO2 BIT4 //P2.4 as TA1.2
#define LED BIT6
#define ADC_CHANNELS 4
#define PARAMETRO_MEDIA 10
#define GIRO_DIREITO 100  
#define GIRO_ESQUERDO -100
#define GIRO_CIMA 100
#define GIRO_BAIXO -100
#define ATRASO_GIRO 25
#define L0 adc[3]
#define L1 adc[2]
#define R1 adc[1]
#define R0 adc[0]
unsigned int adc[ADC_CHANNELS];
volatile unsigned int media=0;
unsigned int comparacao;
unsigned int init=0;
unsigned int new_config=0;
unsigned int P_TA1CCR1=0;
unsigned int P_TA1CCR2=0;

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

void Setup_Servos(void)
{
  P2DIR |= SERVO1+SERVO2;
  P2SEL |= SERVO1+SERVO2;
  P2SEL2 &= ~(SERVO1+SERVO2);
}

void Servos_PWM(int azimute, int elevation)
{
  if(init==0)
  {
    TA1CTL |= TACLR;
    TA1CCR0 = 20000;
    TA1CCR1 = 1600;
    TA1CCR2 = 1300;
    TA1CCTL1 = OUTMOD_7;
    TA1CCTL2 = OUTMOD_7;
    TA1CTL = TASSEL_2 + ID_0 + MC_1;
    init++;
    P_TA1CCR1 = TA1CCR1;
    P_TA1CCR2 = TA1CCR2;
  }
//criterio para eixo polar
  if((TA1CCR1 > 600) && (TA1CCR1 < 2600))
  {
    P_TA1CCR1 = TA1CCR1+azimute;
    new_config=1;
  }
  else if((TA1CCR1==600) && (azimute > 0))
  {
    P_TA1CCR1 = TA1CCR1+azimute;
    new_config=1;
  }
  else if((TA1CCR1==2600) && (azimute < 0))
  {
    P_TA1CCR1 = TA1CCR1+azimute;
    new_config=1;
  }
//criterio para elevacao  
  if((TA1CCR2>600)&&(TA1CCR2<1300))
  {
    P_TA1CCR2 = TA1CCR2+elevation;
    new_config=1;
  }
  else if((TA1CCR2==600) && (elevation > 0))
  {
    P_TA1CCR2 = TA1CCR2+elevation;
    new_config=1;
  }
  else if((TA1CCR2==1300) && (elevation < 0))
  {
    P_TA1CCR2 = TA1CCR2+elevation;
    new_config=1;
  }
//criterio para nova configuracao  
  if(new_config==1)
  {
    TA1CTL |= TACLR;
    TA1CCR0 = 20000;    
    TA1CCR1 = P_TA1CCR1;
    TA1CCR2 = P_TA1CCR2;
    TA1CCTL1 = OUTMOD_7;
    TA1CCTL2 = OUTMOD_7;
    TA1CTL = TASSEL_2 + ID_0 + MC_1;
    new_config=0;
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
  //L0 - adc[3] - Sensor da Esquerda 0
  //L1 - adc[2] - Sensor da Esquerda 1
  //R0 - adc[0] - Sensor da Direita 0
  //R1 - adc[1] - Sensor da Direita 1
  if(media!=0)
  {
    comparacao = media - PARAMETRO_MEDIA;
      if((L0 < comparacao) && (R0 < comparacao))
      {
        //os dois sensores de cima detectaram maior intensidade de luz
        //girar para cima
        Servos_PWM(0,GIRO_BAIXO);
        media=(R0+L1)/2;
      }
      else if ((L1 < comparacao) && (R1 < comparacao))
      {
       //os dois sensores de baixo detectaram maior intensidade de luz
       //girar para baixo
        Servos_PWM(0,GIRO_CIMA);
        media = (R1 + L0)/2;
      }
      else if (L0 < comparacao)
      {
        //sensor da esquerda 0 detectou maior intensidade de luz
        //girar para a esquerda
        Servos_PWM(GIRO_ESQUERDO,0);
        media = (R0+R1+L1)/3;
        Atraso(ATRASO_GIRO);
      }
      else if (L1 < comparacao)
      {
        //sensor da esquerda 1 detectou maior intensidade de luz
        //girar para a esquerda
        //Rotate_Servos(3);
        Servos_PWM(GIRO_ESQUERDO,0);
        media = (L0+R0+R1)/3;
        Atraso(ATRASO_GIRO);        
      }
   
      else if (R1 < comparacao)
      {
        //sensor da direita 0 detectou maior intensidade de luz
        //girar para a direita
        //Rotate_Servos(4);
        Servos_PWM(GIRO_DIREITO,0);
        media = (L0+L1+R0)/3;
        Atraso(ATRASO_GIRO);
      }
      else if (R0 < comparacao)
      {
        //sensor da direita 1 detectou maior intensidade de luz
        //girar para a direita
        //Rotate_Servos(4);
        Servos_PWM(GIRO_DIREITO,0);
        media = (L0+L1+R1)/3;
        Atraso(ATRASO_GIRO);        
      }
      else
      {
      media = (adc[0] + adc[1] + adc[2] + adc[3])/4;
      }
  }
  else
  {
     media = (adc[0] + adc[1] + adc[2] + adc[3])/4;
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

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  Setup_ADC();
  Setup_Servos();
  Servos_PWM(0,0);
  Atraso(1000);
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