/**
 ******************************************************************************
 * @file    imx_btsco.c
 * @version V0.2
 * @date    11-5-2018
 * @brief  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 ******************************************************************************
 ***/

#include <linux/module.h>
#include <linux/of_platform.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include "imx-audmux.h"

/*Bluetooth Module is the i2S master and imx6 is the i2s slave*/
#define IMX6_I2S_SLAVE

/*IMX6 is the i2s master and Bluetooth module is the i2s slave*/
//#define IMX6_I2S_MASTER

static int imx_audmux_config(int slave, int master)
{
	unsigned int ptcr, pdcr;
	slave --;
	master --;
	int ret;

	printk("%s\n",__func__);

	ptcr = IMX_AUDMUX_V2_PTCR_SYN |
		IMX_AUDMUX_V2_PTCR_TFSDIR |
		IMX_AUDMUX_V2_PTCR_TFSEL(slave) |
		IMX_AUDMUX_V2_PTCR_TCLKDIR |
		IMX_AUDMUX_V2_PTCR_TCSEL(slave);
	pdcr = IMX_AUDMUX_V2_PDCR_RXDSEL(slave);
	ret = imx_audmux_v2_configure_port(master, ptcr, pdcr);


	if (ret) {		
		printk("audmux ext_port setup failed\n");		
		return ret;	
	}
	

	ptcr = IMX_AUDMUX_V2_PTCR_SYN;
	pdcr = IMX_AUDMUX_V2_PDCR_RXDSEL(master);
	ret = imx_audmux_v2_configure_port(slave, ptcr, pdcr);
	if (ret) {		
		printk("audmux int_port setup failed\n");		
		return ret;	
	}
	return 0;
}

static int imx_btsco_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{

	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	u32 channels = params_channels(params);
	u32 rate = params_rate(params);

	/*Bitclock is currently selected as 512KHz Depends upon the External Clk*/
	u32 bclk = 512000;
	int ret = 0;
	printk("%s,channels=%d,rate=%d\n,bclk=%d\n",__func__,channels,rate,bclk);
	/*i.MX 6 is I2S Mater */


	/* Ensure That one time initialization of Registered CARD, otherwise Creates nosie in one channel*/
	if (substream->stream != SNDRV_PCM_STREAM_PLAYBACK) {
		return 0;

	}
#ifdef IMX6_I2S_MASTER
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S
			| SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
#endif

#ifdef IMX6_I2S_SLAVE
	/*IMx6 is I2S  salve*/
	ret = snd_soc_dai_set_fmt(cpu_dai,SND_SOC_DAIFMT_I2S 
			| SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM);
#endif

	if (ret) {
		dev_err(cpu_dai->dev, "failed to set dai fmt\n");
		return ret;
	}


	/*Configures DAI TDM operation */
	/*tx_mask = 0x1 one TX active Slot,rx_mask = 0x1 one RX active Slot, slot = 2 make DC=0001*/
	ret = snd_soc_dai_set_tdm_slot(cpu_dai,0x1,0x1,2, 16);
	if (ret) {
		dev_err(cpu_dai->dev, "failed to set dai tdm slot\n");
		return ret;
	}

	/*bclk External clock value, SND_SOC_CLOCK_IN Represent RX-CLK*/
	ret = snd_soc_dai_set_sysclk(cpu_dai, 0, bclk, SND_SOC_CLOCK_IN);

	if (ret)
		dev_err(cpu_dai->dev, "cpu dai failed to set sysclk\n");

	return ret;
}
static struct snd_soc_ops imx_btsco_ops = {
	.hw_params = imx_btsco_hw_params,
};

static struct snd_soc_dai_link imx_btsco_dai = {
	.name = "imx-btsco",
	.stream_name = "imx-btsco",
	.codec_dai_name = "btsco-codec",
	.ops = &imx_btsco_ops,

};

static struct snd_soc_card snd_soc_card_imx_btsco = {
	.name = "imx-audio-btsco",
	.dai_link = &imx_btsco_dai,
	.num_links = 1,


};

static int imx_btsco_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_card_imx_btsco;
	struct device_node *ssi_np, *np = pdev->dev.of_node;
	struct platform_device *ssi_pdev;

	struct device_node *btsco_np;

	int int_port, ext_port;
	int ret;

	ret = of_property_read_u32(np, "mux-int-port", &int_port);
	if (ret) {
		dev_err(&pdev->dev, "mux-int-port missing or invalid\n");
		return ret;
	}
	ret = of_property_read_u32(np, "mux-ext-port", &ext_port);
	if (ret) {
		dev_err(&pdev->dev, "mux-ext-port missing or invalid\n");
		return ret;
	}
#ifdef IMX6_I2S_MASTER
	/*IMX6 is the master */
	imx_audmux_config(int_port, ext_port);
#endif

#ifdef IMX6_I2S_SLAVE	
	/**IMX6 IS SALVE CONFIG*/
	imx_audmux_config(ext_port, int_port);
#endif
	ssi_np = of_parse_phandle(pdev->dev.of_node, "ssi-controller", 0);
	if (!ssi_np) {
		dev_err(&pdev->dev, "ssi-controller phandle missing or invalid\n");
		return -EINVAL;
	}

	ssi_pdev = of_find_device_by_node(ssi_np);
	if (!ssi_pdev) {
		dev_err(&pdev->dev, "failed to find SSI platform device\n");
		ret = -EINVAL;
		goto fail;
	}

	btsco_np = of_parse_phandle(pdev->dev.of_node, "btsco-controller", 0);
	if (!btsco_np) {
		dev_err(&pdev->dev, "btsco-controller phandle missing or invalid\n");
		ret = -EINVAL;
		goto fail;
	}

	card->dev = &pdev->dev;
	card->dai_link->cpu_dai_name = dev_name(&ssi_pdev->dev);
	card->dai_link->platform_of_node = ssi_np;
	card->dai_link->codec_of_node = btsco_np;
	ret = snd_soc_of_parse_card_name(card, "model");
	if (ret)		
		goto fail;

	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, &snd_soc_card_imx_btsco);

	ret = snd_soc_register_card(card);
	if (ret)
		dev_err(&pdev->dev, "Failed to register card: %d\n", ret);

fail:
	if (ssi_np)
		of_node_put(ssi_np);
	if (btsco_np)
		of_node_put(btsco_np);

	return ret;
}


static int imx_btsco_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_card_imx_btsco;
	snd_soc_unregister_card(card);
	return 0;
}


static const struct of_device_id imx_btsco_dt_ids[] = {
	{ .compatible = "fsl,imx-audio-btsco", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, imx_btsco_dt_ids);


static struct platform_driver imx_btsco_driver = {
	.driver = {
		.name = "audio-btsco",
		.owner = THIS_MODULE,
		.of_match_table = imx_btsco_dt_ids,
	},
	.probe = imx_btsco_probe,
	.remove = imx_btsco_remove,
};


module_platform_driver(imx_btsco_driver);

MODULE_ALIAS("platform:imx-audio-btsco");
MODULE_AUTHOR("vimal babu");
MODULE_LICENSE("GPL");
