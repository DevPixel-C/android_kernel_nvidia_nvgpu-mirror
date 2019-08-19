/*
 * GM20B GPC MMU
 *
 * Copyright (c) 2014-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef CONFIG_NVGPU_TRACE
#include <trace/events/gk20a.h>
#endif

#include <nvgpu/sizes.h>
#include <nvgpu/utils.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/bug.h>
#include <nvgpu/ltc.h>

#include "fb_gm20b.h"

#include <nvgpu/io.h>
#include <nvgpu/timers.h>

#include <nvgpu/hw/gm20b/hw_fb_gm20b.h>

#define VPR_INFO_FETCH_WAIT	(5)
#define WPR_INFO_ADDR_ALIGNMENT 0x0000000c

#ifdef CONFIG_NVGPU_DEBUGGER
bool gm20b_fb_debug_mode_enabled(struct gk20a *g)
{
	u32 debug_ctrl = gk20a_readl(g, fb_mmu_debug_ctrl_r());
	return fb_mmu_debug_ctrl_debug_v(debug_ctrl) ==
			fb_mmu_debug_ctrl_debug_enabled_v();
}

void gm20b_fb_set_debug_mode(struct gk20a *g, bool enable)
{
	u32 reg_val, fb_debug_ctrl;

	if (enable) {
		fb_debug_ctrl = fb_mmu_debug_ctrl_debug_enabled_f();
		g->mmu_debug_ctrl = true;
	} else {
		fb_debug_ctrl = fb_mmu_debug_ctrl_debug_disabled_f();
		g->mmu_debug_ctrl = false;
	}

	reg_val = gk20a_readl(g, fb_mmu_debug_ctrl_r());
	reg_val = set_field(reg_val,
			fb_mmu_debug_ctrl_debug_m(), fb_debug_ctrl);
	gk20a_writel(g, fb_mmu_debug_ctrl_r(), reg_val);

	g->ops.gr.set_debug_mode(g, enable);
}
#endif

void gm20b_fb_init_hw(struct gk20a *g)
{
	u64 addr = nvgpu_mem_get_addr(g, &g->mm.sysmem_flush) >> 8;

	nvgpu_assert(u64_hi32(addr) == 0U);
	gk20a_writel(g, fb_niso_flush_sysmem_addr_r(), U32(addr));

	/* init mmu debug buffer */
	addr = nvgpu_mem_get_addr(g, &g->mm.mmu_wr_mem);
	addr >>= fb_mmu_debug_wr_addr_alignment_v();

	nvgpu_assert(u64_hi32(addr) == 0U);
	gk20a_writel(g, fb_mmu_debug_wr_r(),
		     nvgpu_aperture_mask(g, &g->mm.mmu_wr_mem,
				fb_mmu_debug_wr_aperture_sys_mem_ncoh_f(),
				fb_mmu_debug_wr_aperture_sys_mem_coh_f(),
				fb_mmu_debug_wr_aperture_vid_mem_f()) |
		     fb_mmu_debug_wr_vol_false_f() |
		     fb_mmu_debug_wr_addr_f(U32(addr)));

	addr = nvgpu_mem_get_addr(g, &g->mm.mmu_rd_mem);
	addr >>= fb_mmu_debug_rd_addr_alignment_v();

	nvgpu_assert(u64_hi32(addr) == 0U);
	gk20a_writel(g, fb_mmu_debug_rd_r(),
		     nvgpu_aperture_mask(g, &g->mm.mmu_rd_mem,
				fb_mmu_debug_wr_aperture_sys_mem_ncoh_f(),
				fb_mmu_debug_wr_aperture_sys_mem_coh_f(),
				fb_mmu_debug_rd_aperture_vid_mem_f()) |
		     fb_mmu_debug_rd_vol_false_f() |
		     fb_mmu_debug_rd_addr_f(U32(addr)));
}

