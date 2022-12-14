/*
 * pwm-bcm2835 driver
 * Standard raspberry pi (gpio18 - pwm0)
 *
 * Copyright (C) 2014 Thomas more
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2.
 */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>

/*mmio regiser mapping*/

#define DUTY		0x14
#define PERIOD		0x10
#define CHANNEL		0x10

#define PWM_ENABLE	0x00000001
#define PWM_POLARITY	0x00000010

#define MASK_CTL_PWM	0x000000FF
#define CTL_PWM		0x00000081

#define DRIVER_AUTHOR "Bart Tanghe <bart.tanghe@thomasmore.be>"
#define DRIVER_DESC "A bcm2835 pwm driver - raspberry pi development platform"

struct bcm2835_pwm_chip {
	struct pwm_chip chip;
	struct device *dev;
	int channel;
	int scaler;
	void __iomem *mmio_base;
};

static inline struct bcm2835_pwm_chip *to_bcm2835_pwm_chip(
					struct pwm_chip *chip){

	return container_of(chip, struct bcm2835_pwm_chip, chip);
}

static int bcm2835_pwm_config(struct pwm_chip *chip,
			      struct pwm_device *pwm,
			      int duty_ns, int period_ns){

	struct bcm2835_pwm_chip *pc;

	pc = container_of(chip, struct bcm2835_pwm_chip, chip);

	iowrite32(duty_ns/pc->scaler, pc->mmio_base + DUTY);
	iowrite32(period_ns/pc->scaler, pc->mmio_base + PERIOD);

	return 0;
}

static int bcm2835_pwm_enable(struct pwm_chip *chip,
			      struct pwm_device *pwm){

	struct bcm2835_pwm_chip *pc;

	pc = container_of(chip, struct bcm2835_pwm_chip, chip);

	iowrite32(ioread32(pc->mmio_base) | PWM_ENABLE, pc->mmio_base);
	return 0;
}

static void bcm2835_pwm_disable(struct pwm_chip *chip,
				struct pwm_device *pwm)
{
	struct bcm2835_pwm_chip *pc;

	pc = to_bcm2835_pwm_chip(chip);

	iowrite32(ioread32(pc->mmio_base) & ~PWM_ENABLE, pc->mmio_base);
}

static int bcm2835_set_polarity(struct pwm_chip *chip, struct pwm_device *pwm,
				enum pwm_polarity polarity)
{
	struct bcm2835_pwm_chip *pc;

	pc = to_bcm2835_pwm_chip(chip);

	if (polarity == PWM_POLARITY_NORMAL)
		iowrite32((ioread32(pc->mmio_base) & ~PWM_POLARITY),
						pc->mmio_base);
	else if (polarity == PWM_POLARITY_INVERSED)
		iowrite32((ioread32(pc->mmio_base) | PWM_POLARITY),
						pc->mmio_base);

	return 0;
}

static const struct pwm_ops bcm2835_pwm_ops = {
	.config = bcm2835_pwm_config,
	.enable = bcm2835_pwm_enable,
	.disable = bcm2835_pwm_disable,
	.set_polarity = bcm2835_set_polarity,
	.owner = THIS_MODULE,
};

static int bcm2835_pwm_probe(struct platform_device *pdev)
{
	struct bcm2835_pwm_chip *pwm;

	int ret;
	struct resource *r;
	u32 start, end;
	struct clk *clk;

	pwm = devm_kzalloc(&pdev->dev, sizeof(*pwm), GFP_KERNEL);
	if (!pwm) {
		dev_err(&pdev->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	pwm->dev = &pdev->dev;

	clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "could not find clk: %ld\n", PTR_ERR(clk));
		devm_kfree(&pdev->dev, pwm);
		return PTR_ERR(clk);
	}

	pwm->scaler = (int)1000000000/clk_get_rate(clk);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	pwm->mmio_base = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(pwm->mmio_base))
	{
		devm_kfree(&pdev->dev, pwm);
		return PTR_ERR(pwm->mmio_base);
	}

	start = r->start;
	end = r->end;

	pwm->chip.dev = &pdev->dev;
	pwm->chip.ops = &bcm2835_pwm_ops;
	pwm->chip.npwm = 2;

	ret = pwmchip_add(&pwm->chip);
	if (ret < 0) {
		dev_err(&pdev->dev, "pwmchip_add() failed: %d\n", ret);
		devm_kfree(&pdev->dev, pwm);
		return -1;
	}

	/*set the pwm0 configuration*/
	iowrite32((ioread32(pwm->mmio_base) & ~MASK_CTL_PWM)
				| CTL_PWM, pwm->mmio_base);

	platform_set_drvdata(pdev, pwm);

	return 0;
}

