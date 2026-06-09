/*
 * System Firmware
 *
 * Cortex-M3 (CM3) firmware for power management
 *
 * Copyright (C) 2015-2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DEVICE_CLK_H
#define DEVICE_CLK_H

#include <types/short_types.h>
#include <build_assert.h>
#include <types/sbool.h>
#include <pm_types.h>
#include <config.h>

#define DEV_CLK_ID_NONE ((dev_clk_idx_t) (~0U))

/**
 * \brief Input clock type.
 *
 * This is a normal clock, it's an input to the IP from elsewhere.
 */
#define DEV_CLK_TABLE_TYPE_INPUT        0U

/**
 * \brief Mux clock type.
 *
 * This is a clock mux that is meant to be controlled directly. The potential
 * parents of the mux are listed after the mux as TYPE_PARENT. Set frequency
 * calls to a mux are passed to the current parent.
 */
#define DEV_CLK_TABLE_TYPE_MUX          1U

/**
 * \brief Mux parent clock type.
 *
 * This is a potential parent for a mux. Normal clocks are set to follow the
 * device state (requested when device is enabled, unrequested otherwise) but
 * mux parents are set to always unrequested by default. If they are selected
 * they will get a ref count from the mux clock.
 *
 * Mux parents are present so they can be enabled and configured before being
 * selected via the mux.
 */
#define DEV_CLK_TABLE_TYPE_PARENT       2U

/**
 * \brief Output clock type.
 *
 * This is an output clock from the device. Setting the frequency informs
 * potential consumers of the frequency but otherwise has no effect.
 */
#define DEV_CLK_TABLE_TYPE_OUTPUT       3U

struct device;

struct ssc_data;

/**
 * \brief Const clock device data
 *
 * This is the fixed data for tracking clocks attached to devices.
 */
struct dev_clk_data {
	/** Type of table entry */
	u16	type : 2;

	/**
	 * Modifying the frequency of this clock will attempt to modify
	 * the parent freq.
	 */
	u16	modify_parent_freq : 1;

	/**
	 * Clock divider, not valid for output, max 255 for parent otherwise
	 * up to USHORT_MAX/8.
	 */
	u16	cdiv : 13;

	/** For mux, the number of parents, for parents the index for the mux */
	u16	idx : 6;

	/** Which SoC clock this plugs into */
	u16	clk : 10;
};

/**
 * \brief Dynamic clock device data
 *
 * This tracks the current state of a device attached clock via flags.
 * This is just how the upstream clock is configured for the current
 * device. The upstream clock still has its own data and state.
 *
 * A pointer to a bss that has been reserved for this data is pointed
 * to by the device. If there is only one clock on the device, the field
 * is shared with the device flags and a forced cast is made to return
 * the field.
 */
struct dev_clk {
	u8 flags;
};

/* Declare a device clock of type input */
#define DEV_CLK(base, id, parent, _div) [(base) + (id)] = \
{ .clk				= (parent), .cdiv = (_div), .type = \
	  DEV_CLK_TABLE_TYPE_INPUT, \
	.modify_parent_freq	= (u16) 1U }
/* F variant does not allow parent freq to be modified */
#define DEV_CLKF(base, id, parent, _div) [(base) + (id)] = \
{ .clk				= (parent), .cdiv = (_div), .type = \
	  DEV_CLK_TABLE_TYPE_INPUT, \
	.modify_parent_freq	= SFALSE }

/* Declare a device clock of type mux */
#define DEV_CLK_MUX(base, id, parent, _div, n_parents) [(base) + (id)] = \
{ .clk				= (parent),    .cdiv = (_div), .type = DEV_CLK_TABLE_TYPE_MUX, \
	.idx			= (n_parents), \
	.modify_parent_freq	= STRUE }

/*
 * Declare a device clock of type mux parent. To match clk_mux_parent,
 * divider here can only be 8 bit
 */
#define DEV_CLK_PARENT(base, id, parent, _div, _idx) [(base) + (id)] =	 \
{ .clk				= \
	  (parent), \
	.type			= \
		DEV_CLK_TABLE_TYPE_PARENT, \
	.cdiv			= (_div) + (int) BUILD_ASSERT_OR_ZERO( \
	  _div < 256), \
	.idx			= (_idx), \
	.modify_parent_freq	= \
		1U }
/* F variant does not allow parent freq to be modified */
#define DEV_CLK_PARENTF(base, id, parent, _div, idx) [(base) + (id)] =	\
{ .clk				= \
	  (parent), \
	.type			= \
		DEV_CLK_TABLE_TYPE_PARENT, \
	.cdiv			= (_div) + (int) BUILD_ASSERT_OR_ZERO( \
	  _div < 256), \
	.idx			= (_idx), \
	.modify_parent_freq	= \
		SFALSE }

/* Declare a device clock of type output. */
#define DEV_CLK_OUTPUT(base, id, child) [(base) + (id)] = \
{ .clk				= (child), .type = DEV_CLK_TABLE_TYPE_OUTPUT, \
	.modify_parent_freq	= STRUE }

