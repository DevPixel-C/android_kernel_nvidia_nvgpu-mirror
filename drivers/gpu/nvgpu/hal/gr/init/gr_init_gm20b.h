/*
 * Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef NVGPU_GR_INIT_GM20B_H
#define NVGPU_GR_INIT_GM20B_H

#include <nvgpu/types.h>

#ifndef GR_GO_IDLE_BUNDLE
#define GR_GO_IDLE_BUNDLE	0x0000e100U /* --V-B */
#endif

struct gk20a;
struct nvgpu_gr_ctx;
struct nvgpu_gr_config;
struct netlist_av_list;
struct nvgpu_gr_config;

u32 gm20b_gr_init_get_fbp_en_mask(struct gk20a *g);
void gm20b_gr_init_lg_coalesce(struct gk20a *g, u32 data);
void gm20b_gr_init_su_coalesce(struct gk20a *g, u32 data);
void gm20b_gr_init_pes_vsc_stream(struct gk20a *g);
void gm20b_gr_init_gpc_mmu(struct gk20a *g);
void gm20b_gr_init_fifo_access(struct gk20a *g, bool enable);
void gm20b_gr_init_get_access_map(struct gk20a *g,
				   u32 **whitelist, int *num_entries);
void gm20b_gr_init_sm_id_numbering(struct gk20a *g,
					u32 gpc, u32 tpc, u32 smid);
u32 gm20b_gr_init_get_sm_id_size(void);
int gm20b_gr_init_sm_id_config(struct gk20a *g, u32 *tpc_sm_id,
			       struct nvgpu_gr_config *gr_config);
void gm20b_gr_init_tpc_mask(struct gk20a *g, u32 gpc_index, u32 pes_tpc_mask);
int gm20b_gr_init_rop_mapping(struct gk20a *g,
			      struct nvgpu_gr_config *gr_config);
int gm20b_gr_init_fs_state(struct gk20a *g);
void gm20b_gr_init_pd_tpc_per_gpc(struct gk20a *g,
			      struct nvgpu_gr_config *gr_config);
void gm20b_gr_init_pd_skip_table_gpc(struct gk20a *g,
			      struct nvgpu_gr_config *gr_config);
void gm20b_gr_init_cwd_gpcs_tpcs_num(struct gk20a *g,
				     u32 gpc_count, u32 tpc_count);
int gm20b_gr_init_wait_idle(struct gk20a *g);
int gm20b_gr_init_wait_fe_idle(struct gk20a *g);
int gm20b_gr_init_fe_pwr_mode_force_on(struct gk20a *g, bool force_on);
void gm20b_gr_init_override_context_reset(struct gk20a *g);
void gm20b_gr_init_fe_go_idle_timeout(struct gk20a *g, bool enable);
void gm20b_gr_init_pipe_mode_override(struct gk20a *g, bool enable);
void gm20b_gr_init_load_method_init(struct gk20a *g,
		struct netlist_av_list *sw_method_init);
int gm20b_gr_init_load_sw_bundle_init(struct gk20a *g,
		struct netlist_av_list *sw_bundle_init);
void gm20b_gr_init_commit_global_timeslice(struct gk20a *g);

u32 gm20b_gr_init_get_bundle_cb_default_size(struct gk20a *g);
u32 gm20b_gr_init_get_min_gpm_fifo_depth(struct gk20a *g);
u32 gm20b_gr_init_get_bundle_cb_token_limit(struct gk20a *g);
u32 gm20b_gr_init_get_attrib_cb_default_size(struct gk20a *g);
u32 gm20b_gr_init_get_alpha_cb_default_size(struct gk20a *g);
u32 gm20b_gr_init_get_attrib_cb_size(struct gk20a *g, u32 tpc_count);
u32 gm20b_gr_init_get_alpha_cb_size(struct gk20a *g, u32 tpc_count);
u32 gm20b_gr_init_get_global_attr_cb_size(struct gk20a *g, u32 tpc_count,
	u32 max_tpc);
u32 gm20b_gr_init_get_global_ctx_cb_buffer_size(struct gk20a *g);
u32 gm20b_gr_init_get_global_ctx_pagepool_buffer_size(struct gk20a *g);

void gm20b_gr_init_commit_global_bundle_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, u64 addr, u64 size, bool patch);
u32 gm20b_gr_init_pagepool_default_size(struct gk20a *g);
void gm20b_gr_init_commit_global_pagepool(struct gk20a *g,
	struct nvgpu_gr_ctx *ch_ctx, u64 addr, u32 size, bool patch,
	bool global_ctx);
void gm20b_gr_init_commit_global_attrib_cb(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, u32 tpc_count, u32 max_tpc, u64 addr,
	bool patch);
void gm20b_gr_init_commit_global_cb_manager(struct gk20a *g,
	struct nvgpu_gr_config *config, struct nvgpu_gr_ctx *gr_ctx,
	bool patch);

#endif /* NVGPU_GR_INIT_GM20B_H */