int gm20b_fb_tlb_invalidate(struct gk20a *g, struct nvgpu_mem *pdb)
{
	struct nvgpu_timeout timeout;
	u32 addr_lo;
	u32 data;
	int err = 0;

	nvgpu_log_fn(g, " ");

	/* pagetables are considered sw states which are preserved after
	   prepare_poweroff. When gk20a deinit releases those pagetables,
	   common code in vm unmap path calls tlb invalidate that touches
	   hw. Use the power_on flag to skip tlb invalidation when gpu
	   power is turned off */

	if (!g->power_on) {
		return err;
	}

	addr_lo = u64_lo32(nvgpu_mem_get_addr(g, pdb) >> 12);

	nvgpu_mutex_acquire(&g->mm.tlb_lock);

#ifdef CONFIG_NVGPU_TRACE
	trace_gk20a_mm_tlb_invalidate(g->name);
#endif

	err = nvgpu_timeout_init(g, &timeout, 1000, NVGPU_TIMER_RETRY_TIMER);
	if (err != 0) {
		nvgpu_err(g, "nvgpu_timeout_init(mmu fifo space) failed err=%d",
			err);
		goto out;
	}

	do {
		data = gk20a_readl(g, fb_mmu_ctrl_r());
		if (fb_mmu_ctrl_pri_fifo_space_v(data) != 0U) {
			break;
		}
		nvgpu_udelay(2);
	} while (nvgpu_timeout_expired_msg(&timeout,
					 "wait mmu fifo space") == 0);

	if (nvgpu_timeout_peek_expired(&timeout)) {
		err = -ETIMEDOUT;
		goto out;
	}

	err = nvgpu_timeout_init(g, &timeout, 1000, NVGPU_TIMER_RETRY_TIMER);
	if (err != 0) {
		nvgpu_err(g, "nvgpu_timeout_init(mmu invalidate) failed err=%d",
			err);
		goto out;
	}

	gk20a_writel(g, fb_mmu_invalidate_pdb_r(),
		fb_mmu_invalidate_pdb_addr_f(addr_lo) |
		nvgpu_aperture_mask(g, pdb,
				fb_mmu_invalidate_pdb_aperture_sys_mem_f(),
				fb_mmu_invalidate_pdb_aperture_sys_mem_f(),
				fb_mmu_invalidate_pdb_aperture_vid_mem_f()));

	gk20a_writel(g, fb_mmu_invalidate_r(),
		fb_mmu_invalidate_all_va_true_f() |
		fb_mmu_invalidate_trigger_true_f());

	do {
		data = gk20a_readl(g, fb_mmu_ctrl_r());
		if (fb_mmu_ctrl_pri_fifo_empty_v(data) !=
			fb_mmu_ctrl_pri_fifo_empty_false_f()) {
			break;
		}
		nvgpu_udelay(2);
	} while (nvgpu_timeout_expired_msg(&timeout,
					 "wait mmu invalidate") == 0);

#ifdef CONFIG_NVGPU_TRACE
	trace_gk20a_mm_tlb_invalidate_done(g->name);
#endif

out:
	nvgpu_mutex_release(&g->mm.tlb_lock);
	return err;
}

u32 gm20b_fb_mmu_ctrl(struct gk20a *g)
{
	return gk20a_readl(g, fb_mmu_ctrl_r());
}

u32 gm20b_fb_mmu_debug_ctrl(struct gk20a *g)
{
	return gk20a_readl(g, fb_mmu_debug_ctrl_r());
}

u32 gm20b_fb_mmu_debug_wr(struct gk20a *g)
{
	return gk20a_readl(g, fb_mmu_debug_wr_r());
}

u32 gm20b_fb_mmu_debug_rd(struct gk20a *g)
{
	return gk20a_readl(g, fb_mmu_debug_rd_r());
}

void gm20b_fb_dump_vpr_info(struct gk20a *g)
{
	u32 val;
	u32 addr_lo, addr_hi, cya_lo, cya_hi;

	/* print vpr info */
	val = gk20a_readl(g, fb_mmu_vpr_info_r());
	val &= ~0x3U;
	val |= fb_mmu_vpr_info_index_addr_lo_v();
	gk20a_writel(g, fb_mmu_vpr_info_r(), val);

	addr_lo = gk20a_readl(g, fb_mmu_vpr_info_r());
	addr_hi = gk20a_readl(g, fb_mmu_vpr_info_r());
	cya_lo = gk20a_readl(g, fb_mmu_vpr_info_r());
	cya_hi = gk20a_readl(g, fb_mmu_vpr_info_r());

	nvgpu_err(g, "VPR: %08x %08x %08x %08x",
		addr_lo, addr_hi, cya_lo, cya_hi);
}