/**
 * \brief Get the dynamic data associated with a device clock.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \return
 * The dynamic device data. If clock ID is invalid, NULL is returned. Note
 * that if there is only one clock attached to the device, a struct device
 * is actually returned. Both struct dev_clk and struct device contain a
 * single u8 flags field. The flags are mutually exclusive.
 */
struct dev_clk *get_dev_clk(struct device *dev, dev_clk_idx_t clk_idx);

/**
 * \brief Get the const data associated with a device clock.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \return
 * The const data associated with the given clock, NULL if the clk_idx is
 * invalid.
 */
const struct dev_clk_data *get_dev_clk_data(struct device *dev, dev_clk_idx_t clk_idx);

/**
 * \brief Lookup the clock API clock associated with a device clock index.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \return
 * The internal clock API clock. NULL if the clk_idx is invalid or no such
 * clock exists.
 */
struct clk *dev_get_clk(struct device *dev, dev_clk_idx_t clk_idx);

/**
 * \brief Set the gate/ungate property for a device clock
 *
 * This sets/clears the gate/ungate property for a device clock. If a clock
 * is gated, the internal clock API clock that it references is not ref
 * counted. If it is ungated, then the refcount is incremented when the device
 * is enabled and decremented when the device is disabled.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \param gated
 * True to gate the clock, SFALSE to ungate.
 *
 * \return
 * True if operation succeeded, SFALSE if it failed or clk_idx was invalid.
 */
sbool device_clk_set_gated(struct device *dev, dev_clk_idx_t clk_idx, sbool gated);

/**
 * \brief Get the current hardware state of the clock.
 *
 * This returns the actual hardware status of the clock which can change
 * independentaly of the gated proprety. Specifically, the clock can be
 * running even though the clock is set to gated. This is mainly for
 * diagnostic purposes.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \return
 * True if the clock is currently running, SFALSE if the clock is gated or
 * in transition.
 */
sbool device_clk_get_hw_ready(struct device *dev, dev_clk_idx_t clk_idx);

/**
 * \brief Get the current state of the gated flag.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \return
 * The current state of the gated flag as set by device_clk_set_gated.
 */
sbool device_clk_get_sw_gated(struct device *dev, dev_clk_idx_t clk_idx);

/**
 * \brief Set the SSC for a device's clock
 *
 * This locates the correct clock and calls the internal clock API
 * set function. Configures the SSC on the PLL.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \param mod_freq_hz
 * The desired modulation frequency in Hz. The modulation period is the time
 * required to cycle the clock nominal frequency from an initial value through
 * all the different values along the modulation profile and back to the
 * initial value. The modulation frequency is the inverse of the period. The
 * minimum allowable frequency is 32 kHz and maximum is fREFCLK / 200. For
 * example, if the fREFCLK is 25 MHz, the maximum allowable modulation
 * frequency would be 125 kHz.
 *
 * \param mod_depth
 * The target modulation depth in "permyriad". The modulation depth refers to
 * the maximum variation in frequency as a percentage of the clock center
 * frequency. The minimum modulation depth is 0.1% and the maximum is 3.1%.
 * Modulation depth can be adjusted in 0.1% increments.
 *
 * \param spread_type
 * The target spread type. Both center and down spread are supported.
 *
 * \param enable
 * Enable or Disable SSC
 *
 * \return
 * True if the new SSC configuration was accepted, SFALSE otherwise.
 */
sbool device_clk_set_ssc(struct device *dev, dev_clk_idx_t clk_idx, u32 modfreq_hz, u32 mod_depth, u8 spread_type, sbool enable);

/**
 * \brief Get the current SSC configuration of a device clock.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \param ssc_data
 * The current SSC configuration of the clock.
 *
 * \return
 * True if the SSC configuration was found, SFALSE otherwise.
 */
sbool device_clk_get_ssc(struct device *dev, dev_clk_idx_t clk_idx, struct ssc_data *ssc_datap);

/**
 * \brief Allow or block frequency changes.
 *
 * Device clocks can be changed via the set_freq calls. However, changing the
 * frequency of one clock may also effect the frequency of another. Normally
 * this is blocked if that other clock is enabled. Setting this allow flag to
 * STRUE prevents these frequency changes from being blocked.
 *
 * For some clocks, such as observation clocks, this flag is set to STRUE by
 * default.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \param allow
 * True to allow frequency changes of this clock, even when enabled. False
 * to block frequency changes while the clock is enabled.
 */
void device_clk_set_freq_change(struct device *dev, dev_clk_idx_t clk_idx, sbool allow);

/**
 * \brief Get the current state of the allow frequency change flag.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \return
 * True if frequency changes are allowed, even when enabled. False otherwise.
 */
sbool device_clk_get_freq_change(struct device *dev, dev_clk_idx_t clk_idx);

/**
 * \brief Set/clear the input termination flag for a device clock.
 *
 * This API is not yet fully implemented, but is meant to control the input
 * termination setting for clocks.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \param enable
 * True to enable termination flag, SFALSE to disable.
 */
void device_clk_set_input_term(struct device *dev, dev_clk_idx_t clk_idx, sbool enable);

