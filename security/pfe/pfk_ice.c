// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015-2020, The Linux Foundation. All rights reserved.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/async.h>
#include <linux/mm.h>
#include <linux/of.h>
#include <linux/device-mapper.h>
#include <soc/qcom/scm.h>
#include <soc/qcom/qseecomi.h>
#include <soc/qcom/qtee_shmbridge.h>
#include <crypto/ice.h>
#include <linux/soc/qcom/crypto-qti-hwkm.h>
#include "pfk_ice.h"

/**********************************/
/** global definitions		 **/
/**********************************/

#define TZ_ES_CONFIG_SET_ICE_KEY_CE_TYPE 0x5
#define TZ_ES_INVALIDATE_ICE_KEY_CE_TYPE 0x6

/* index 0 and 1 is reserved for FDE */
#define MIN_ICE_KEY_INDEX 2

#define MAX_ICE_KEY_INDEX 31

#define TZ_ES_CONFIG_SET_ICE_KEY_CE_TYPE_ID \
	TZ_SYSCALL_CREATE_SMC_ID(TZ_OWNER_SIP, TZ_SVC_ES, \
	TZ_ES_CONFIG_SET_ICE_KEY_CE_TYPE)

#define TZ_ES_INVALIDATE_ICE_KEY_CE_TYPE_ID \
		TZ_SYSCALL_CREATE_SMC_ID(TZ_OWNER_SIP, \
			TZ_SVC_ES, TZ_ES_INVALIDATE_ICE_KEY_CE_TYPE)

#define TZ_ES_INVALIDATE_ICE_KEY_CE_TYPE_PARAM_ID \
	TZ_SYSCALL_CREATE_PARAM_ID_2( \
	TZ_SYSCALL_PARAM_TYPE_VAL, TZ_SYSCALL_PARAM_TYPE_VAL)

#define TZ_ES_CONFIG_SET_ICE_KEY_CE_TYPE_PARAM_ID \
	TZ_SYSCALL_CREATE_PARAM_ID_6( \
	TZ_SYSCALL_PARAM_TYPE_VAL, \
	TZ_SYSCALL_PARAM_TYPE_BUF_RW, TZ_SYSCALL_PARAM_TYPE_VAL, \
	TZ_SYSCALL_PARAM_TYPE_VAL, TZ_SYSCALL_PARAM_TYPE_VAL, \
	TZ_SYSCALL_PARAM_TYPE_VAL)

#define CONTEXT_SIZE 0x1000

#define ICE_BUFFER_SIZE 64

#define PFK_UFS "ufs"
#define PFK_SDCC "sdcc"
#define PFK_UFS_CARD "ufscard"

#define UFS_CE 10
#define SDCC_CE 20
#define UFS_CARD_CE 30

enum {
	ICE_CIPHER_MODE_XTS_128 = 0,
	ICE_CIPHER_MODE_CBC_128 = 1,
	ICE_CIPHER_MODE_XTS_256 = 3,
	ICE_CIPHER_MODE_CBC_256 = 4
};

static void qti_pfk_ice_stat_failure(char *type, uint32_t id, int32_t err)
{
	static uint32_t set_key_failure, invalidate_key_failure;

	if (err == 0)
		return;

	if (id == TZ_ES_CONFIG_SET_ICE_KEY_CE_TYPE_ID)
		set_key_failure++;
	else if (id == TZ_ES_INVALIDATE_ICE_KEY_CE_TYPE_ID)
		invalidate_key_failure++;
	else {
		pr_warn("%s: unsupported id %d\n", __func__, id);
		return;
	}
	pr_warn("%s: failed to call scm %d (%d %d)\n",
			__func__, id, set_key_failure, invalidate_key_failure);
	if (err != -EBUSY || !strcmp(type, (char *)PFK_SDCC))
		BUG();
	BUG_ON((invalidate_key_failure + set_key_failure) > 10);
}
static bool pfk_key_set_using_hwkm;

static int set_key(uint32_t index, const uint8_t *key, const uint8_t *salt,
		unsigned int data_unit, struct ice_device *ice_dev)
{
	struct scm_desc desc = {0};
	int ret = 0;
	uint32_t smc_id = 0;
	char *tzbuf = NULL;
	uint32_t key_size = ICE_BUFFER_SIZE / 2;
	struct qtee_shm shm;

	ret = qtee_shmbridge_allocate_shm(ICE_BUFFER_SIZE, &shm);
	if (ret)
		return -ENOMEM;

	tzbuf = shm.vaddr;

	memcpy(tzbuf, key, key_size);
	memcpy(tzbuf + key_size, key + key_size, key_size);
	dmac_flush_range(tzbuf, tzbuf + ICE_BUFFER_SIZE);

	smc_id = TZ_ES_CONFIG_SET_ICE_KEY_CE_TYPE_ID;

	desc.arginfo = TZ_ES_CONFIG_SET_ICE_KEY_CE_TYPE_PARAM_ID;
	desc.args[0] = index;
	desc.args[1] = shm.paddr;
	desc.args[2] = shm.size;
	desc.args[3] = ICE_CIPHER_MODE_XTS_256;
	desc.args[4] = data_unit;

	if (!strcmp(ice_dev->ice_instance_type, (char *)PFK_UFS_CARD))
		desc.args[5] = UFS_CARD_CE;
	else if (!strcmp(ice_dev->ice_instance_type, (char *)PFK_SDCC))
		desc.args[5] = SDCC_CE;
	else if (!strcmp(ice_dev->ice_instance_type, (char *)PFK_UFS))
		desc.args[5] = UFS_CE;

	ret = scm_call2_noretry(smc_id, &desc);
	if (ret)
		pr_err("%s:SCM call Error: 0x%x\n", __func__, ret);

	qti_pfk_ice_stat_failure(ice_dev->ice_instance_type, smc_id, ret);
	qtee_shmbridge_free_shm(&shm);
	return ret;
}

