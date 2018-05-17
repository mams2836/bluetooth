 /**
  ******************************************************************************
  * @file    btsco-codec.c
  * @version V0.1
  * @date    25-4-2018
  * @brief  
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License version 2 as
  * published by the Free Software Foundation.
  ******************************************************************************
***/

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <sound/soc.h>
#include <sound/initval.h>

static const struct snd_soc_dapm_widget btsco_widgets[] = {
	SND_SOC_DAPM_INPUT("RX"),
	SND_SOC_DAPM_OUTPUT("TX"),
};

static const struct snd_soc_dapm_route btsco_routes[] = {
	{ "Capture", NULL, "RX" },
	{ "TX", NULL, "Playback" },
};

static int btsco_codec_set_dai_fmt(struct snd_soc_dai *codec_dai,
				    unsigned int fmt)
{
	printk("%s\n",__func__);
	return 0;
}

static int btsco_codec_hw_params(struct snd_pcm_substream *substream,
				  struct snd_pcm_hw_params *params,
				  struct snd_soc_dai *dai)
{
	printk("%s\n",__func__);

	return 0;
}
static int btsco_codec_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				  int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;


	return 0;
}
static unsigned int btsco_codec_read(struct snd_soc_codec *codec,
				      unsigned int reg)
{
	
	return 0;
}


static struct snd_soc_dai_ops btsco_dai_ops = {	
	.hw_params	= btsco_codec_hw_params,	
	.set_fmt	= btsco_codec_set_dai_fmt,
};

static int btsco_codec_probe(struct snd_soc_codec *codec)
{
	return 0;
}


static struct snd_soc_dai_driver btsco_dai = {
	.name		= "btsco-codec",
	.playback   = {
		.stream_name	= "Playback",
		.channels_min	= 1,
		.channels_max   = 1,
		.rates = SNDRV_PCM_RATE_8000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE, 
	},
	.capture	= {
		.stream_name	= "Capture",
		.channels_min	= 1,
		.channels_max   = 1,
		.rates = SNDRV_PCM_RATE_8000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
	.ops		= &btsco_dai_ops,
};


static struct snd_soc_codec_driver soc_codec_dev_btsco = {
	.dapm_widgets = btsco_widgets,
	.num_dapm_widgets = ARRAY_SIZE(btsco_widgets),
	.dapm_routes = btsco_routes,
	.num_dapm_routes = ARRAY_SIZE(btsco_routes),
};

static int btsco_platform_probe(struct platform_device *pdev)
{
	printk("%s\n",__func__);
	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_btsco,
				      &btsco_dai, 1);
}

static int btsco_platform_remove(struct platform_device *pdev)
{
	printk("%s\n",__func__);
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static const struct of_device_id btsco_codec_dt_ids[] = {
	{ .compatible = "btsco-codec", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, btsco_codec_dt_ids);


static struct platform_driver btsco_platform_driver = {
	.driver = {
		.name = "btsco-codec",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(btsco_codec_dt_ids),
	},
	.probe = btsco_platform_probe,
	.remove	= btsco_platform_remove,
};


module_platform_driver(btsco_platform_driver);

MODULE_DESCRIPTION("Bt Sco codec driver");
MODULE_AUTHOR("vimal babu");
MODULE_LICENSE("GPL");
