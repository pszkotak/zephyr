/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <random/rand32.h>
#include <logging/log.h>

#include <openthread/platform/entropy.h>

#include "platform-zephyr.h"

LOG_MODULE_REGISTER(net_otPlat_entropy, CONFIG_OPENTHREAD_L2_LOG_LEVEL);

otError otPlatEntropyGet(uint8_t *aOutput, uint16_t aOutputLength)
{
	if (IS_ENABLED(CONFIG_ENTROPY_HAS_DRIVER)) {
		sys_csrand_get(aOutput, aOutputLength);
	} else {
		sys_rand_get(aOutput, aOutputLength);
	}

	return OT_ERROR_NONE;
}
