/*
 * Copyright (c) 2018-2019, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvgpu/timers.h>
#include <nvgpu/soc.h>
#include <os/posix/os_posix.h>

bool nvgpu_platform_is_silicon(struct gk20a *g)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	return p->is_silicon;
}

bool nvgpu_platform_is_simulation(struct gk20a *g)
{
	return false;
}

bool nvgpu_platform_is_fpga(struct gk20a *g)
{
	return false;
}

bool nvgpu_is_hypervisor_mode(struct gk20a *g)
{
	return false;
}

bool nvgpu_is_bpmp_running(struct gk20a *g)
{
	return false;
}

bool nvgpu_is_soc_t194_a01(struct gk20a *g)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	return p->is_soc_t194_a01;
}

void nvgpu_delay_usecs(unsigned int usecs)
{
}

#ifdef CONFIG_NVGPU_NON_FUSA
u64 nvgpu_us_counter(void)
{
	return (u64)nvgpu_current_time_us();
}
#endif

u64 nvgpu_get_cycles(void)
{
	return (u64)nvgpu_current_time_us();
}

int nvgpu_init_soc_vars(struct gk20a *g)
{
	return 0;
}