void gm20b_fb_dump_wpr_info(struct gk20a *g)
{
	u32 val;
	u32 allow_read, allow_write;
	u32 wpr1_addr_lo, wpr1_addr_hi;
	u32 wpr2_addr_lo, wpr2_addr_hi;

	/* print wpr info */
	val = gk20a_readl(g, fb_mmu_wpr_info_r());
	val &= ~0xfU;
	val |= (fb_mmu_wpr_info_index_allow_read_v());
	gk20a_writel(g, fb_mmu_wpr_info_r(), val);

	allow_read = gk20a_readl(g, fb_mmu_wpr_info_r());
	allow_write = gk20a_readl(g, fb_mmu_wpr_info_r());
	wpr1_addr_lo = gk20a_readl(g, fb_mmu_wpr_info_r());
	wpr1_addr_hi = gk20a_readl(g, fb_mmu_wpr_info_r());
	wpr2_addr_lo = gk20a_readl(g, fb_mmu_wpr_info_r());
	wpr2_addr_hi = gk20a_readl(g, fb_mmu_wpr_info_r());

	nvgpu_err(g, "WPR: %08x %08x %08x %08x %08x %08x",
		allow_read, allow_write,
		wpr1_addr_lo, wpr1_addr_hi,
		wpr2_addr_lo, wpr2_addr_hi);
}

static int gm20b_fb_vpr_info_fetch_wait(struct gk20a *g,
					    unsigned int msec)
{
	struct nvgpu_timeout timeout;
	int err = 0;

	err = nvgpu_timeout_init(g, &timeout, msec, NVGPU_TIMER_CPU_TIMER);
	if (err != 0) {
		nvgpu_err(g, "nvgpu_timeout_init failed err=%d", err);
		return err;
	}

	do {
		u32 val;

		val = gk20a_readl(g, fb_mmu_vpr_info_r());
		if (fb_mmu_vpr_info_fetch_v(val) ==
		    fb_mmu_vpr_info_fetch_false_v()) {
			return 0;
		}

	} while (nvgpu_timeout_expired(&timeout) == 0);

	return -ETIMEDOUT;
}

int gm20b_fb_vpr_info_fetch(struct gk20a *g)
{
	if (gm20b_fb_vpr_info_fetch_wait(g, VPR_INFO_FETCH_WAIT) != 0) {
		return -ETIMEDOUT;
	}

	gk20a_writel(g, fb_mmu_vpr_info_r(),
			fb_mmu_vpr_info_fetch_true_v());

	return gm20b_fb_vpr_info_fetch_wait(g, VPR_INFO_FETCH_WAIT);
}

void gm20b_fb_read_wpr_info(struct gk20a *g, u64 *wpr_base, u64 *wpr_size)
{
	u32 val = 0;
	u64 wpr_start = 0;
	u64 wpr_end = 0;

	val = gk20a_readl(g, fb_mmu_wpr_info_r());
	val &= ~0xFU;
	val |= fb_mmu_wpr_info_index_wpr1_addr_lo_v();
	gk20a_writel(g, fb_mmu_wpr_info_r(), val);

	val = gk20a_readl(g, fb_mmu_wpr_info_r()) >> 0x4;
	wpr_start = hi32_lo32_to_u64(
			(val >> (32 - WPR_INFO_ADDR_ALIGNMENT)),
			(val << WPR_INFO_ADDR_ALIGNMENT));

	val = gk20a_readl(g, fb_mmu_wpr_info_r());
	val &= ~0xFU;
	val |= fb_mmu_wpr_info_index_wpr1_addr_hi_v();
	gk20a_writel(g, fb_mmu_wpr_info_r(), val);

	val = gk20a_readl(g, fb_mmu_wpr_info_r()) >> 0x4;
	wpr_end = hi32_lo32_to_u64(
			(val >> (32 - WPR_INFO_ADDR_ALIGNMENT)),
			(val << WPR_INFO_ADDR_ALIGNMENT));

	*wpr_base = wpr_start;
	*wpr_size = nvgpu_safe_sub_u64(wpr_end, wpr_start);
}
