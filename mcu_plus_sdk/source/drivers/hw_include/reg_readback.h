/*  ============================================================================
 *  Copyright (C) 2026 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/
/** ============================================================================
 *   \file  reg_readback.h
 *
 *   \brief  Register readback-verify support.
 *
 *           Readback re-reads a register right after it has been written and
 *           compares the value against what was written. A mismatch invokes a
 *           caller-supplied error handler. It is a diagnostic measure against
 *           latent faults in the register interface.
 *
 *           This is the single home for every readback macro and helper. It
 *           is included by both cslr.h (the CSL register layer, pointer-based
 *           access, CSL_-prefixed field macros) and hw_types.h (the HW access
 *           layer, address-based access, unprefixed field macros) so that the
 *           macros exist exactly once regardless of which layer a driver uses.
 *
 *           Configuration is two-level:
 *
 *           1. REG_READBACK_ENABLED - master build switch. When it is not
 *              defined, every RB_CHECK_* / HW_RB_CHECK_* macro compiles to
 *              nothing and the generated code is identical to a build with no
 *              readback support at all.
 *
 *           2. REG_READBACK_MODULE_MASK - selects which drivers actually
 *              perform readback, out of those that have been instrumented.
 *              Each instrumented .c declares its own identity by defining
 *              REG_READBACK_MODULE before including cslr.h / hw_types.h:
 *
 *                  #define REG_READBACK_MODULE  REG_RB_MOD_DCC
 *                  #include <drivers/hw_include/cslr.h>
 *
 *              The mask is a compile-time constant, so the check folds away
 *              and de-selected drivers cost nothing in code size or cycles.
 *
 *           Example - readback in DCC and DSS only, DCC/LPDDR/ADC
 *           instrumented but switched off:
 *
 *              -DREG_READBACK_ENABLED
 *              -DREG_READBACK_MODULE_MASK="(REG_RB_MOD_DCC | REG_RB_MOD_DSS)"
 *
 */

#ifndef REG_READBACK_H_
#define REG_READBACK_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <drivers/hw_include/tistdtypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  \anchor REG_ReadbackModuleId
 *  \name   Readback module identifiers
 *
 *  One bit per driver that carries readback instrumentation. Add a bit here only once
 *  a driver is actually instrumented; the value must stay stable, because integrators
 *  reference these names in their build configuration.
 *
 *  @{
 */
#define REG_RB_MOD_NONE          (0x0000000000000000ULL)

/* Drivers writing registers through the CSL register layer (cslr.h). */
#define REG_RB_MOD_LPDDR         (0x0000000000000001ULL)
#define REG_RB_MOD_DCC           (0x0000000000000002ULL)
#define REG_RB_MOD_DSS           (0x0000000000000004ULL)

/* Drivers writing registers through the HW access layer (hw_types.h). */
#define REG_RB_MOD_ADC           (0x0000000000000010ULL)

#define REG_RB_MOD_ALL           (0xFFFFFFFFFFFFFFFFULL)
/** @} */

/**
 *  \brief Drivers that perform readback. Override at build time, e.g.
 *         -DREG_READBACK_MODULE_MASK="(REG_RB_MOD_DCC | REG_RB_MOD_DSS)"
 *
 *         Defaults to none, so enabling REG_READBACK_ENABLED on its own does
 *         not silently add readback to every instrumented driver.
 */
#ifndef REG_READBACK_MODULE_MASK
#define REG_READBACK_MODULE_MASK (REG_RB_MOD_NONE)
#endif

/**
 *  \brief Identity of the translation unit being compiled. A source file that
 *         does not declare itself performs no readback.
 */
#ifndef REG_READBACK_MODULE
#define REG_READBACK_MODULE      (REG_RB_MOD_NONE)
#endif

/**
 *  \brief Mask selecting every bit of a 32-bit register.
 */
#ifndef RB_MASK_ALL_BITS_32
#define RB_MASK_ALL_BITS_32     (0xFFFFFFFFU)
#endif

/**
 *  \brief Mask selecting every bit of a 16-bit register.
 */
#ifndef RB_MASK_ALL_BITS_16
#define RB_MASK_ALL_BITS_16     (0xFFFFU)
#endif

/**
 *  \brief Evaluates to TRUE when module \c m is selected for readback.
 *
 *         Both operands are compile-time constants, so this normally folds
 *         away entirely and no code is emitted for de-selected modules.
 */
#define REG_RB_ON(m)            ((((REG_READBACK_MODULE_MASK) & (m)) != REG_RB_MOD_NONE) ? \
                                    ((uint32_t) 1U) : ((uint32_t) 0U))

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief   Re-reads a 32-bit register and compares the masked value against
 *           the value that was written. Calls \c errRbFunc on mismatch.
 *
 *  \param   p           Address of the memory mapped hardware register.
 *  \param   refVal      Value expected to be in the register.
 *  \param   mask        Selects which bits take part in the comparison.
 *  \param   errRbFunc   Handler called as (expected, actual) on mismatch. May
 *                       be NULL_PTR, in which case a mismatch is ignored.
 */
