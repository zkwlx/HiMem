/**
 * Copyright 2004-present, Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

//#if defined(__arm__) || defined(__aarch64__)

// The ARM ELF TLS ABI specifies[1] that the thread pointer points at a 2-word
// TCB followed by the executable's TLS segment. Both the TCB and the
// executable's segment are aligned according to the segment, so Bionic requires
// a minimum segment alignment, which effectively reserves an 8-word TCB. The
// ARM spec allocates the first TCB word to the DTV.
//
// [1] "Addenda to, and Errata in, the ABI for the ARM Architecture". Section 3.
// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0045e/IHI0045E_ABI_addenda.pdf

#define MIN_TLS_SLOT            (-1) // update this value when reserving a slot
#define TLS_SLOT_BIONIC_TLS     (-1)
#define TLS_SLOT_DTV              0
#define TLS_SLOT_THREAD_ID        1
#define TLS_SLOT_APP              2 // was historically used for errno
#define TLS_SLOT_OPENGL           3
#define TLS_SLOT_OPENGL_API       4
#define TLS_SLOT_STACK_GUARD      5
#define TLS_SLOT_SANITIZER        6 // was historically used for dlerror
#define TLS_SLOT_ART_THREAD_SELF  7

#define ANDROID_VERSION_NUM 900

#if ANDROID_VERSION_NUM >= 700

#if defined(__aarch64__)
#define __get_tls()                             \
  ({                                            \
    void** __val;                               \
    __asm__("mrs %0, tpidr_el0" : "=r"(__val)); \
    __val;                                      \
  })
#elif defined(__arm__)
#define __get_tls()                                      \
  ({                                                     \
    void** __val;                                        \
    __asm__("mrc p15, 0, %0, c13, c0, 3" : "=r"(__val)); \
    __val;                                               \
  })
#elif defined(__mips__)
#define __get_tls()                                                      \
  /* On mips32r1, this goes via a kernel illegal instruction trap that's \
   * optimized for v1. */                                                \
  ({                                                                     \
    register void** __val asm("v1");                                     \
    __asm__(                                                             \
        ".set    push\n"                                                 \
        ".set    mips32r2\n"                                             \
        "rdhwr   %0,$29\n"                                               \
        ".set    pop\n"                                                  \
        : "=r"(__val));                                                  \
    __val;                                                               \
  })
#elif defined(__i386__)
#define __get_tls()                           \
  ({                                          \
    void** __val;                             \
    __asm__("movl %%gs:0, %0" : "=r"(__val)); \
    __val;                                    \
  })
#elif defined(__x86_64__)
#define __get_tls()                          \
  ({                                         \
    void** __val;                            \
    __asm__("mov %%fs:0, %0" : "=r"(__val)); \
    __val;                                   \
  })
#else
#error unsupported architecture
#endif

#endif