/**
 * \brief Get the current state of the termination flag.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \return
 * True if terminatino is enabled, SFALSE if it is disabled.
 */
sbool device_clk_get_input_term(struct device *dev, dev_clk_idx_t clk_idx);

/**
 * \brief Change a device mux clocks's parent.
 *
 * \param dev
 * The device ID that the clock mux and new parent is connected to.
 *
 * \param clk_idx
 * The index of the clock mux on this device.
 *
 * \param parent_idx
 * The index of the new clock parent on this device.
 *
 * \return
 * True if parent change was successful, SFALSE if it failed or one of the
 * clock indexes was invalid.
 */
sbool device_clk_set_parent(struct device *dev, dev_clk_idx_t clk_idx, dev_clk_idx_t parent_idx);

/**
 * \brief Get the current parent of a device mux clock.
 *
 * \param dev
 * The device ID that the clock mux is connected to.
 *
 * \param clk_idx
 * The index of the clock mux on this device.
 *
 * \return
 * The index of the device clock that is the current parent of the mux, or
 * DEV_CLK_ID_NONE if clk_idx was invalid, is not a mux, or does not point to
 * a valid clock.
 */
dev_clk_idx_t device_clk_get_parent(struct device *dev, dev_clk_idx_t clk_idx);

/**
 * \brief Get the number of parents of a given device mux clock.
 *
 * \param dev
 * The device ID that the clock mux is connected to.
 *
 * \param clk_idx
 * The index of the clock mux on this device.
 *
 * \return
 * If this is a clock mux, it returns the number of possible parent clocks
 * of the clock mux. If it is not a clock mux, it returns 1 if the clock
 * has a parent, or 0 if it is a parentless clock. If clk_idx is invalid,
 * DEV_CLK_ID_NONE is returned.
 */
dev_clk_idx_t device_clk_get_num_parents(struct device *dev, dev_clk_idx_t clk_idx);

/**
 * \brief Set the frequency of a device's clock
 *
 * This locates the correct clock and calls the internal clock API
 * set function. If the clock is a mux type, we instead send the request to
 * the parent. This is because calling set freq on the mux may switch the mux
 * which is not what we want on muxes that are exposed on devices.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \param min_freq_hz
 * The minimum acceptable frequency (Hz).
 *
 * \param target_freq_hz
 * The clock API will attempt to return a frequency as close as possible to the
 * target frequency (Hz).
 *
 * \param max_freq_hz
 * The maximum acceptable frequency (Hz).
 *
 * \return
 * True if the new frequency was accepted, SFALSE otherwise.
 */
sbool device_clk_set_freq(struct device *dev, dev_clk_idx_t clk_idx, u32 min_freq_hz, u32 target_freq_hz, u32 max_freq_hz);

/**
 * \brief Query the clock API.
 *
 * This determines what response the current clock tree would have to a
 * hypothetical frequency change request.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \param min_freq_hz
 * The minimum acceptable frequency (Hz).
 *
 * \param target_freq_hz
 * The clock API will attempt to return a frequency as close as possible to the
 * target frequency (Hz).
 *
 * \param max_freq_hz
 * The maximum acceptable frequency (Hz).
 *
 * \return
 * The frequency that would be programmed if device_clk_set_freq were called
 * with the same parameters. Returns 0 if an acceptable configuration was not
 * found.
 */
u32 device_clk_query_freq(struct device *dev, dev_clk_idx_t clk_idx, u32 min_freq_hz, u32 target_freq_hz, u32 max_freq_hz);

/**
 * \brief Get the current frequency of a device clock.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \return
 * The frequency the clock is running at if it enabled. If the clock is
 * disabled, it returns the frequency the clock would be running at if it
 * were enabled. Returns 0 if the clk_idx is invalid or if the clock cannot
 * run for some reason.
 */
u32 device_clk_get_freq(struct device *dev, dev_clk_idx_t clk_idx);

#ifdef CONFIG_CLOCK
/**
 * \brief Enable a device clock
 *
 * This is called by the device pm code for each clock when the device is
 * enabled. This allows the device clk code to take appropriate action.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 */
void device_clk_enable(struct device *dev, dev_clk_idx_t clk_idx);

/**
 * \brief Disable a device clock
 *
 * This is called by the device pm code for each clock when the device is
 * disabled. This allows the device clk code to take appropriate action.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 */
void device_clk_disable(struct device *dev, dev_clk_idx_t clk_idx);

/**
 * \brief Initialize a device clock
 *
 * This is called by the device pm code for each clock when the device is
 * disabled. This allows the device clk code to take appropriate action such
 * setting default state.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 */
void device_clk_init(struct device *dev, dev_clk_idx_t clk_idx);

#else
static inline void device_clk_enable(struct device *dev, dev_clk_idx_t clk_idx)
{
}
static inline void device_clk_disable(struct device *dev, dev_clk_idx_t clk_idx)
{
}
static inline void device_clk_init(struct device *dev, dev_clk_idx_t clk_idx)
{
}
#endif

#endif
