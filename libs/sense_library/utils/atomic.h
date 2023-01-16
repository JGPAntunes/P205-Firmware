/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  atomic.h
 * @brief Implements a simple (platform-specific) mechanism for handling
 * critical code sections, through the use of atomic code blocks.
 */

#ifndef ATOMIC_H
#define ATOMIC_H

/********************************** Includes ***********************************/

/* C standard library */
#include <stdint.h>

/* Libs */
#include "debug.h"
#include "externs.h"

#if(!ATOMIC_DISABLE_INT && !TESTING_REPOSITORY)
#include "app_util_platform.h"
#endif

/********************************** Definitions ********************************/

#ifndef TESTING_REPOSITORY
#if(ATOMIC_DISABLE_INT)
#define __ASM            __asm					///< ASM keyword for GNU Compile.
#define __INLINE         inline					///< Inline keyword for GNU Compiler.
#define __STATIC_INLINE  static inline

/**
 * @brief Get priority mask.
 * @detail This function returns the current state of the priority mask bit from the Priority Mask Register.
 * @return Priority mask value.
 */
__attribute__( ( always_inline ))   __STATIC_INLINE uint32_t __get_PRIMASK(void) {
	uint32_t result;

	__ASM volatile ("MRS %0, primask" : "=r" (result) );
	return (result);
}

/** 
 * @brief  Disable IRQ interrupts.
 * @detail This function disables IRQ interrupts by setting the I-bit in the CPSR.
 * Can only be executed in Privileged modes.
 */
__attribute__( ( always_inline ) ) __STATIC_INLINE void __disable_irq(void) {
	__ASM volatile ("cpsid i" : : : "memory");
}

/** 
 * @brief Enable IRQ interrupts.
 * @detail This function enables IRQ interrupts by clearing the I-bit in the CPSR.
 * Can only be executed in Privileged modes.
 */
__attribute__( ( always_inline ) ) __STATIC_INLINE void __enable_irq(void) {
	__ASM volatile ("cpsie i" : : : "memory");
}

/**
 * Implements a function-like macro that executes code atomically.
 * For this, we start by reading the value of the PRIMASK register,
 * which stands for "Priority Mask Register". The PRIMASK is a
 * 32-bit register holding a single bit. If that bit is set, the
 * CPU prevents the activation of all exceptions with configurable
 * priority and if that bit is cleared, it has no effect. As such,
 * in order to support nesting of atomic code blocks, we only
 * re-enable interrupts if they were previously enabled, otherwise
 * unexpected behavior may happen.
 */
#define atomic(code) { \
  uint32_t primask = __get_PRIMASK(); \
  __disable_irq(); \
  code \
  ; \
  if (!primask) __enable_irq(); \
}
#else
/**
 * Implements a function-like macro that executes code atomically.
 * For this, we start by reading the value of the PRIMASK register,
 * which stands for "Priority Mask Register". The PRIMASK is a
 * 32-bit register holding a single bit. If that bit is set, the
 * CPU prevents the activation of all exceptions with configurable
 * priority and if that bit is cleared, it has no effect. As such,
 * in order to support nesting of atomic code blocks, we only
 * re-enable interrupts if they were previously enabled, otherwise
 * unexpected behavior may happen.
 */
#define atomic(code) { \
  CRITICAL_REGION_ENTER(); \
	code \
	; \
  CRITICAL_REGION_EXIT(); \
}
#endif
#endif

#ifdef TESTING_REPOSITORY
#define atomic(code) { \
	code \
	; \
}
#endif

#endif /* ATOMIC_H */