static inline void REG_rbVerifyReg32(volatile const uint32_t *p,
                                     uint32_t refVal,
                                     uint32_t mask,
                                     void (*errRbFunc)(uint32_t, uint32_t));
static inline void REG_rbVerifyReg32(volatile const uint32_t *p,
                                     uint32_t refVal,
                                     uint32_t mask,
                                     void (*errRbFunc)(uint32_t, uint32_t))
{
    uint32_t readVal = ((*p) & mask);
    uint32_t expVal  = (refVal & mask);

    if (readVal != expVal)
    {
        if (errRbFunc != NULL_PTR)
        {
            errRbFunc(expVal, readVal);
        }
    }

    return;
}

/**
 *  \brief   Re-reads a bit field of a 32-bit register and compares it against
 *           the value that was written. Calls \c errRbFunc on mismatch.
 *
 *  \param   p           Address of the memory mapped hardware register.
 *  \param   refVal      Expected field value, right-aligned.
 *  \param   mask        Bit field mask, in register position.
 *  \param   shift       Bit field shift from LSB.
 *  \param   errRbFunc   Handler called as (expected, actual) on mismatch. May
 *                       be NULL_PTR, in which case a mismatch is ignored.
 */
static inline void REG_rbVerifyField32(volatile const uint32_t *p,
                                       uint32_t refVal,
                                       uint32_t mask,
                                       uint32_t shift,
                                       void (*errRbFunc)(uint32_t, uint32_t));
static inline void REG_rbVerifyField32(volatile const uint32_t *p,
                                       uint32_t refVal,
                                       uint32_t mask,
                                       uint32_t shift,
                                       void (*errRbFunc)(uint32_t, uint32_t))
{
    uint32_t readField = (((*p) & mask) >> shift);

    if (readField != refVal)
    {
        if (errRbFunc != NULL_PTR)
        {
            errRbFunc(refVal, readField);
        }
    }

    return;
}

/**
 *  \brief   Re-reads a 16-bit register and compares the masked value against
 *           the value that was written. Calls \c errRbFunc on mismatch.
 *
 *  \param   p           Address of the memory mapped hardware register.
 *  \param   refVal      Value expected to be in the register.
 *  \param   mask        Selects which bits take part in the comparison.
 *  \param   errRbFunc   Handler called as (expected, actual) on mismatch. May
 *                       be NULL_PTR, in which case a mismatch is ignored.
 */
static inline void REG_rbVerifyReg16(volatile const uint16_t *p,
                                     uint16_t refVal,
                                     uint16_t mask,
                                     void (*errRbFunc)(uint32_t, uint32_t));
static inline void REG_rbVerifyReg16(volatile const uint16_t *p,
                                     uint16_t refVal,
                                     uint16_t mask,
                                     void (*errRbFunc)(uint32_t, uint32_t))
{
    uint16_t readVal = (uint16_t) ((*p) & mask);
    uint16_t expVal  = (uint16_t) (refVal & mask);

    if (readVal != expVal)
    {
        if (errRbFunc != NULL_PTR)
        {
            errRbFunc((uint32_t) expVal, (uint32_t) readVal);
        }
    }

    return;
}

/**
 *  \brief   Re-reads a bit field of a 16-bit register and compares it against
 *           the value that was written. Calls \c errRbFunc on mismatch.
 *
 *  \param   p           Address of the memory mapped hardware register.
 *  \param   refVal      Expected field value, right-aligned.
 *  \param   mask        Bit field mask, in register position.
 *  \param   shift       Bit field shift from LSB.
 *  \param   errRbFunc   Handler called as (expected, actual) on mismatch. May
 *                       be NULL_PTR, in which case a mismatch is ignored.
 */
static inline void REG_rbVerifyField16(volatile const uint16_t *p,
                                       uint16_t refVal,
                                       uint16_t mask,
                                       uint32_t shift,
                                       void (*errRbFunc)(uint32_t, uint32_t));
