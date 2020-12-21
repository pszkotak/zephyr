/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef OPENAMP_MULTI_INSTANCE_H
#define OPENAMP_MULTI_INSTANCE_H

/**
 * @file
 * @defgroup rpmsg.h IPC multi instance communication library
 * @{
 * @brief IPC multi instance communication library.
 *        This library enables user to use multiple OpenAmp
 *        instances and multiple endpoints for each instance.
 */

#include <openamp/virtio_ring.h>

#define IPC_INSTANCE_COUNT                                                     \
	(CONFIG_OPENAMP_INSTANCES_NUM) /**< Total number of open-amp instances.*/

#define VRING_ALIGNMENT    ( 4 )   /**< Alignment of vring buffer.*/

#define VDEV_STATUS_SIZE   ( 0x4 ) /**< Size of status region. */

/* Private macros. */
#define VRING_DESC_SIZEOF(num) ((num) * (sizeof(struct vring_desc)))
#define VRING_AVAIL_SIZEOF(num) (sizeof(struct vring_avail) +             \
                                ((num) * sizeof(uint16_t)) +              \
                                sizeof(uint16_t))
#define VRING_ALIGN(size, align) (((size) + (align) - 1) & ~((align) - 1))
#define VRING_USED_SIZEOF(num) (sizeof(struct vring_used) +               \
                               ((num) * sizeof(struct vring_used_elem)) + \
                               sizeof(uint16_t))

#define VRING_FIRST_SUM(num) (VRING_DESC_SIZEOF(num) + VRING_AVAIL_SIZEOF(num))

/* Public macros */
/* Compute size of vring buffer basen on its size and allignment. */
#define VRING_SIZE_COMPUTE(vring_size, align) (VRING_ALIGN(VRING_FIRST_SUM((vring_size)),(align)) + \
                                               VRING_USED_SIZEOF((vring_size)))

/* Macro for calculating used memory by virtqueue buffers for remote device. */
#define VIRTQUEUE_SIZE_GET(vring_size)  (RPMSG_BUFFER_SIZE * (vring_size))

/* Macro for getting the size of shared memory occupied by single IPC instance. */
#define SHMEM_INST_SIZE_GET(vring_size) (VDEV_STATUS_SIZE +                      \
                                        (2 * VIRTQUEUE_SIZE_GET((vring_size))) + \
                                        (2 * VRING_SIZE_COMPUTE((vring_size),(VRING_ALIGNMENT))))

/* Returns size of used shared memory consumed by all IPC instances*/
#define SHMEM_CONSUMED_SIZE_GET(vring_size) ( IPC_INSTANCE_COUNT * SHMEM_INST_SIZE_GET((vring_size)))

/* Returns maximum allowable size of vring buffers to fit memory requirements. */
#define VRING_SIZE_GET(shmem_size) ((SHMEM_CONSUMED_SIZE_GET(32)) < (shmem_size) ? 32 :\
                                    (SHMEM_CONSUMED_SIZE_GET(16)) < (shmem_size) ? 16 :\
                                    (SHMEM_CONSUMED_SIZE_GET(8))  < (shmem_size) ? 8  :\
                                    (SHMEM_CONSUMED_SIZE_GET(4))  < (shmem_size) ? 4  :\
                                    (SHMEM_CONSUMED_SIZE_GET(2))  < (shmem_size) ? 2 : 1)

/* Returns size of used shared memory of single instance in case of using
 * maximum allowable vring buffer size. */
#define SHMEM_INST_SIZE_AUTOALLOC_GET(shmem_size) (SHMEM_INST_SIZE_GET(VRING_SIZE_GET((shmem_size))))

/* Returns start address of ipc instance in shared memory. It assumes that
 * maximum allowable vring buffer size is used. */
#define SHMEM_INST_ADDR_AUTOALLOC_GET(shmem_addr, shmem_size, id) ((shmem_addr) + ((id) * (SHMEM_INST_SIZE_AUTOALLOC_GET(shmem_size))))

/**
 *@}
 */

#endif /* OPENAMP_MULTI_INSTANCE_H  */
