/* xenolinux/include/asm-xeno/keyboard.h */
/* Portions copyright (c) 2003 James Scott, Intel Research Cambridge */
/* Talks to hypervisor to get PS/2 keyboard and mouse events, and send keyboard and mouse commands */

/*  Based on:
 *  linux/include/asm-i386/keyboard.h
 *
 *  Created 3 Nov 1996 by Geert Uytterhoeven
 */

#ifndef _XENO_KEYBOARD_H
#define _XENO_KEYBOARD_H

#ifdef __KERNEL__

#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/kd.h>
#include <linux/pm.h>
#include <asm/io.h>


#define KEYBOARD_IRQ _EVENT_KBD
#define AUX_IRQ _EVENT_KBD

/* THIS SECTION RELATIVELY UNTOUCHED */

extern int pckbd_setkeycode(unsigned int scancode, unsigned int keycode);
extern int pckbd_getkeycode(unsigned int scancode);
extern int pckbd_translate(unsigned char scancode, unsigned char *keycode,
			   char raw_mode);
extern char pckbd_unexpected_up(unsigned char keycode);
extern void pckbd_leds(unsigned char leds);
extern void pckbd_init_hw(void);
extern int pckbd_pm_resume(struct pm_dev *, pm_request_t, void *);

extern pm_callback pm_kbd_request_override;
extern unsigned char pckbd_sysrq_xlate[128];

#define kbd_setkeycode		pckbd_setkeycode
#define kbd_getkeycode		pckbd_getkeycode
#define kbd_translate		pckbd_translate
#define kbd_unexpected_up	pckbd_unexpected_up
#define kbd_leds		pckbd_leds
#define kbd_init_hw		pckbd_init_hw
#define kbd_sysrq_xlate		pckbd_sysrq_xlate

#define SYSRQ_KEY 0x54


/* THIS SECTION TALKS TO XEN TO DO PS2 SUPPORT */
#include <asm/hypervisor-ifs/kbd.h>
#include <asm/hypervisor-ifs/hypervisor-if.h>

#define kbd_controller_present xen_kbd_controller_present

static inline int xen_kbd_controller_present ()
{
	if( start_info.flags & SIF_CONSOLE )
		{
		printk("Enable keyboard\n");
		return 1;
		}
	else
		return 0;
}

/* resource allocation */
#define kbd_request_region() do { } while (0)
#define kbd_request_irq(handler) request_irq(_EVENT_KBD, handler, 0, "PS/2 kbd", NULL)

// could implement these with command to xen to filter mouse stuff...
#define aux_request_irq(hand, dev_id) 0
#define aux_free_irq(dev_id) do { } while(0)

/* Some stoneage hardware needs delays after some operations.  */
#define kbd_pause() do { } while(0)


static unsigned char kbd_current_scancode = 0;

static unsigned char kbd_read_input(void) 
{
  //xprintk("kbd_read_input: returning scancode 0x%2x\n", kbd_current_scancode);
  return kbd_current_scancode;
}

static unsigned char kbd_read_status(void) 
{
  long res;
  res = HYPERVISOR_kbd_op(KBD_OP_READ,0);
  if(res<0) {
    //printk("kbd_read_status: error from hypervisor: %d", res);
    kbd_current_scancode = 0;
    return 0; // error with our request - wrong domain?
  }
  kbd_current_scancode = KBD_CODE_SCANCODE(res);
  //printk("kbd_read_status: returning status 0x%2x\n", KBD_CODE_STATUS(res));
  return KBD_CODE_STATUS(res);
}


#define kbd_write_output(val)  HYPERVISOR_kbd_op(KBD_OP_WRITEOUTPUT, val);
#define kbd_write_command(val) HYPERVISOR_kbd_op(KBD_OP_WRITECOMMAND, val);


#endif /* __KERNEL__ */
#endif /* _XENO_KEYBOARD_H */
