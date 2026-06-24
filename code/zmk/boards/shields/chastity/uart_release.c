#include <zephyr/init.h>
#include <hal/nrf_uarte.h>
#include <cmsis_core.h>

/* Force UICR.NFCPINS to GPIO mode if it's still in NFC mode, releasing
 * P0.09 (NFC1) and P0.10 (NFC2). Zephyr's boot-time SoC code is supposed
 * to do this when nfct-pins-as-gpios is set in DT, but on these MCUs it
 * apparently isn't taking effect (intermittent ] / 1 spam survives the
 * standard flow). Programming UICR directly here is identical to what
 * Zephyr does internally; UICR change requires a soft reset to apply. */
static void chastity_program_nfc_uicr(void)
{
	if ((NRF_UICR->NFCPINS & UICR_NFCPINS_PROTECT_Msk) !=
	    (UICR_NFCPINS_PROTECT_Disabled << UICR_NFCPINS_PROTECT_Pos)) {
		NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
		while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
		}
		NRF_UICR->NFCPINS &= ~UICR_NFCPINS_PROTECT_Msk;
		while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
		}
		NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
		while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
		}
		NVIC_SystemReset();
	}
}

/* The Adafruit nice!nano bootloader leaves UARTE0 enabled with
 * PSEL.TXD=P0.06 and PSEL.RXD=P0.08. With uart0 status="disabled" in DT,
 * the Zephyr UART driver never runs and never clears that hold — so kscan
 * can't drive P0.06/P0.08, which is why right-half cols 8 and 9 are silent.
 * Run this before any driver init to force UARTE0 off and free its pins. */
static int chastity_release_uart_pins(void)
{
	chastity_program_nfc_uicr();
	nrf_uarte_disable(NRF_UARTE0);
	nrf_uarte_txrx_pins_disconnect(NRF_UARTE0);
	nrf_uarte_hwfc_pins_disconnect(NRF_UARTE0);
	return 0;
}

SYS_INIT(chastity_release_uart_pins, PRE_KERNEL_1, 50);
