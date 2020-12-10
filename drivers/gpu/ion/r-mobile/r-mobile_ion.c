/*
 * drivers/gpu/ion/r-mobile/r-mobile_ion.c
 *
 * Copyright (C) 2012 Renesas Electronics Corporation
 *
 * Based on the following files:
 *	drivers/ion/gpu/tegra/tegra_ion.c
 *	drivers/ion/gpu/omap/omap_ion.c
 *
 * Copyright (C) 2011 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt) "ion-rmobile: " fmt

#include <linux/module.h>
#include <linux/err.h>
#include <linux/ion.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/scatterlist.h>
#include <rtapi/system_memory.h>
#include <linux/r-mobile/r_mobile_ion.h>
#include "../ion_priv.h"

struct rt_heap {
	unsigned int	type;
	unsigned int	base;
	unsigned int	size;
	void		*rtaddr;
};


static struct ion_device *idev;
static int num_heaps;
static struct ion_heap **heaps;
static void *rtmem;
static struct rt_heap *rt_heaps;

/**
 *  Acquire the ION device. It is created during ION probe
 *  and the global handle is shared with clients.
 *  It is client's responsibility to check for valid ION device
 *  and make sure that the acquisition of ION device is done
 *  after ION device probe is over.
 *
 *  Returns the ION device
 */
struct ion_device *r_mobile_ion_device_get(void)
{
	return idev;
}
EXPORT_SYMBOL(r_mobile_ion_device_get);

/**
 *  Get the RT mapped (iommu) address for the ION buffer. The mapping
 *  is created when the buffer is created.
 *  The mapping gets destroyed when the buffer is destroyed.
 *
 *  Returns the RT mapped address
 **/
unsigned int r_mobile_ion_rt_addr_get(struct ion_client *client,
		struct ion_handle *handle)
{
	struct ion_buffer *buffer;
	unsigned int rt_addr = 0;

	buffer = ion_buffer_lock(client, handle);
	if (!buffer) {
		pr_err("%s could not lock the buffer\n", __func__);
		return rt_addr;
	}

	rt_addr = buffer->rt_addr;
	ion_buffer_unlock(client, buffer);
	if (!rt_addr)
		pr_err("%s No valid RT address\n", __func__);

	pr_debug("ion get_rt client(%p) handle(%p) buffer(%p) rt_addr(%x)\n",
			client, handle, buffer, rt_addr);
	return rt_addr;
}
EXPORT_SYMBOL(r_mobile_ion_rt_addr_get);


/**
 *  Map the ION buffer to RT-Domain and store the RT address
 *  in ion_buffer structure
 **/
int r_mobile_ion_map_pnc_nma(struct ion_buffer *buffer)
{
	int ret;
	system_mem_rt_map_pnc_nma rt_map_pnc_nma;
	int num_pages = PAGE_ALIGN(buffer->size) / PAGE_SIZE;
	struct sg_table *table = buffer->sg_table;
	struct scatterlist *sg;
	struct page **pages;
	int i, j, k = 0;

	pages = vmalloc(sizeof(struct page *) * num_pages);
	for_each_sg(table->sgl, sg, table->nents, i) {
		struct page *page = sg_page(sg);
		for (j = 0; j < sg_dma_len(sg) / PAGE_SIZE; j++)
			pages[k++] = page++;
	}
	rt_map_pnc_nma.handle = rtmem;
	rt_map_pnc_nma.map_size = buffer->size;
	rt_map_pnc_nma.pages = pages;
	ret = system_memory_rt_map_pnc_nma(&rt_map_pnc_nma);
	if (ret != SMAP_LIB_MEMORY_OK) {
		pr_err("%s system_memory_rt_map_pnc_nma error\n", __func__);
		vfree(pages);
		buffer->rt_addr = 0;
		return -ENOMEM;
	}
	vfree(pages);
	buffer->rt_addr = rt_map_pnc_nma.rt_addr_wb;
	pr_debug("ion map buffer(%p) rt_addr(%x)\n", buffer, buffer->rt_addr);
	return 0;
}

