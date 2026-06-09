/*
 * System Firmware Source File
 *
 * DMSC Tx Routing Data for AM275X device
 *
 * Data version: 220316_180112
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include <hosts.h>
#include <sciserver_secproxyConfigData.h>
#include <sciserver_secproxyRoutingDescription.h>

const uint16_t sproxy_route_data_count = AM62X_SEC_PROXY_TX_CONFIG_MAX;

const struct sproxy_host_tx_thread_data sproxy_route_data[AM62X_SEC_PROXY_TX_CONFIG_MAX] = {
	/*
	 * Non Secure Host MAIN_0_R5_1[41]
	 */
	[HOST_ID_MAIN_0_R5_1] = {
		.sproxy_resp_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_MAIN_0_R5_1_RESPONSE_TX_THR066_CONF007,
		.sproxy_ntfy_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_MAIN_0_R5_1_RESPONSE_TX_THR066_CONF007,
	},
	/*
	 * Non Secure Host MAIN_0_R5_3[43]
	 */
	[HOST_ID_MAIN_0_R5_3] = {
		.sproxy_resp_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_MAIN_0_R5_3_RESPONSE_TX_THR065_CONF008,
		.sproxy_ntfy_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_MAIN_0_R5_3_RESPONSE_TX_THR065_CONF008,
	},
	/*
	 * Non Secure Host MAIN_1_R5_1[46]
	 */
	[HOST_ID_MAIN_1_R5_1] = {
		.sproxy_resp_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_MAIN_1_R5_1_RESPONSE_TX_THR064_CONF009,
		.sproxy_ntfy_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_MAIN_1_R5_1_RESPONSE_TX_THR064_CONF009,
	},
	/*
	 * Non Secure Host MAIN_1_R5_3[48]
	 */
	[HOST_ID_MAIN_1_R5_3] = {
		.sproxy_resp_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_MAIN_1_R5_3_RESPONSE_TX_THR063_CONF010,
		.sproxy_ntfy_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_MAIN_1_R5_3_RESPONSE_TX_THR063_CONF010,
	},
	/*
	 * Secure Host DM2TIFS[250]
	 */
	[HOST_ID_DM2TIFS] =	{
		.sproxy_resp_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_TIFS_SEC_DM2TIFS_RESPONSE_TX_THR069_CONF005,
		.sproxy_ntfy_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_TIFS_SEC_DM2TIFS_RESPONSE_TX_THR069_CONF005,
	},
	/*
	 * Non Secure Host TIFS2DM[251]
	 */
	[HOST_ID_TIFS2DM] =	{
		.sproxy_resp_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_TIFS2DM_RESPONSE_TX_THR060_CONF013,
		.sproxy_ntfy_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_TIFS2DM_RESPONSE_TX_THR060_CONF013,
	},
	/*
	 * Non Secure Host WKUP R5[35]
	 */
	[HOST_ID_WKUP_0_R5_1] =	{
		.sproxy_resp_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_WKUP_0_R5_1_RESPONSE_TX_THR067_CONF006,
		.sproxy_ntfy_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_WKUP_0_R5_1_RESPONSE_TX_THR067_CONF006,
	},
	/*
	 * Non Secure Host C7x[20]
	 */
	[HOST_ID_C7X_0_0] =	{
		.sproxy_resp_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_C7X_0_0_RESPONSE_TX_THR062_CONF011,
		.sproxy_ntfy_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_C7X_0_0_RESPONSE_TX_THR062_CONF011,
	},
		/*
	 * Non Secure Host C7x1[22]
	 */
	[HOST_ID_C7X_1_0] =	{
		.sproxy_resp_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_C7X_1_0_RESPONSE_TX_THR061_CONF012,
		.sproxy_ntfy_conf_id	= AM275X_DMASS0_SEC_PROXY_0_TX_DM_NONSEC_C7X_1_0_RESPONSE_TX_THR061_CONF012,
	},
};