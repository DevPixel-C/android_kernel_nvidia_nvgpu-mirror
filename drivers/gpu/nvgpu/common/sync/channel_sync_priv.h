/*
 *
 * Nvgpu Channel Synchronization Abstraction
 *
 * Copyright (c) 2014-2018, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef NVGPU_CHANNEL_SYNC_PRIV_H
#define NVGPU_CHANNEL_SYNC_PRIV_H

#include <nvgpu/atomic.h>
#include <nvgpu/types.h>

struct priv_cmd_entry;
struct gk20a_fence;

/*
 * This struct is private and should not be used directly. Users should
 * instead use the public APIs starting with nvgpu_channel_sync_*
 */
struct nvgpu_channel_sync {
	nvgpu_atomic_t refcount;

	int (*wait_fence_raw)(struct nvgpu_channel_sync *s, u32 id, u32 thresh,
			   struct priv_cmd_entry *entry);

	int (*wait_fence_fd)(struct nvgpu_channel_sync *s, int fd,
		       struct priv_cmd_entry *entry, u32 max_wait_cmds);

	int (*incr)(struct nvgpu_channel_sync *s,
		    struct priv_cmd_entry *entry,
		    struct gk20a_fence *fence,
		    bool need_sync_fence,
		    bool register_irq);

	int (*incr_user)(struct nvgpu_channel_sync *s,
			 int wait_fence_fd,
			 struct priv_cmd_entry *entry,
			 struct gk20a_fence *fence,
			 bool wfi,
			 bool need_sync_fence,
			 bool register_irq);

	void (*set_min_eq_max)(struct nvgpu_channel_sync *s);

	void (*set_safe_state)(struct nvgpu_channel_sync *s);

	void (*destroy)(struct nvgpu_channel_sync *s);
};

#endif /* NVGPU_CHANNEL_SYNC_PRIV_H */