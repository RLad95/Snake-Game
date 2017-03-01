#include <LPC17xx.H> /* LPC17xx definitions */
#include "KBD.h"
uint32_t KBD_val = 0;
/*----------------------------------------------------------------------------
 initialize Joystick
 *----------------------------------------------------------------------------*/
void KBD_Init (void) {
	LPC_SC->PCONP |= (1 << 15); /* enable power to GPIO & IOCON */
	/* P1.20, P1.23..26 is GPIO (Joystick) */
	
	LPC_PINCON->PINSEL4 &= ~(3<<20);           /* P2.10 is GPIO (INT0)          */
	LPC_GPIO2->FIODIR   &= ~(1<<10);           /* P2.10 is input   
	
	 LPC_PINCON->PINSEL3 &= ~((3<< 8)|(3<<14)|(3<<16)|(3<<18)|(3<<20));
	/* P1.20, P1.23..26 is input */
	 LPC_GPIO1->FIODIR &= ~((1<<20)|(1<<23)|(1<<24)|(1<<25)|(1<<26));
}
/*----------------------------------------------------------------------------
 Get Joystick value.. part of get_button
 *----------------------------------------------------------------------------*/
uint32_t KBD_get (void) {
	 uint32_t kbd_val;
	 kbd_val = (LPC_GPIO1->FIOPIN >> 20) & KBD_MASK;
	 return (kbd_val);
}
/*----------------------------------------------------------------------------
 Get Joystick value
 *----------------------------------------------------------------------------*/
uint32_t get_button (void) {
	 uint32_t val = 0;
	 val = KBD_get(); /* read Joystick state */
	 val = (~val & KBD_MASK); /* key pressed is read as a non '0' value*/
	 return (val);
} 

uint32_t INT0_Get  (void) {
  uint32_t int0_val;
  int0_val = (LPC_GPIO2->FIOPIN >> 10) & 0x01;
  return (int0_val);
}