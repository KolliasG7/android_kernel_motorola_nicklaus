/*
 * mt_server_stubs.c - Weak stub symbols for server kernel build
 *
 * All functions are declared __weak so that:
 *   - Android builds: real implementations win automatically
 *   - Server builds:  these stubs satisfy the linker
 *
 * Subsystems stubbed out here are intentionally absent in the server kernel:
 *   - SPM v1 (GCC10+ segfault; useless without modem/deep-sleep)
 *   - MTK display stack (headless server, no screen)
 *   - MTK LED / vibrator / timed-output (no hardware on server)
 *   - MTK dpidle / soidle idle drivers (SPM-dependent)
 *   - MMDVFS / mmclk callbacks (display memory bandwidth mgr)
 */

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/export.h>

/* ---------- SPM base / lock (mt_spm.h) ----------------------------------- */
void __iomem * __weak spm_base;
EXPORT_SYMBOL(spm_base);

DEFINE_SPINLOCK(__spm_lock);
/* __spm_lock is a global spinlock — can't be __weak, but the linker
 * will use this definition when spm_v1 is absent. */
EXPORT_SYMBOL(__spm_lock);

/* ---------- SPM internal PCM helpers (mt_spm_internal.h) ----------------- */
void __weak __spm_reset_and_init_pcm(const void *pcmdesc) {}
EXPORT_SYMBOL(__spm_reset_and_init_pcm);

void __weak __spm_kick_im_to_fetch(const void *pcmdesc) {}
EXPORT_SYMBOL(__spm_kick_im_to_fetch);

void __weak __spm_init_pcm_register(void) {}
EXPORT_SYMBOL(__spm_init_pcm_register);

void __weak __spm_init_event_vector(const void *pcmdesc) {}
EXPORT_SYMBOL(__spm_init_event_vector);

void __weak __spm_set_power_control(const void *pwrctrl) {}
EXPORT_SYMBOL(__spm_set_power_control);

void __weak __spm_set_wakeup_event(const void *pwrctrl) {}
EXPORT_SYMBOL(__spm_set_wakeup_event);

void __weak __spm_kick_pcm_to_run(const void *pwrctrl) {}
EXPORT_SYMBOL(__spm_kick_pcm_to_run);

void __weak spm_set_dram_bank_info_pcm_flag(unsigned int *pcm_flags) {}
EXPORT_SYMBOL(spm_set_dram_bank_info_pcm_flag);

/* ---------- SPM sleep (mt_spm_sleep.h) ----------------------------------- */
int __weak spm_set_sleep_wakesrc(unsigned int wakesrc, bool enable, bool replace)
{
	return 0;
}
EXPORT_SYMBOL(spm_set_sleep_wakesrc);

int __weak spm_go_to_sleep(unsigned int spm_flags, unsigned int spm_data)
{
	return 0;
}
EXPORT_SYMBOL(spm_go_to_sleep);

bool __weak spm_is_md2_sleep(void)
{
	return false;
}
EXPORT_SYMBOL(spm_is_md2_sleep);

void __weak spm_output_sleep_option(void) {}
EXPORT_SYMBOL(spm_output_sleep_option);

/* ---------- SPM module init (mt_spm.h) ----------------------------------- */
int __weak spm_module_init(void)
{
	return 0;
}
EXPORT_SYMBOL(spm_module_init);

void __weak spm_ap_bsi_gen(unsigned int *clk_buf_cfg) {}
EXPORT_SYMBOL(spm_ap_bsi_gen);

/* ---------- MTK idle (mt_idle.h) ----------------------------------------- */
void __weak enable_dpidle_by_bit(int id) {}
EXPORT_SYMBOL(enable_dpidle_by_bit);

void __weak disable_dpidle_by_bit(int id) {}
EXPORT_SYMBOL(disable_dpidle_by_bit);

void __weak enable_soidle_by_bit(int id) {}
EXPORT_SYMBOL(enable_soidle_by_bit);

void __weak disable_soidle_by_bit(int id) {}
EXPORT_SYMBOL(disable_soidle_by_bit);

/* ---------- MTK display (primary_display_*, mtkfb_*, disp_bls_*) --------- */
void __weak primary_display_setbacklight_hbm(unsigned int level, bool enable) {}
EXPORT_SYMBOL(primary_display_setbacklight_hbm);

unsigned int __weak primary_recognition_hbm_level(void) { return 0; }
EXPORT_SYMBOL(primary_recognition_hbm_level);

void __weak primary_display_setcabc(int mode) {}
EXPORT_SYMBOL(primary_display_setcabc);

int __weak primary_recognition_cabc_mode(void) { return 0; }
EXPORT_SYMBOL(primary_recognition_cabc_mode);

void __weak primary_display_force_set_vsync_fps(unsigned int fps) {}
EXPORT_SYMBOL(primary_display_force_set_vsync_fps);

unsigned int __weak primary_display_get_fps(void) { return 60; }
EXPORT_SYMBOL(primary_display_get_fps);

void __weak mtkfb_set_backlight_level(unsigned int level) {}
EXPORT_SYMBOL(mtkfb_set_backlight_level);

void __weak disp_bls_set_backlight(unsigned int level) {}
EXPORT_SYMBOL(disp_bls_set_backlight);

/* ---------- Timed output (vibrator) -------------------------------------- */
struct timed_output_dev;
int __weak timed_output_dev_register(struct timed_output_dev *dev) { return 0; }
EXPORT_SYMBOL(timed_output_dev_register);

/* ---------- MMDVFS (SMI memory bandwidth) -------------------------------- */
/* mmdvfs_set_mmsys_clk is referenced by camera_pipe_mgr_D2.c           */
int __weak mmdvfs_set_mmsys_clk(int scenario, int mmsys_clk_mode) { return 0; }
EXPORT_SYMBOL(mmdvfs_set_mmsys_clk);

/* register_mmclk_switch_vdec_ctrl_cb: videocodec registers a callback   */
int __weak register_mmclk_switch_vdec_ctrl_cb(void *cb) { return 0; }
EXPORT_SYMBOL(register_mmclk_switch_vdec_ctrl_cb);
