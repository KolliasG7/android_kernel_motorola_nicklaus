/*
 * wmt_dev.c - stub replacement to work around GCC 4.9 ICE on this file.
 * The real wmt_dev.c creates /proc/wmt_dev and a char device for WMT userspace.
 * For a headless server kernel, userspace WMT control is not needed.
 * All symbols are stubbed to satisfy the linker.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

/* Satisfy external references from other WMT files */
int  wmt_dev_proc_for_aee_setup(void) { return 0; }
void wmt_dev_proc_for_aee_remove(void) {}
void wmt_dev_rx_event_cb(void) {}
void wmt_dev_rx_timeout(void) {}
int  wmt_dev_read_file(unsigned char *dst, unsigned int sz,
                       unsigned long offset, unsigned int *read_sz) { return -1; }
int  wmt_dev_patch_get(unsigned char *pPatchName, void **ppPatch, unsigned int *pPatchSize) { return -1; }
void wmt_dev_patch_put(unsigned char **ppPatch) {}
void wmt_dev_patch_info_free(void *pPatchInfo) {}
long WMT_write(void *filp, const char *buf, unsigned long count, long long *f_pos) { return -1; }
long WMT_read(void *filp, char *buf, unsigned long count, long long *f_pos) { return -1; }
int  wmt_dev_bgw_desense_init(void *pwmt_dev, unsigned long data) { return 0; }
void wmt_dev_bgw_desense_deinit(void) {}
int  wmt_dev_send_cmd_to_daemon(unsigned int cmd) { return 0; }

int  wmt_dev_get_early_suspend_state(void) { return 0; }
int  wmt_dev_tm_temp_query(void) { return 50; } /* return dummy 50°C */

static int __init wmt_dev_stub_init(void)
{
    pr_info("wmt_dev: stub loaded (server build)\n");
    return 0;
}

static void __exit wmt_dev_stub_exit(void)
{
    pr_info("wmt_dev: stub unloaded\n");
}

int mtk_wcn_common_drv_init(void) { return wmt_dev_stub_init(); }
void mtk_wcn_common_drv_exit(void) { wmt_dev_stub_exit(); }

module_init(wmt_dev_stub_init);
module_exit(wmt_dev_stub_exit);