static int clear_key(uint32_t index, struct ice_device *ice_dev)
{
	struct scm_desc desc = {0};
	int ret = 0;
	uint32_t smc_id = 0;

	smc_id = TZ_ES_INVALIDATE_ICE_KEY_CE_TYPE_ID;

	desc.arginfo = TZ_ES_INVALIDATE_ICE_KEY_CE_TYPE_PARAM_ID;
	desc.args[0] = index;

	if (!strcmp(ice_dev->ice_instance_type, (char *)PFK_UFS_CARD))
		desc.args[1] = UFS_CARD_CE;
	else if (!strcmp(ice_dev->ice_instance_type, (char *)PFK_SDCC))
		desc.args[1] = SDCC_CE;
	else if (!strcmp(ice_dev->ice_instance_type, (char *)PFK_UFS))
		desc.args[1] = UFS_CE;

	ret = scm_call2_noretry(smc_id, &desc);
	if (ret)
		pr_err("%s:SCM call Error: 0x%x\n", __func__, ret);
	qti_pfk_ice_stat_failure(ice_dev->ice_instance_type, smc_id, ret);
	return ret;
}

int qti_pfk_ice_set_key(uint32_t index, uint8_t *key, uint8_t *salt,
			struct ice_device *ice_dev, unsigned int data_unit,
			size_t size)
{
	int ret = 0, ret1 = 0;

	if (index < MIN_ICE_KEY_INDEX || index > MAX_ICE_KEY_INDEX) {
		pr_err("%s Invalid index %d\n", __func__, index);
		return -EINVAL;
	}
	if (!key) {
		pr_err("%s Invalid key/salt\n", __func__);
		return -EINVAL;
	}

	ret = enable_ice_setup(ice_dev);
	if (ret) {
		pr_err("%s: could not enable clocks: %d\n", __func__, ret);
		goto out;
	}

	if (ice_dev->hwkm_supported && (size > 64)) {
		pr_debug("%s through hkm\n", __func__);
		ret = crypto_qti_program_key(index, key, ice_dev, data_unit);
		if (ret) {
			pr_err("%s hwkm key programing failed with error %d\n",
				__func__, ret);

			ret1 = crypto_qti_invalidate_key(ice_dev, index);
			if (ret1)
				pr_err("%s key invalidate failed error %d\n",
					__func__, ret1);
		} else {
			pfk_key_set_using_hwkm = true;
		}
	} else {
		pr_debug("%s through scm\n", __func__);
		ret = set_key(index, key, salt, data_unit, ice_dev);
		if (ret) {
			pr_err("%s: Set Key Error: %d\n", __func__, ret);
			if (ret == -EBUSY) {
				if (disable_ice_setup(ice_dev))
					pr_err("%s: clock disable failed\n",
						__func__);
				goto out;
			}
			/* Invalidate the key to keep ICE in proper state */
			ret1 = clear_key(index, ice_dev);
			if (ret1)
				pr_err("%s: Invalidate key error: %d\n",
					__func__, ret1);
		}
	}

	ret1 = disable_ice_setup(ice_dev);
	if (ret1)
		pr_err("%s: Error %d disabling clocks\n", __func__, ret);

out:
	return ret;
}

int qti_pfk_ice_invalidate_key(uint32_t index, struct ice_device *ice_dev)
{
	int ret = 0;

	if (index < MIN_ICE_KEY_INDEX || index > MAX_ICE_KEY_INDEX) {
		pr_err("%s Invalid index %d\n", __func__, index);
		return -EINVAL;
	}

	ret = enable_ice_setup(ice_dev);
	if (ret) {
		pr_err("%s: could not enable clocks: 0x%x\n", __func__, ret);
		return ret;
	}

	if (ice_dev->hwkm_supported && pfk_key_set_using_hwkm) {
		ret = crypto_qti_invalidate_key(ice_dev, index);
		if (ret)
			pr_err("%s hwkm key invalidate failed with error %d\n",
				__func__, ret);
	} else {
		ret = clear_key(index, ice_dev);
		if (ret)
			pr_err("%s: Invalidate key error: %d\n", __func__, ret);
	}

	if (disable_ice_setup(ice_dev))
		pr_err("%s: could not disable clocks\n", __func__);

	return ret;
}
