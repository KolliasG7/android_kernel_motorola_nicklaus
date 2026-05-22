/*
 * mt_server_stubs.c - Stub symbols for server kernel build
 *
 * When building a headless server kernel the following subsystems are
 * intentionally omitted:
 *   - SPM v1 (causes GCC10+ segfault; useless without modem/deep-sleep)
 *   - MTK display stack (no screen)
 *   - MTK LED / vibrator / timed-output (no hardware)
 *   - MTK dpidle / soidle (SPM-dependent idle drivers)
 *
 * The obj-y drivers in this directory still reference their exported
 * symbols, so we provide no-op stubs to satisfy the linker.
 */

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/io.h>

/* ---------- SPM base / lock (mt_spm.h) ----------------------------------- */
void __iomem *spm_base;
EXPORT_SYMBOL(spm_base);

DEFINE_SPINLOCK(__spm_lock);
EXPORT_SYMBOL(__spm_lock);

/* ---------- SPM internal PCM helpers (mt_spm_internal.h) ----------------- */
void __spm_reset_and_init_pcm(const void *pcmdesc) {}
EXPORT_SYMBOL(__spm_reset_and_init_pcm);

void __spm_kick_im_to_fetch(const void *pcmdesc) {}
EXPORT_SYMBOL(__spm_kick_im_to_fetch);

void __spm_init_pcm_register(void) {}
EXPORT_SYMBOL(__spm_init_pcm_register);

void __spm_init_event_vector(const void *pcmdesc) {}
EXPORT_SYMBOL(__spm_init_event_vector);

void __spm_set_power_control(const void *pwrctrl) {}
EXPORT_SYMBOL(__spm_set_power_control);

void __spm_set_wakeup_event(const void *pwrctrl) {}
EXPORT_SYMBOL(__spm_set_wakeup_event);

void __spm_kick_pcm_to_run(const void *pwrctrl) {}
EXPORT_SYMBOL(__spm_kick_pcm_to_run);

void spm_set_dram_bank_info_pcm_flag(unsigned int *pcm_flags) {}
EXPORT_SYMBOL(spm_set_dram_bank_info_pcm_flag);

/* ---------- SPM sleep (mt_spm_sleep.h) ----------------------------------- */
int spm_set_sleep_wakesrc(unsigned int wakesrc, bool enable, bool replace)
{
	return 0;
}
EXPORT_SYMBOL(spm_set_sleep_wakesrc);

int spm_go_to_sleep(unsigned int spm_flags, unsigned int spm_data)
{
	return 0;
}
EXPORT_SYMBOL(spm_go_to_sleep);

bool spm_is_md2_sleep(void)
{
	return false;
}
EXPORT_SYMBOL(spm_is_md2_sleep);

void spm_output_sleep_option(void) {}
EXPORT_SYMBOL(spm_output_sleep_option);

/* ---------- SPM module init (mt_spm.h) ----------------------------------- */
int spm_module_init(void)
{
	return 0;
}
EXPORT_SYMBOL(spm_module_init);

void spm_ap_bsi_gen(unsigned int *clk_buf_cfg) {}
EXPORT_SYMBOL(spm_ap_bsi_gen);

/* ---------- MTK idle (mt_idle.h) ----------------------------------------- */
void enable_dpidle_by_bit(int id) {}
EXPORT_SYMBOL(enable_dpidle_by_bit);

void disable_dpidle_by_bit(int id) {}
EXPORT_SYMBOL(disable_dpidle_by_bit);

void enable_soidle_by_bit(int id) {}
EXPORT_SYMBOL(enable_soidle_by_bit);

void disable_soidle_by_bit(int id) {}
EXPORT_SYMBOL(disable_soidle_by_bit);

/* ---------- MTK display (primary_display_*, mtkfb_*, disp_bls_*) --------- */
void primary_display_setbacklight_hbm(unsigned int level, bool enable) {}
EXPORT_SYMBOL(primary_display_setbacklight_hbm);

unsigned int primary_recognition_hbm_level(void)
{
	return 0;
}
EXPORT_SYMBOL(primary_recognition_hbm_level);

void primary_display_setcabc(int mode) {}
EXPORT_SYMBOL(primary_display_setcabc);

int primary_recognition_cabc_mode(void)
{
	return 0;
}
EXPORT_SYMBOL(primary_recognition_cabc_mode);

void primary_display_force_set_vsync_fps(unsigned int fps) {}
EXPORT_SYMBOL(primary_display_force_set_vsync_fps);

unsigned int primary_display_get_fps(void)
{
	return 60;
}
EXPORT_SYMBOL(primary_display_get_fps);

void mtkfb_set_backlight_level(unsigned int level) {}
EXPORT_SYMBOL(mtkfb_set_backlight_level);

void disp_bls_set_backlight(unsigned int level) {}
EXPORT_SYMBOL(disp_bls_set_backlight);

/* ---------- Timed output (vibrator) -------------------------------------- */
struct timed_output_dev;
int timed_output_dev_register(struct timed_output_dev *dev)
{
	return 0;
}
EXPORT_SYMBOL(timed_output_dev_register);