/**
 *  Unmap the ION buffer RT-Domain mapping
 **/
void r_mobile_ion_unmap_pnc_nma(struct ion_buffer *buffer)
{
	int ret;
	system_mem_rt_unmap_pnc_nma rt_unmap_pnc_nma;

	pr_debug("ion unmap buffer(%p) rt_addr(%x)\n", buffer,
			buffer->rt_addr);
	if (buffer->rt_addr) {
		rt_unmap_pnc_nma.handle = rtmem;
		rt_unmap_pnc_nma.map_size = buffer->size;
		rt_unmap_pnc_nma.rt_addr_wb = buffer->rt_addr;
		ret = system_memory_rt_unmap_pnc_nma(&rt_unmap_pnc_nma);
		if (ret != SMAP_LIB_MEMORY_OK)
			pr_err("%s system_memory_rt_unmap_pnc_nma error\n",
					__func__);
	}
}

static void *r_mobile_ion_map_rtmem(unsigned int base, unsigned int size)
{
	int ret;
	system_mem_rt_map rt_map;
	system_mem_reg_phymem reg_phymem;

	/* map the video heap to RT-Domain */
	rt_map.handle = rtmem;
	rt_map.phys_addr = base;
	rt_map.map_size = size;
	ret = system_memory_rt_map(&rt_map);
	if (ret != SMAP_LIB_MEMORY_OK) {
		printk(KERN_ERR "%s: system_memory_rt_map error: %d\n",
		       __func__, ret);
		return ERR_PTR(-ENOMEM);
	}

	/* register the RT-mapped area */
	reg_phymem.handle = rtmem;
	reg_phymem.phys_addr = base;
	reg_phymem.map_size = size;
	reg_phymem.rtaddr = rt_map.rtaddr;
	ret = system_memory_reg_phymem(&reg_phymem);
	if (ret != SMAP_LIB_MEMORY_OK) {
		system_mem_rt_unmap rt_unmap;

		printk(KERN_ERR "%s: system_memory_reg_phymem error: %d\n",
		       __func__, ret);

		rt_unmap.handle = rtmem;
		rt_unmap.rtaddr = rt_map.rtaddr;
		rt_unmap.map_size = size;
		system_memory_rt_unmap(&rt_unmap);
		return ERR_PTR(-ENOMEM);
	}

	return (void *) rt_map.rtaddr;
}

static int r_mobile_ion_unmap_rtmem(unsigned int base, unsigned int size,
				    void *rtaddr)
{
	system_mem_unreg_phymem unreg_phymem;
	system_mem_rt_unmap rt_unmap;

	/* unregister the RT-mapped area */
	unreg_phymem.handle = rtmem;
	unreg_phymem.phys_addr = base;
	unreg_phymem.map_size = size;
	unreg_phymem.rtaddr = (unsigned int)rtaddr;
	system_memory_unreg_phymem(&unreg_phymem);

	/* unmap the video heap */
	rt_unmap.handle = rtmem;
	rt_unmap.rtaddr = (unsigned int)rtaddr;
	rt_unmap.map_size = size;
	system_memory_rt_unmap(&rt_unmap);

	return 0;
}

static long r_mobile_ion_map_rtmem_all(void)
{
	int i;
	void *rtaddr;

	for (i = 0; i < num_heaps; i++) {
		if (rt_heaps[i].type == ION_HEAP_TYPE_CARVEOUT) {
			rtaddr = r_mobile_ion_map_rtmem(rt_heaps[i].base,
							rt_heaps[i].size);
			if (IS_ERR_OR_NULL(rtaddr))
				return -ENOMEM;
			rt_heaps[i].rtaddr = rtaddr;
		}
	}
	return 0;
}

