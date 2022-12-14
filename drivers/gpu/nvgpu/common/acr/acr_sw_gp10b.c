/*
 * Copyright (c) 2019-2022, NVIDIA CORPORATION.  All rights reserved.
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

#include "acr_sw_gp10b.h"

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/pmu.h>

#include "acr_blob_construct_v0.h"
#include "acr_priv.h"

#include "acr_sw_gm20b.h"
#include "acr_sw_gp10b.h"

/* LSF static config functions */
static u32 gp10b_acr_lsf_gpccs(struct gk20a *g,
		struct acr_lsf_config *lsf)
{
	(void)g;
	/* GPCCS LS falcon info */
	lsf->falcon_id = FALCON_ID_GPCCS;
	lsf->falcon_dma_idx = GK20A_PMU_DMAIDX_UCODE;
	lsf->is_lazy_bootstrap = true;
	lsf->is_priv_load = true;
	lsf->get_lsf_ucode_details = nvgpu_acr_lsf_gpccs_ucode_details_v0;
	lsf->get_cmd_line_args_offset = NULL;

	return BIT32(lsf->falcon_id);
}

static void gp10b_acr_default_sw_init(struct gk20a *g, struct hs_acr *hs_acr)
{
	nvgpu_log_fn(g, " ");

	/* ACR HS ucode type & f/w name*/
	hs_acr->acr_type = ACR_DEFAULT;

	if (!g->ops.pmu.is_debug_mode_enabled(g)) {
		hs_acr->acr_fw_name = HSBIN_ACR_PROD_UCODE;
	} else {
		hs_acr->acr_fw_name = HSBIN_ACR_DBG_UCODE;
	}

	/* set on which falcon ACR need to execute*/
	hs_acr->acr_flcn = g->pmu->flcn;
	hs_acr->acr_engine_bus_err_status =
		g->ops.pmu.bar0_error_status;
}

void nvgpu_gp10b_acr_sw_init(struct gk20a *g, struct nvgpu_acr *acr)
{
	nvgpu_log_fn(g, " ");

	/* inherit the gm20b config data */
	nvgpu_gm20b_acr_sw_init(g, acr);
	gp10b_acr_default_sw_init(g, &acr->acr);

	/* gp10b supports LSF gpccs bootstrap */
	acr->lsf_enable_mask |= gp10b_acr_lsf_gpccs(g,
		&acr->lsf[FALCON_ID_GPCCS]);
}