static inline void REG_rbVerifyField16(volatile const uint16_t *p,
                                       uint16_t refVal,
                                       uint16_t mask,
                                       uint32_t shift,
                                       void (*errRbFunc)(uint32_t, uint32_t))
{
    uint16_t readField = (uint16_t) (((*p) & mask) >> shift);

    if (readField != refVal)
    {
        if (errRbFunc != NULL_PTR)
        {
            errRbFunc((uint32_t) refVal, (uint32_t) readField);
        }
    }

    return;
}

/* ========================================================================== */
/*                    Readback-Verify Macros - CSL layer (cslr.h)             */
/*                                                                            */
/*  Pointer-based access, register field masks/shifts taken from the         */
/*  CSL_-prefixed field macros generated for each register (CSL_##fld##_MASK)*/
/* ========================================================================== */

/**
 *  \brief This macro re-reads a 32 bit register after it has been written and
 *         verifies the masked value against the value that was written. On
 *         mismatch errRbFunc is called with (expected, actual).
 *
 *         Expands to nothing unless REG_READBACK_ENABLED is defined and the
 *         module owning this translation unit is selected in
 *         REG_READBACK_MODULE_MASK.
 *
 *  \param   p            Address of the memory mapped hardware register.
 *  \param   v            Value expected to be in the register.
 *  \param   mask         Selects which bits take part in the comparison.
 *  \param   errRbFunc    Handler called on failed verification, or NULL_PTR.
 */
#ifdef REG_READBACK_ENABLED
#define RB_CHECK_REG32(p, v, mask, errRbFunc)                                  \
    do {                                                                       \
        if (REG_RB_ON(REG_READBACK_MODULE) != 0U)                              \
        {                                                                      \
            REG_rbVerifyReg32(((volatile const uint32_t *) (p)),               \
                              ((uint32_t) (v)),                                \
                              ((uint32_t) (mask)),                             \
                              ((void (*)(uint32_t, uint32_t)) (errRbFunc)));   \
        }                                                                      \
    } while (0)

/**
 *  \brief This macro re-reads a bit field of a 32 bit register after it has
 *         been written and verifies it against a literal expected value. The
 *         mask and shift are framed from the register field macro.
 *
 *  \param   reg              Register holding the field, as an lvalue.
 *  \param   PER_REG_FIELD    Register field name, used to derive MASK/SHIFT.
 *  \param   val              Expected value of the field.
 *  \param   errRbFunc        Handler called on failed verification, or NULL_PTR.
 */