static void r_mobile_ion_unmap_rtmem_all(void)
{
	int i;

	for (i = 0; i < num_heaps; i++) {
		if (rt_heaps[i].type == ION_HEAP_TYPE_CARVEOUT) {
			r_mobile_ion_unmap_rtmem(rt_heaps[i].base,
						 rt_heaps[i].size,
						 rt_heaps[i].rtaddr);
		}
	}
}

static long r_mobile_ion_ioctl(struct ion_client *client, unsigned int cmd,
			       unsigned long arg)
{

	switch (cmd) {
	case R_MOBILE_ION_RT_MAP:
		return r_mobile_ion_map_rtmem_all();
	default:
		pr_err("%s: Unknown custom ioctl\n", __func__);
		return -ENOTTY;
	}
	return 0;
}

static int r_mobile_ion_probe(struct platform_device *pdev)
{
	struct ion_platform_data *pdata = pdev->dev.platform_data;
	int err;
	int i;
	system_mem_info_delete info_delete;

	num_heaps = pdata->nr;

	rtmem = system_memory_info_new();
	if (!rtmem) {
		printk(KERN_ERR "%s: system_memory_info_new error\n", __func__);
		return -ENOMEM;
	}

	heaps = kzalloc(sizeof(struct ion_heap *) * pdata->nr, GFP_KERNEL);
	if (!heaps) {
		err = -ENOMEM;
		goto err_heap;
	}

	rt_heaps = kzalloc(sizeof(struct rt_heap) * pdata->nr, GFP_KERNEL);
	if (!rt_heaps) {
		err = -ENOMEM;
		goto err_rt_heap;
	}

	idev = ion_device_create(r_mobile_ion_ioctl);
	if (IS_ERR_OR_NULL(idev)) {
		err = PTR_ERR(idev);
		goto err_dev;
	}

	/* create the heaps as specified in the board file */
	for (i = 0; i < num_heaps; i++) {
		struct ion_platform_heap *heap_data = &pdata->heaps[i];

		heaps[i] = ion_heap_create(heap_data);
		if (IS_ERR_OR_NULL(heaps[i])) {
			err = PTR_ERR(heaps[i]);
			goto err;
		}
		ion_device_add_heap(idev, heaps[i]);

		rt_heaps[i].type = heap_data->type;
		if (heap_data->type == ION_HEAP_TYPE_CARVEOUT) {
			rt_heaps[i].base = heap_data->base;
			rt_heaps[i].size = heap_data->size;
		}
	}
	platform_set_drvdata(pdev, idev);
	return 0;
err:
	for (i = 0; i < num_heaps; i++) {
		if (heaps[i])
			ion_heap_destroy(heaps[i]);
	}
err_dev:
	kfree(rt_heaps);
err_rt_heap:
	kfree(heaps);
err_heap:
	info_delete.handle = rtmem;
	system_memory_info_delete(&info_delete);
	return err;
}

static int r_mobile_ion_remove(struct platform_device *pdev)
{
	struct ion_device *idev = platform_get_drvdata(pdev);
	int i;
	system_mem_info_delete info_delete;

	ion_device_destroy(idev);
	for (i = 0; i < num_heaps; i++)
		ion_heap_destroy(heaps[i]);
	kfree(heaps);

	r_mobile_ion_unmap_rtmem_all();
	kfree(rt_heaps);

	info_delete.handle = rtmem;
	system_memory_info_delete(&info_delete);
	return 0;
}

static struct platform_driver ion_driver = {
	.probe = r_mobile_ion_probe,
	.remove = r_mobile_ion_remove,
	.driver = { .name = "ion-r-mobile" }
};

static int __init ion_init(void)
{
	return platform_driver_register(&ion_driver);
}

static void __exit ion_exit(void)
{
	platform_driver_unregister(&ion_driver);
}

module_init(ion_init);
module_exit(ion_exit);

MODULE_AUTHOR("Renesas Electronics Corporation");
MODULE_LICENSE("GPL");
