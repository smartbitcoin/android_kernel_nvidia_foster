/*
 * Copyright (c) 2014, NVIDIA CORPORATION.  All rights reserved.
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

#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/tegra-powergate.h>

#include "../../../arch/arm/mach-tegra/powergate-priv.h"
#include "../../../arch/arm/mach-tegra/powergate-ops-t1xx.h"
#include "../../../arch/arm/mach-tegra/powergate-ops-txx.h"
#include "../../../arch/arm/mach-tegra/dvfs.h"

enum mc_client {
	MC_CLIENT_AFI		= 0,
	MC_CLIENT_AVPC		= 1,
	MC_CLIENT_DC		= 2,
	MC_CLIENT_DCB		= 3,
	MC_CLIENT_HC		= 6,
	MC_CLIENT_HDA		= 7,
	MC_CLIENT_ISP2		= 8,
	MC_CLIENT_NVENC		= 11,
	MC_CLIENT_SATA		= 15,
	MC_CLIENT_VI		= 17,
	MC_CLIENT_VIC		= 18,
	MC_CLIENT_XUSB_HOST	= 19,
	MC_CLIENT_XUSB_DEV	= 20,
	MC_CLIENT_BPMP		= 21,
	MC_CLIENT_TSEC		= 22,
	MC_CLIENT_SDMMC1	= 29,
	MC_CLIENT_SDMMC2	= 30,
	MC_CLIENT_SDMMC3	= 31,
	MC_CLIENT_SDMMC4	= 32,
	MC_CLIENT_ISP2B		= 33,
	MC_CLIENT_GPU		= 34,
	MC_CLIENT_NVDEC		= 37,
	MC_CLIENT_APE		= 38,
	MC_CLIENT_SE		= 39,
	MC_CLIENT_NVJPG		= 40,
	MC_CLIENT_TSECB		= 45,
	MC_CLIENT_LAST		= -1,
};

struct tegra210_mc_client_info {
	enum mc_client	hot_reset_clients[MAX_HOTRESET_CLIENT_NUM];
};

static struct tegra210_mc_client_info tegra210_pg_mc_info[] = {
	[TEGRA_POWERGATE_CRAIL] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_LAST,
		},
	},
	[TEGRA_POWERGATE_VE] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_ISP2,
			[1] = MC_CLIENT_VI,
			[2] = MC_CLIENT_LAST,
		},
	},
#ifdef CONFIG_ARCH_TEGRA_HAS_PCIE
	[TEGRA_POWERGATE_PCIE] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_AFI,
			[1] = MC_CLIENT_LAST,
		},
	},
#endif
#ifdef CONFIG_ARCH_TEGRA_HAS_SATA
	[TEGRA_POWERGATE_SATA] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_SATA,
			[1] = MC_CLIENT_LAST,
		},
	},
#endif
	[TEGRA_POWERGATE_NVENC] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_NVENC,
			[1] = MC_CLIENT_LAST,
		},
	},
	[TEGRA_POWERGATE_SOR] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_LAST,
		},
	},
	[TEGRA_POWERGATE_DISA] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_DC,
			[1] = MC_CLIENT_LAST,
		},
	},
	[TEGRA_POWERGATE_DISB] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_DCB,
			[1] = MC_CLIENT_LAST,
		},
	},
	[TEGRA_POWERGATE_XUSBA] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_LAST,
		},
	},
	[TEGRA_POWERGATE_XUSBB] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_XUSB_DEV,
			[1] = MC_CLIENT_LAST,
		},
	},
	[TEGRA_POWERGATE_XUSBC] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_XUSB_HOST,
			[1] = MC_CLIENT_LAST,
		},
	},
#ifdef CONFIG_ARCH_TEGRA_VIC
	[TEGRA_POWERGATE_VIC] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_VIC,
			[1] = MC_CLIENT_LAST,
		},
	},
#endif
	[TEGRA_POWERGATE_NVDEC] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_NVDEC,
			[1] = MC_CLIENT_LAST,
		},
	},
	[TEGRA_POWERGATE_NVJPG] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_NVJPG,
			[1] = MC_CLIENT_LAST,
		},
	},
	[TEGRA_POWERGATE_APE] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_APE,
			[1] = MC_CLIENT_LAST,
		},
	},
	[TEGRA_POWERGATE_VE2] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_ISP2B,
			[1] = MC_CLIENT_LAST,
		},
	},
	[TEGRA_POWERGATE_GPU] = {
		.hot_reset_clients = {
			[0] = MC_CLIENT_GPU,
			[1] = MC_CLIENT_LAST,
		},
	},
};

static struct powergate_partition_info tegra210_pg_partition_info[] = {
	[TEGRA_POWERGATE_VE] = {
		.name = "ve",
		.clk_info = {
			[0] = { .clk_name = "ispa", .clk_type = CLK_AND_RST },
			[1] = { .clk_name = "vi", .clk_type = CLK_AND_RST },
			[2] = { .clk_name = "csi", .clk_type = CLK_AND_RST },
			[3] = { .clk_name = "vi_i2c", .clk_type = CLK_AND_RST },
			[4] = { .clk_name = "cilab", .clk_type = CLK_ONLY },
			[5] = { .clk_name = "cilcd", .clk_type = CLK_ONLY },
			[6] = { .clk_name = "cile", .clk_type = CLK_ONLY },
		},
	},
#ifdef CONFIG_ARCH_TEGRA_HAS_PCIE
	[TEGRA_POWERGATE_PCIE] = {
		.name = "pcie",
		.clk_info = {
			[0] = { .clk_name = "afi", .clk_type = CLK_AND_RST },
			[1] = { .clk_name = "pcie", .clk_type = CLK_AND_RST },
			[2] = { .clk_name = "cml0", .clk_type = CLK_ONLY },
			[3] = { .clk_name = "pciex", .clk_type = RST_ONLY },
		},
	},
#endif
#ifdef CONFIG_ARCH_TEGRA_HAS_SATA
	[TEGRA_POWERGATE_SATA] = {
		.name = "sata",
		.clk_info = {
			[0] = { .clk_name = "sata", .clk_type = CLK_AND_RST },
			[1] = { .clk_name = "sata_oob", .clk_type = CLK_AND_RST },
			[2] = { .clk_name = "cml1", .clk_type = CLK_ONLY },
			[3] = { .clk_name = "sata_cold", .clk_type = RST_ONLY },
		},
	},
#endif
	[TEGRA_POWERGATE_NVENC] = {
		.name = "nvenc",
		.clk_info = {
			[0] = { .clk_name = "msenc.cbus", .clk_type = CLK_AND_RST },
		},
	},
	[TEGRA_POWERGATE_SOR] = {
		.name = "sor",
		.clk_info = {
			[0] = { .clk_name = "sor0", .clk_type = CLK_AND_RST },
			[1] = { .clk_name = "dsia", .clk_type = CLK_AND_RST },
			[2] = { .clk_name = "dsib", .clk_type = CLK_AND_RST },
			[3] = { .clk_name = "hdmi", .clk_type = CLK_AND_RST },
			[4] = { .clk_name = "mipi-cal", .clk_type = CLK_AND_RST },
			[5] = { .clk_name = "dpaux", .clk_type = CLK_ONLY },
		},
	},
	[TEGRA_POWERGATE_DISA] = {
		.name = "disa",
		.clk_info = {
			[0] = { .clk_name = "disp1", .clk_type = CLK_AND_RST },
		},
	},
	[TEGRA_POWERGATE_DISB] = {
		.name = "disb",
		.clk_info = {
			[0] = { .clk_name = "disp2", .clk_type = CLK_AND_RST },
		},
	},
	[TEGRA_POWERGATE_XUSBA] = {
		.name = "xusba",
		.clk_info = {
			[0] = { .clk_name = "xusb_ss", .clk_type = CLK_AND_RST },
		},
	},
	[TEGRA_POWERGATE_XUSBB] = {
		.name = "xusbb",
		.clk_info = {
			[0] = { .clk_name = "xusb_dev", .clk_type = CLK_AND_RST },
		},
	},
	[TEGRA_POWERGATE_XUSBC] = {
		.name = "xusbc",
		.clk_info = {
			[0] = { .clk_name = "xusb_host", .clk_type = CLK_AND_RST },
		},
	},
#ifdef CONFIG_ARCH_TEGRA_VIC
	[TEGRA_POWERGATE_VIC] = {
		.name = "vic",
		.clk_info = {
			[0] = { .clk_name = "vic03.cbus", .clk_type = CLK_AND_RST },
		},
	},
#endif
	[TEGRA_POWERGATE_NVDEC] = {
		.name = "nvdec",
		.clk_info = {
			[0] = { .clk_name = "nvdec", .clk_type = CLK_AND_RST },
		},
	},
	[TEGRA_POWERGATE_NVJPG] = {
		.name = "nvjpg",
		.clk_info = {
			[0] = { .clk_name = "nvjpg", .clk_type = CLK_AND_RST },
		},
	},
	[TEGRA_POWERGATE_APE] = {
		.name = "ape",
		.clk_info = {
			[0] = { .clk_name = "ape", .clk_type = CLK_AND_RST },
		},
	},
	[TEGRA_POWERGATE_VE2] = {
		.name = "ve2",
		.clk_info = {
			[0] = { .clk_name = "ispb", .clk_type = CLK_AND_RST },
		},
	},
	[TEGRA_POWERGATE_GPU] = {
		.name = "gpu",
		.clk_info = {
			[0] = { .clk_name = "gpu_ref", .clk_type = CLK_AND_RST },
			[1] = { .clk_name = "pll_p_out5", .clk_type = CLK_ONLY },
		},
	},
};

struct mc_client_hotreset_reg {
	u32 control_reg;
	u32 status_reg;
};

static struct mc_client_hotreset_reg tegra210_mc_reg[] = {
	[0] = { .control_reg = 0x200, .status_reg = 0x204 },
	[1] = { .control_reg = 0x970, .status_reg = 0x974 },
};

#define PMC_GPU_RG_CONTROL		0x2d4

static DEFINE_SPINLOCK(tegra210_pg_lock);

static struct dvfs_rail *gpu_rail;

#define HOTRESET_READ_COUNTS		5

static bool tegra210_pg_hotreset_check(u32 status_reg, u32 *status)
{
	int i;
	u32 curr_status;
	u32 prev_status;
	unsigned long flags;

	spin_lock_irqsave(&tegra210_pg_lock, flags);
	prev_status = mc_read(status_reg);
	for (i = 0; i < HOTRESET_READ_COUNTS; i++) {
		curr_status = mc_read(status_reg);
		if (curr_status != prev_status) {
			spin_unlock_irqrestore(&tegra210_pg_lock, flags);
			return false;
		}
	}
	*status = curr_status;
	spin_unlock_irqrestore(&tegra210_pg_lock, flags);

	return 0;
}

static int tegra210_pg_mc_flush(int id)
{
	u32 idx, rst_control, rst_status;
	u32 rst_control_reg, rst_status_reg;
	enum mc_client mc_client_bit;
	unsigned long flags;
	bool ret;
	int reg_idx;

	for (idx = 0; idx < MAX_HOTRESET_CLIENT_NUM; idx++) {
		mc_client_bit = tegra210_pg_mc_info[id].hot_reset_clients[idx];
		if (mc_client_bit == MC_CLIENT_LAST)
			break;

		reg_idx = mc_client_bit / 32;
		mc_client_bit %= 32;
		rst_control_reg = tegra210_mc_reg[reg_idx].control_reg;
		rst_status_reg = tegra210_mc_reg[reg_idx].status_reg;

		spin_lock_irqsave(&tegra210_pg_lock, flags);
		rst_control = mc_read(rst_control_reg);
		rst_control |= (1 << mc_client_bit);
		mc_write(rst_control, rst_control_reg);
		spin_unlock_irqrestore(&tegra210_pg_lock, flags);

		do {
			udelay(10);
			rst_status = 0;
			ret = tegra210_pg_hotreset_check(rst_status_reg,
								&rst_status);
			if (!ret)
				continue;
		} while (!(rst_status & (1 << mc_client_bit)));
	}

	return 0;
}

static int tegra210_pg_mc_flush_done(int id)
{
	u32 idx, rst_control, rst_control_reg;
	enum mc_client mc_client_bit;
	unsigned long flags;
	int reg_idx;

	for (idx = 0; idx <= MAX_HOTRESET_CLIENT_NUM; idx++) {
		mc_client_bit = tegra210_pg_mc_info[id].hot_reset_clients[idx];
		if (mc_client_bit == MC_CLIENT_LAST)
			break;

		reg_idx = mc_client_bit / 32;
		rst_control_reg = tegra210_mc_reg[reg_idx].control_reg;

		spin_lock_irqsave(&tegra210_pg_lock, flags);
		rst_control = mc_read(rst_control_reg);
		rst_control &= ~(1 << mc_client_bit);
		mc_write(rst_control, rst_control_reg);
		spin_unlock_irqrestore(&tegra210_pg_lock, flags);

	}
	wmb();

	return 0;
}

static int tegra210_pg_powergate(int id)
{
	struct powergate_partition_info *partition =
				&tegra210_pg_partition_info[id];

	if (partition->refcount-- <= 0)
		return 0;

	if (!tegra_powergate_is_powered(id)) {
		WARN(1, "Partition %s already powergated, refcount \
				and status mismatch\n", partition->name);
		return 0;
	}

	return tegra1xx_powergate(id, partition);
}

static int tegra210_pg_unpowergate(int id)
{
	struct powergate_partition_info *partition =
				&tegra210_pg_partition_info[id];

	if (partition->refcount++ > 0)
		return 0;

	if (tegra_powergate_is_powered(id)) {
		WARN(1, "Partition %s is already unpowergated, refcount \
				and status mismatch\n", partition->name);
		return 0;
	}

	return tegra1xx_unpowergate(id, partition);
}

static int tegra210_pg_gpu_powergate(int id)
{
	int ret;
	struct powergate_partition_info *partition =
				&tegra210_pg_partition_info[id];

	if (partition->refcount-- <= 0)
		return 0;

	if (!tegra_powergate_is_powered(id)) {
		WARN(1, "GPU rail is already off, \
					refcount and status mismatch\n");
		return 0;
	}

	if (!partition->clk_info[0].clk_ptr)
		get_clk_info(partition);

	tegra_powergate_mc_flush(id);

	udelay(10);

	powergate_partition_assert_reset(partition);

	udelay(10);

	pmc_write(0x1, PMC_GPU_RG_CONTROL);

	udelay(10);

	partition_clk_disable(partition);

	udelay(10);

	if (gpu_rail) {
		ret = tegra_dvfs_rail_power_down(gpu_rail);
		if (ret) {
			WARN(1, "Could not power down GPU rail\n");
			return ret;
		}
	} else {
		pr_info("No GPU regulator?\n");
	}

	return 0;
}

static int tegra210_pg_gpu_unpowergate(int id)
{
	int ret = 0;
	bool first = false;
	struct powergate_partition_info *partition =
				&tegra210_pg_partition_info[id];

	if (partition->refcount++ > 0)
		return 0;

	if (!gpu_rail) {
		gpu_rail = tegra_dvfs_get_rail_by_name("vdd_gpu");
		if (IS_ERR_OR_NULL(gpu_rail)) {
			WARN(1, "No GPU regulator?\n");
			goto err_power;
		}
		first = true;
	} else {
		if (tegra_powergate_is_powered(id)) {
			WARN(1, "GPU rail is already on, \
					refcount and status mismatch\n");
			return 0;
		}

		ret = tegra_dvfs_rail_power_up(gpu_rail);
		if (ret)
			goto err_power;
	}

	if (!partition->clk_info[0].clk_ptr)
		get_clk_info(partition);

	if (!first) {
		ret = partition_clk_enable(partition);
		if (ret)
			goto err_clk_on;
	}

	udelay(10);

	powergate_partition_assert_reset(partition);

	pmc_write(0, PMC_GPU_RG_CONTROL);

	udelay(10);

	powergate_partition_deassert_reset(partition);

	udelay(10);

	tegra_powergate_mc_flush_done(id);

	udelay(10);

	return 0;

err_clk_on:
	powergate_module(id);
err_power:
	WARN(1, "Could not turn on GPU rail\n");
	return ret;
}

static int tegra210_pg_powergate_sor(int id)
{
	int ret;

	ret = tegra210_pg_powergate(id);
	if (ret)
		return ret;

	ret = tegra_powergate_partition(TEGRA_POWERGATE_SOR);
	if (ret)
		return ret;

	return 0;
}

static int tegra210_pg_unpowergate_sor(int id)
{
	int ret;

	ret = tegra_unpowergate_partition(TEGRA_POWERGATE_SOR);
	if (ret)
		return ret;

	ret = tegra210_pg_unpowergate(id);
	if (ret) {
		tegra_powergate_partition(TEGRA_POWERGATE_SOR);
		return ret;
	}

	return 0;
}

static int tegra210_pg_nvdec_powergate(int id)
{
	tegra210_pg_powergate(TEGRA_POWERGATE_NVDEC);
	tegra_powergate_partition(TEGRA_POWERGATE_NVJPG);

	return 0;
}

static int tegra210_pg_nvdec_unpowergate(int id)
{
	tegra_unpowergate_partition(TEGRA_POWERGATE_NVJPG);
	tegra210_pg_unpowergate(TEGRA_POWERGATE_NVDEC);

	return 0;
}

static int tegra210_pg_powergate_partition(int id)
{
	int ret;

	switch (id) {
		case TEGRA_POWERGATE_GPU:
			ret = tegra210_pg_gpu_powergate(id);
			break;
		case TEGRA_POWERGATE_DISA:
		case TEGRA_POWERGATE_DISB:
		case TEGRA_POWERGATE_VE:
			ret = tegra210_pg_powergate_sor(id);
			break;
		case TEGRA_POWERGATE_NVDEC:
			ret = tegra210_pg_nvdec_powergate(id);
			break;
		default:
			ret = tegra210_pg_powergate(id);
	}

	return ret;
}

static int tegra210_pg_unpowergate_partition(int id)
{
	int ret;

	switch (id) {
		case TEGRA_POWERGATE_GPU:
			ret = tegra210_pg_gpu_unpowergate(id);
			break;
		case TEGRA_POWERGATE_DISA:
		case TEGRA_POWERGATE_DISB:
		case TEGRA_POWERGATE_VE:
			ret = tegra210_pg_unpowergate_sor(id);
			break;
		case TEGRA_POWERGATE_NVDEC:
			ret = tegra210_pg_nvdec_unpowergate(id);
			break;
		default:
			ret = tegra210_pg_unpowergate(id);
	}

	return ret;
}

static int tegra210_pg_powergate_clk_off(int id)
{
	BUG_ON(TEGRA_IS_GPU_POWERGATE_ID(id));

	return tegraxx_powergate_partition_with_clk_off(id,
			&tegra210_pg_partition_info[id]);
}

static int tegra210_pg_unpowergate_clk_on(int id)
{
	BUG_ON(TEGRA_IS_GPU_POWERGATE_ID(id));

	return tegraxx_unpowergate_partition_with_clk_on(id,
			&tegra210_pg_partition_info[id]);
}

static const char *tegra210_pg_get_name(int id)
{
	return tegra210_pg_partition_info[id].name;
}

static spinlock_t *tegra210_pg_get_lock(void)
{
	return &tegra210_pg_lock;
}

static bool tegra210_pg_skip(int id)
{
	switch (id) {
#ifdef CONFIG_ARCH_TEGRA_HAS_SATA
		case TEGRA_POWERGATE_SATA:
			return true;
#endif
		default:
			return false;
	}
}

static bool tegra210_pg_is_powered(int id)
{
	u32 status = 0;

	if (TEGRA_IS_GPU_POWERGATE_ID(id)) {
		if (gpu_rail)
			status = tegra_dvfs_is_rail_up(gpu_rail);
	} else {
		status = pmc_read(PWRGATE_STATUS) & (1 << id);
	}

	return !!status;
}

static int tegra210_pg_init_refcount(void)
{
	int i;

	for (i = 0; i < TEGRA_NUM_POWERGATE; i++)
		if (tegra_powergate_is_powered(i))
			tegra210_pg_partition_info[i].refcount = 1;

	return 0;
}

static struct powergate_ops tegra210_pg_ops = {
	.soc_name = "tegra210",

	.num_powerdomains = TEGRA_NUM_POWERGATE,

	.get_powergate_lock = tegra210_pg_get_lock,
	.get_powergate_domain_name = tegra210_pg_get_name,

	.powergate_partition = tegra210_pg_powergate_partition,
	.unpowergate_partition = tegra210_pg_unpowergate_partition,

	.powergate_partition_with_clk_off = tegra210_pg_powergate_clk_off,
	.unpowergate_partition_with_clk_on = tegra210_pg_unpowergate_clk_on,

	.powergate_mc_flush = tegra210_pg_mc_flush,
	.powergate_mc_flush_done = tegra210_pg_mc_flush_done,

	.powergate_skip = tegra210_pg_skip,

	.powergate_is_powered = tegra210_pg_is_powered,

	.powergate_init_refcount = tegra210_pg_init_refcount,
};

struct powergate_ops *tegra210_powergate_init_chip_support(void)
{
	return &tegra210_pg_ops;
}