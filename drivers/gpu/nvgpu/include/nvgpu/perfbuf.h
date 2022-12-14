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

#ifndef NVGPU_PERFBUF
#define NVGPU_PERFBUF

#ifdef CONFIG_NVGPU_DEBUGGER

/*
 * Size of the GPU VA window within which the PMA unit is allowed to
 * access.
 */
#define PERFBUF_PMA_MEM_WINDOW_SIZE	SZ_4G
#define PERFBUF_PMA_BUF_MAX_SIZE	0xFFE00000ULL
#define PMA_BYTES_AVAILABLE_BUFFER_SIZE	SZ_4K

#include <nvgpu/types.h>

struct gk20a;

int nvgpu_perfbuf_enable_locked(struct gk20a *g, u64 offset, u32 size);
int nvgpu_perfbuf_disable_locked(struct gk20a *g);

int nvgpu_perfbuf_init_vm(struct gk20a *g);
void nvgpu_perfbuf_deinit_vm(struct gk20a *g);

int nvgpu_perfbuf_init_inst_block(struct gk20a *g);
void nvgpu_perfbuf_deinit_inst_block(struct gk20a *g);

int nvgpu_perfbuf_update_get_put(struct gk20a *g, u64 bytes_consumed, u64 *bytes_available,
		void *cpuva, bool wait, u64 *put_ptr, bool *overflowed);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif
