/*
 * Copyright (c) 2018-2022, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvgpu/types.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/netlist.h>
#include <nvgpu/gr/gr_falcon.h>

#include "gr_pri_gk20a.h"
#include "gr_tu104.h"
#include "hal/gr/gr/gr_gv11b.h"

#include <nvgpu/hw/tu104/hw_gr_tu104.h>

int gr_tu104_get_offset_in_gpccs_segment(struct gk20a *g,
					enum ctxsw_addr_type addr_type,
					u32 num_tpcs,
					u32 num_ppcs,
					u32 reg_list_ppc_count,
					u32 *__offset_in_segment)
{
	u32 offset_in_segment = 0;
	u32 num_pes_per_gpc = nvgpu_get_litter_value(g,
				GPU_LIT_NUM_PES_PER_GPC);
	u32 tpc_count = nvgpu_netlist_get_tpc_ctxsw_regs_count(g);
	u32 gpc_count = nvgpu_netlist_get_gpc_ctxsw_regs_count(g);

	if (addr_type == CTXSW_ADDR_TYPE_TPC) {
		/*
		 * reg = nvgpu_netlist_get_tpc_ctxsw_regs(g)->l;
		 * offset_in_segment = 0;
		 */
	} else if (addr_type == CTXSW_ADDR_TYPE_PPC) {
		/*
		 * The ucode stores TPC data before PPC data.
		 * Advance offset past TPC data to PPC data.
		 */
		offset_in_segment = ((tpc_count * num_tpcs) << 2);
	} else if (addr_type == CTXSW_ADDR_TYPE_GPC) {
		/*
		 * The ucode stores TPC/PPC data before GPC data.
		 * Advance offset past TPC/PPC data to GPC data.
		 *
		 * Note 1 PES_PER_GPC case
		 */
		if (num_pes_per_gpc > 1U) {
			offset_in_segment = (((tpc_count * num_tpcs) << 2) +
				((reg_list_ppc_count * num_ppcs) << 2));
		} else {
			offset_in_segment = ((tpc_count * num_tpcs) << 2);
		}
	} else if ((addr_type == CTXSW_ADDR_TYPE_EGPC) ||
			(addr_type == CTXSW_ADDR_TYPE_ETPC)) {
		if (num_pes_per_gpc > 1U) {
			offset_in_segment =
				((tpc_count * num_tpcs) << 2) +
				((reg_list_ppc_count * num_ppcs) << 2) +
							(gpc_count << 2);
		} else {
			offset_in_segment =
				((tpc_count * num_tpcs) << 2) +
							(gpc_count << 2);
		}

		/* aligned to next 256 byte */
		offset_in_segment = NVGPU_ALIGN(offset_in_segment, 256U);

		nvgpu_log(g, gpu_dbg_info | gpu_dbg_gpu_dbg,
			"egpc etpc offset_in_segment 0x%#08x",
			offset_in_segment);
	} else {
		nvgpu_log_fn(g, "Unknown address type.");
		return -EINVAL;
	}

	*__offset_in_segment = offset_in_segment;
	return 0;
}

void gr_tu104_init_sm_dsm_reg_info(void)
{
	return;
}

void gr_tu104_get_sm_dsm_perf_ctrl_regs(struct gk20a *g,
				        u32 *num_sm_dsm_perf_ctrl_regs,
				        u32 **sm_dsm_perf_ctrl_regs,
				        u32 *ctrl_register_stride)
{
	(void)g;
	*num_sm_dsm_perf_ctrl_regs = 0;
	*sm_dsm_perf_ctrl_regs = NULL;
	*ctrl_register_stride = 0;
}

int tu104_gr_update_smpc_global_mode(struct gk20a *g, bool enable)
{
	int err;

	if (enable) {
		err = g->ops.gr.falcon.ctrl_ctxsw(g,
			NVGPU_GR_FALCON_METHOD_START_SMPC_GLOBAL_MODE,
			0U, NULL);
	} else {
		err = g->ops.gr.falcon.ctrl_ctxsw(g,
			NVGPU_GR_FALCON_METHOD_STOP_SMPC_GLOBAL_MODE,
			0U, NULL);
	}

	return err;
}

void tu104_gr_disable_cau(struct gk20a *g)
{
	u32 i;

	for (i = 0U; i < gr_gpcs_tpcs_cau_control__size_1_v(); ++i) {
		nvgpu_writel(g, gr_gpcs_tpcs_cau_control_r(i), 0U);
	}

	if (g->ops.priv_ring.read_pri_fence != NULL) {
		g->ops.priv_ring.read_pri_fence(g);
	}
}

void tu104_gr_disable_smpc(struct gk20a *g)
{
	nvgpu_writel(g, gr_egpcs_etpcs_sm_dsm_perf_counter_control_r(), 0U);
	nvgpu_writel(g, gr_egpcs_etpcs_sm_dsm_perf_counter_control0_r(), 0U);
	nvgpu_writel(g, gr_egpcs_etpcs_sm_dsm_perf_counter_control5_r(), 0U);

	if (g->ops.priv_ring.read_pri_fence != NULL) {
		g->ops.priv_ring.read_pri_fence(g);
	}
}

static const u32 hwpm_cau_init_data[] =
{
	/* This list is autogenerated. Do not edit. */
	0x00419980,
	0x00000000,
	0x00419988,
	0x00000000,
	0x0041998c,
	0x00000000,
	0x00419990,
	0x00000000,
	0x00419994,
	0x00000000,
	0x00419998,
	0x00000000,
	0x0041999c,
	0x00000000,
	0x004199a4,
	0x00000001,
};

const u32 *tu104_gr_get_hwpm_cau_init_data(u32 *count)
{
	*count = sizeof(hwpm_cau_init_data) / sizeof(hwpm_cau_init_data[0]);
	return hwpm_cau_init_data;
}

void tu104_gr_init_cau(struct gk20a *g)
{
	const u32 *data;
	u32 cau_stride;
	u32 num_cau;
	u32 count;
	u32 i, j;

	num_cau = gr_gpcs_tpcs_cau_control__size_1_v();
	cau_stride = g->ops.regops.get_cau_register_stride();

	data = g->ops.gr.get_hwpm_cau_init_data(&count);

	for (i = 0U; i < num_cau; i++) {
		for (j = 0U; j < count; j += 2U) {
			nvgpu_writel(g, data[j] + i * cau_stride, data[j + 1U]);
		}
	}

	if (g->ops.priv_ring.read_pri_fence != NULL) {
		g->ops.priv_ring.read_pri_fence(g);
	}
}

/*
 * This function will decode a priv address and return the partition
 * type and numbers
 */
int gr_tu104_decode_priv_addr(struct gk20a *g, u32 addr,
	enum ctxsw_addr_type *addr_type,
	u32 *gpc_num, u32 *tpc_num, u32 *ppc_num, u32 *be_num,
	u32 *broadcast_flags)
{
	/*
	 * Special handling for LTS_TSTG registers.
	 *
	 * Unlike the other ltc registers which are stored as part of
	 * pm_ctxsw buffer these are stored in fecs ctxsw image priv
	 * segment regionid: NETLIST_REGIONID_CTXREG_SYS.
	 */
	if (g->ops.ltc.pri_is_ltc_addr(g, addr) &&
			g->ops.ltc.pri_is_lts_tstg_addr(g, addr)) {
		*addr_type = CTXSW_ADDR_TYPE_SYS;
		return 0;
	}

	return gr_gv11b_decode_priv_addr(g, addr, addr_type, gpc_num,
			tpc_num, ppc_num, be_num, broadcast_flags);
}