static int bcm2835_pwm_remove(struct platform_device *pdev)
{

	struct bcm2835_pwm_chip *pc;
	pc  = platform_get_drvdata(pdev);

	if (WARN_ON(!pc))
		return -ENODEV;

	return pwmchip_remove(&pc->chip);
}

static const struct of_device_id bcm2835_pwm_of_match[] = {
	{ .compatible = "bcrm,pwm-bcm2835", },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, bcm2835_pwm_of_match);

static struct platform_driver bcm2835_pwm_driver = {
	.driver = {
		.name = "pwm-bcm2835",
		.owner = THIS_MODULE,
		.of_match_table = bcm2835_pwm_of_match,
	},
	.probe = bcm2835_pwm_probe,
	.remove = bcm2835_pwm_remove,
};
module_platform_driver(bcm2835_pwm_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
========================================================
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h> 
#include <linux/init.h>
#include <linux/ioctl.h>
#include <mach/platform.h>



#define CLASS_NAME "PWM_BUZZER"

#define PWM_BASE 	(BCM2708_PERI_BASE + 0x20C000)

#define RNG1		0x10
#define DAT1		0x14
#define RNG2		0x20
#define DAT2		0x24
#define PWM0_ENABLE	0x00000001
#define PWM1_ENABLE	0x00000100
#define MASK_CTL_PWM0	0x000000FF
#define MASK_CTL_PWM1	0x0000FF00

#define PWM_CH0 0
#define PWM_CH1 1

// 이중에서 clock 설정안했다고 함.

//20
#define SCALE  (1/100000000)

/*-GPIO-*/
#define GPFSEL1 (0x04)

#define DEBUG

static dev_t majorminor;
static struct class *pwm_class = NULL;

typedef struct{
	int ch;
	dev_t majorminor;
	struct cdev c_dev;
} pwm_ch;

static void* pwm_base = NULL;
static void* gpio_base = NULL;
pwm_ch *pwm_ch_vec;

static void change_duty(unsigned long duty,pwm_ch* pwmch);
static void change_period(unsigned long period,pwm_ch* pwmch);
static void UpdatePWMStatus(unsigned int en,pwm_ch* pwmch);

static long my_ioctl(struct file *fp, unsigned int cmd, unsigned long arg){

	pwm_ch * pwmch = fp->private_data;	
	unsigned int temp;

	switch(cmd){
		case CHANGE_PERIOD:
			//temp = 1.0/(float)arg; 	//Get period (ns) of given frequency
			//temp = (float)temp/SCALE;	//Scale for clock ticks (max. granularity)
			temp = 30000;
			change_period(temp,pwmch); 
			break;
		case CHANGE_DUTY:
			temp = ioread32(pwm_base + RNG1);
			//temp = ((float)arg/100.0)*temp;
			temp = 100000;
			change_duty(temp,pwmch); 
			break;
		case ENABLE:
			UpdatePWMStatus(ENABLE,pwmch);
			break;
		case DISABLE:
			UpdatePWMStatus(DISABLE,pwmch);
	}

	return 0;
}

static void change_duty(unsigned long duty,pwm_ch* pwmch){
	if(pwmch->ch == PWM_CH0){
		iowrite32(duty, pwm_base + DAT1);
	#ifdef DEBUG
		printk (KERN_INFO "Duty Changed for pwm0\n");
	#endif
	}
	else{
		iowrite32(duty, pwm_base + DAT2);
	#ifdef DEBUG
		printk (KERN_INFO "Duty Changed for pwm1\n");
	#endif
	}
}

static void change_period(unsigned long period,pwm_ch* pwmch){
	if(pwmch->ch == PWM_CH0){
		iowrite32(period, pwm_base + RNG1);
	#ifdef DEBUG
		printk (KERN_INFO "Period Changed for pwm0\n");
	#endif
	}
	else{
		iowrite32(period, pwm_base + RNG2);
	#ifdef DEBUG
		printk (KERN_INFO "Period Changed for pwm1\n");
	#endif
	}
}

static void UpdatePWMStatus(unsigned int en,pwm_ch* pwmch){
	if(en == ENABLE){
		if(pwmch->ch == PWM_CH0){
			iowrite32(ioread32(pwm_base) | PWM0_ENABLE, pwm_base);
		#ifdef DEBUG
			printk (KERN_INFO "pwm0 Enabed\n");
		#endif
		}
		else{
			iowrite32(ioread32(pwm_base) | PWM1_ENABLE, pwm_base);
		#ifdef DEBUG
			printk (KERN_INFO "pwm0 Enabed\n");
		#endif
		}
	}
	else{
		if(pwmch->ch == PWM_CH0){
			iowrite32(ioread32(pwm_base) & ~PWM0_ENABLE, pwm_base);
		#ifdef DEBUG
			printk (KERN_INFO "pwm0 Disabled\n");
		#endif
		}
		else{
			iowrite32(ioread32(pwm_base) & ~PWM1_ENABLE, pwm_base);
		#ifdef DEBUG
			printk (KERN_INFO "pwm0 Disabled\n");
		#endif
		}
	}
}
static int my_open(struct inode *node, struct file *file){
	/*init GPIO*/
	int temp;
	pwm_ch *my_dev;

    	my_dev = container_of(node->i_cdev, pwm_ch, c_dev);
    	file->private_data = my_dev;
	
	/* Configure pins as alternate function 0 for pwm */
	if(my_dev->ch == PWM_CH0){
		temp = ioread32(gpio_base + GPFSEL1);
		temp &= ~(3 << (2*3)); //Clear last bit
		temp |= (4 << (2*3)); 
		iowrite32(temp, gpio_base  + GPFSEL1);
	#ifdef DEBUG
		printk (KERN_INFO "Pin 12 Configured for AT0 for pwm0\n");
	#endif
	}	
	else{
		temp = ioread32(gpio_base + GPFSEL1);
		temp &= ~(4 << 3);
		iowrite32(temp, gpio_base  + GPFSEL1);
	#ifdef DEBUG
		printk (KERN_INFO "Pin 13 Configured for AT0 for pwm1\n");
	#endif
	}

	return 0;
}

static int my_release(struct inode *node, struct file *file){

	pwm_ch *pwmch = file->private_data;

	if(pwmch->ch == PWM_CH0){ //clear PWM0 to reset state
		iowrite32((ioread32(pwm_base) & ~MASK_CTL_PWM0)	
			, pwm_base); 
		#ifdef DEBUG
			printk (KERN_INFO "pwm0 to zeros\n");
		#endif
	}
	else{			//clear PWM1 to reset state
		iowrite32((ioread32(pwm_base) & ~MASK_CTL_PWM1)
			, pwm_base);
		#ifdef DEBUG
			printk (KERN_INFO "pwm0 to zeros\n");
		#endif
	}
	return 0;
	printk (KERN_INFO "Exiting...");
}

static struct file_operations fops = {

	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_release,
	.unlocked_ioctl = my_ioctl

};

static __init int my_init(void){

	int i, ret = 0;
	dev_t tempmajorminor;	
	char device_name[8];

	if((ret = alloc_chrdev_region(&majorminor, 0, 2, CLASS_NAME)) < 0){
		printk(KERN_INFO "Failed allocating major...\n");
		return ret;
	}

	if(IS_ERR(pwm_class = class_create(THIS_MODULE, CLASS_NAME))){
		printk(KERN_INFO "Failed creating class..\n");
		ret = PTR_ERR(pwm_class);
		return -1;
	}

	pwm_ch_vec = (pwm_ch*) kmalloc(2 * sizeof(pwm_ch), GFP_KERNEL);
	if(!pwm_ch_vec){
		ret = ENOMEM;
		return -1;
	}
	memset(pwm_ch_vec, 0, 2 * sizeof(pwm_ch));

	for(i=0;i<2;i++){
		snprintf(device_name, 11, "pwm%d",i);
		
		tempmajorminor = MKDEV(MAJOR(majorminor), i );

		if(IS_ERR(device_create(pwm_class, NULL, tempmajorminor, NULL, device_name))){
			printk(KERN_INFO "Failed creating device...\n");
			ret = PTR_ERR(pwm_class);
			return -1;
		}

		cdev_init(&(pwm_ch_vec[i].c_dev), &fops);

		pwm_ch_vec[i].c_dev.owner = THIS_MODULE;
		pwm_ch_vec[i].c_dev.ops = &fops;
		pwm_ch_vec[i].ch = i;
		if((ret = (cdev_add(&(pwm_ch_vec[i].c_dev), tempmajorminor, 1))) < 0){
			printk(KERN_INFO "Failed register device...\n");
			return -1;
		}
	}

	if((pwm_base = ioremap(PWM_BASE, 0x60)) == NULL){
		printk(KERN_INFO "Failed mapping pwm registers...\n");
		ret = -1;
		return -1;
	}

	if((gpio_base = ioremap(GPIO_BASE, 0x60)) == NULL){
		printk(KERN_INFO "Failed mapping gpio registers...\n");
		ret = -1;
		return -1;
	}
	
	printk(KERN_INFO "Sucess...\n");
	return 0;
}

static __exit void my_exit(void){
	//Everything deactivated
	iowrite32(0, pwm_base);	
}


module_init(my_init);
module_exit(my_exit);