#define RB_CHECK_FIELD32_VAL(reg, PER_REG_FIELD, val, errRbFunc)               \
    do {                                                                       \
        if (REG_RB_ON(REG_READBACK_MODULE) != 0U)                              \
        {                                                                      \
            REG_rbVerifyField32(((volatile const uint32_t *) &(reg)),          \
                                ((uint32_t) (val)),                            \
                                ((uint32_t) CSL_##PER_REG_FIELD##_MASK),        \
                                ((uint32_t) CSL_##PER_REG_FIELD##_SHIFT),       \
                                ((void (*)(uint32_t, uint32_t)) (errRbFunc))); \
        }                                                                      \
    } while (0)

/**
 *  \brief This macro re-reads a bit field of a 32 bit register after it has
 *         been written and verifies it against a symbolic field token.
 *
 *  \param   reg              Register holding the field, as an lvalue.
 *  \param   PER_REG_FIELD    Register field name, used to derive MASK/SHIFT.
 *  \param   TOKEN            Symbolic token of the expected field value.
 *  \param   errRbFunc        Handler called on failed verification, or NULL_PTR.
 */
#define RB_CHECK_FIELD32_T(reg, PER_REG_FIELD, TOKEN, errRbFunc)               \
    do {                                                                       \
        if (REG_RB_ON(REG_READBACK_MODULE) != 0U)                              \
        {                                                                      \
            REG_rbVerifyField32(((volatile const uint32_t *) &(reg)),          \
                                ((uint32_t) CSL_##PER_REG_FIELD##_##TOKEN),     \
                                ((uint32_t) CSL_##PER_REG_FIELD##_MASK),        \
                                ((uint32_t) CSL_##PER_REG_FIELD##_SHIFT),       \
                                ((void (*)(uint32_t, uint32_t)) (errRbFunc))); \
        }                                                                      \
    } while (0)

/**
 *  \brief 16 bit register variant of \ref RB_CHECK_REG32.
 */
#define RB_CHECK_REG16(p, v, mask, errRbFunc)                                  \
    do {                                                                       \
        if (REG_RB_ON(REG_READBACK_MODULE) != 0U)                              \
        {                                                                      \
            REG_rbVerifyReg16(((volatile const uint16_t *) (p)),               \
                              ((uint16_t) (v)),                                \
                              ((uint16_t) (mask)),                             \
                              ((void (*)(uint32_t, uint32_t)) (errRbFunc)));   \
        }                                                                      \
    } while (0)

/**
 *  \brief 16 bit register variant of \ref RB_CHECK_FIELD32_VAL.
 */
#define RB_CHECK_FIELD16_VAL(reg, PER_REG_FIELD, val, errRbFunc)               \
    do {                                                                       \
        if (REG_RB_ON(REG_READBACK_MODULE) != 0U)                              \
        {                                                                      \
            REG_rbVerifyField16(((volatile const uint16_t *) &(reg)),          \
                                ((uint16_t) (val)),                            \
                                ((uint16_t) CSL_##PER_REG_FIELD##_MASK),        \
                                ((uint32_t) CSL_##PER_REG_FIELD##_SHIFT),       \
                                ((void (*)(uint32_t, uint32_t)) (errRbFunc))); \
        }                                                                      \
    } while (0)

#else   /* REG_READBACK_ENABLED */

#define RB_CHECK_REG32(p, v, mask, errRbFunc)                                  \
    do { (void) (p); (void) (v); (void) (mask); (void) (errRbFunc); } while (0)

#define RB_CHECK_FIELD32_VAL(reg, PER_REG_FIELD, val, errRbFunc)               \
    do { (void) (reg); (void) (val); (void) (errRbFunc); } while (0)

#define RB_CHECK_FIELD32_T(reg, PER_REG_FIELD, TOKEN, errRbFunc)               \
    do { (void) (reg); (void) (errRbFunc); } while (0)

#define RB_CHECK_REG16(p, v, mask, errRbFunc)                                  \
    do { (void) (p); (void) (v); (void) (mask); (void) (errRbFunc); } while (0)

#define RB_CHECK_FIELD16_VAL(reg, PER_REG_FIELD, val, errRbFunc)               \
    do { (void) (reg); (void) (val); (void) (errRbFunc); } while (0)

#endif  /* REG_READBACK_ENABLED */

/* ========================================================================== */
/*                Readback-Verify Macros - HW layer (hw_types.h)              */
/*                                                                            */
/*  Address-based access, register field masks/shifts taken from the         */
/*  unprefixed field macros used by HW_RD_FIELD32/HW_WR_FIELD32               */
/*  (REG_FIELD##_MASK, with no CSL_ prefix).                                  */
/* ========================================================================== */

/**
 *  \brief This macro re-reads a 32 bit register after it has been written and
 *         verifies the masked value against the value that was written. On
 *         mismatch errRbFunc is called with (expected, actual).
 *
 *         Expands to nothing unless REG_READBACK_ENABLED is defined and the
 *         module owning this translation unit is selected in
 *         REG_READBACK_MODULE_MASK.
 *
 *  \param regAddr        Register Address.
 *  \param refVal         Value expected to be in the register.
 *  \param mask           Selects which bits take part in the comparison.
 *  \param errRbFunc      Handler called on failed verification, or NULL_PTR.
 */
#ifdef REG_READBACK_ENABLED
#define HW_RB_CHECK_REG32_MASK(regAddr, refVal, mask, errRbFunc)               \
    do {                                                                       \
        if (REG_RB_ON(REG_READBACK_MODULE) != 0U)                              \
        {                                                                      \
            REG_rbVerifyReg32(((volatile const uint32_t *) (uintptr_t) (regAddr)),\
                              ((uint32_t) (refVal)),                           \
                              ((uint32_t) (mask)),                             \
                              ((void (*)(uint32_t, uint32_t)) (errRbFunc)));   \
        }                                                                      \
    } while (0)

/**
 *  \brief Whole-register variant of \ref HW_RB_CHECK_REG32_MASK, comparing all
 *         32 bits.
 *
 *  \param regAddr        Register Address.
 *  \param refVal         Value expected to be in the register.
 *  \param errRbFunc      Handler called on failed verification, or NULL_PTR.
 */
#define HW_RB_CHECK_REG32(regAddr, refVal, errRbFunc)                          \
    HW_RB_CHECK_REG32_MASK((regAddr), (refVal), RB_MASK_ALL_BITS_32, (errRbFunc))

/**
 *  \brief This macro re-reads a bit field of a 32 bit register after it has
 *         been written and verifies it against a literal expected value. The
 *         mask and shift are framed from the register field macro.
 *
 *  \param regAddr        Register Address.
 *  \param REG_FIELD      Register field name, used to derive MASK/SHIFT.
 *  \param val            Expected value of the field.
 *  \param errRbFunc      Handler called on failed verification, or NULL_PTR.
 */
#define HW_RB_CHECK_FIELD32_VAL(regAddr, REG_FIELD, val, errRbFunc)            \
    do {                                                                       \
        if (REG_RB_ON(REG_READBACK_MODULE) != 0U)                              \
        {                                                                      \
            REG_rbVerifyField32(((volatile const uint32_t *) (uintptr_t) (regAddr)),\
                                ((uint32_t) (val)),                            \
                                ((uint32_t) REG_FIELD##_MASK),                  \
                                ((uint32_t) REG_FIELD##_SHIFT),                 \
                                ((void (*)(uint32_t, uint32_t)) (errRbFunc))); \
        }                                                                      \
    } while (0)

/**
 *  \brief 16 bit register variant of \ref HW_RB_CHECK_REG32_MASK.
 */
#define HW_RB_CHECK_REG16_MASK(regAddr, refVal, mask, errRbFunc)               \
    do {                                                                       \
        if (REG_RB_ON(REG_READBACK_MODULE) != 0U)                              \
        {                                                                      \
            REG_rbVerifyReg16(((volatile const uint16_t *) (uintptr_t) (regAddr)),\
                              ((uint16_t) (refVal)),                           \
                              ((uint16_t) (mask)),                             \
                              ((void (*)(uint32_t, uint32_t)) (errRbFunc)));   \
        }                                                                      \
    } while (0)

/**
 *  \brief 16 bit whole-register variant of \ref HW_RB_CHECK_REG32.
 */
#define HW_RB_CHECK_REG16(regAddr, refVal, errRbFunc)                          \
    HW_RB_CHECK_REG16_MASK((regAddr), (refVal), RB_MASK_ALL_BITS_16, (errRbFunc))

/**
 *  \brief 16 bit register variant of \ref HW_RB_CHECK_FIELD32_VAL.
 */
#define HW_RB_CHECK_FIELD16_VAL(regAddr, REG_FIELD, val, errRbFunc)            \
    do {                                                                       \
        if (REG_RB_ON(REG_READBACK_MODULE) != 0U)                              \
        {                                                                      \
            REG_rbVerifyField16(((volatile const uint16_t *) (uintptr_t) (regAddr)),\
                                ((uint16_t) (val)),                            \
                                ((uint16_t) REG_FIELD##_MASK),                  \
                                ((uint32_t) REG_FIELD##_SHIFT),                 \
                                ((void (*)(uint32_t, uint32_t)) (errRbFunc))); \
        }                                                                      \
    } while (0)

#else   /* REG_READBACK_ENABLED */

#define HW_RB_CHECK_REG32_MASK(regAddr, refVal, mask, errRbFunc)               \
    do { (void) (regAddr); (void) (refVal); (void) (mask); (void) (errRbFunc); } while (0)

#define HW_RB_CHECK_REG32(regAddr, refVal, errRbFunc)                          \
    do { (void) (regAddr); (void) (refVal); (void) (errRbFunc); } while (0)

#define HW_RB_CHECK_FIELD32_VAL(regAddr, REG_FIELD, val, errRbFunc)            \
    do { (void) (regAddr); (void) (val); (void) (errRbFunc); } while (0)

#define HW_RB_CHECK_REG16_MASK(regAddr, refVal, mask, errRbFunc)               \
    do { (void) (regAddr); (void) (refVal); (void) (mask); (void) (errRbFunc); } while (0)

#define HW_RB_CHECK_REG16(regAddr, refVal, errRbFunc)                          \
    do { (void) (regAddr); (void) (refVal); (void) (errRbFunc); } while (0)

#define HW_RB_CHECK_FIELD16_VAL(regAddr, REG_FIELD, val, errRbFunc)            \
    do { (void) (regAddr); (void) (val); (void) (errRbFunc); } while (0)

#endif  /* REG_READBACK_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef REG_READBACK_H_ */