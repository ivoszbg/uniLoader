/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2025, Ivaylo Ivanov <ivo.ivanov.ivanov1@gmail.com>
 */

#ifndef __S2MPS_25_26_PMIC_H__
#define __S2MPS_25_26_PMIC_H__

/* SPMI ID allocation */
#define MAIN_PMIC_ID		0
#define SUB_PMIC_ID		1
#define RF_PMIC_ID		2

/* S2MPS25 slave address */
#define S2MPS25_VGPIO_ADDR		0x0
#define S2MPS25_COMMON_ADDR		0x3
#define S2MPS25_RTC_ADDR		0x4
#define S2MPS25_PM1_ADDR		0x5
#define S2MPS25_PM2_ADDR		0x6
#define S2MPS25_CLOSE1_ADDR		0xE
#define S2MPS25_CLOSE2_ADDR		0xF
#define S2MPS25_GPADC_ADDR		0xA

/* S2MPS26 slave address */
#define S2MPS26_VGPIO_ADDR		0x0
#define S2MPS26_COMMON_ADDR		0x3
#define S2MPS26_PM1_ADDR		0x5
#define S2MPS26_PM2_ADDR		0x6
#define S2MPS26_CLOSE1_ADDR		0xE
#define S2MPS26_CLOSE2_ADDR		0xF
#define S2MPS26_GPIO_ADDR		0xB

/* S2MPM07 slave address */
#define S2MPM07_COMMON_ADDR		0x3
#define S2MPM07_PM1_ADDR		0x5
#define S2MPM07_CLOSE1_ADDR		0xE

/* S2MPS25 COMMON Address */
#define S2MPS25_REG_CHIP_ID             0x0E

#define S2MPS25_REG_INT1		0x00
#define S2MPS25_REG_INT2		0x01
#define S2MPS25_REG_PWRONSRC1		0x14
#define S2MPS25_REG_TX_MASK		0x16

/* S2MPS25 Register(PM1) Address */
#define S2MPS25_REG_RTC_BUF             0x1D
#define S2MPS25_REG_CTRL1               0x1E
#define S2MPS25_REG_CTRL2               0x1F
#define S2MPS25_REG_CTRL3               0x20
#define S2MPS25_REG_ETC_OTP1		0x21

#define S2MPS25_REG_BUCK1M_CTRL         0x26
#define S2MPS25_REG_BUCK2M_CTRL         0x32
#define S2MPS25_REG_BUCK3M_CTRL         0x3E
#define S2MPS25_REG_BUCK4M_CTRL         0x4A
#define S2MPS25_REG_BUCK5M_CTRL         0x56
#define S2MPS25_REG_BUCK6M_CTRL         0x62
#define S2MPS25_REG_BUCK7M_CTRL         0x6E
#define S2MPS25_REG_BUCK8M_CTRL         0x7A
#define S2MPS25_REG_BUCK9M_CTRL         0x86
#define S2MPS25_REG_BUCK10M_CTRL        0x92
#define S2MPS25_REG_BUCK_SR1M_CTRL      0x9E
#define S2MPS25_REG_BUCK_SR1M_OUT1	0x9F
#define S2MPS25_REG_BUCK_SR1M_OUT2	0xA0
#define S2MPS25_REG_BUCK_SR2M_CTRL      0xA3
#define S2MPS25_REG_BUCK_SR3M_CTRL      0xA8
#define S2MPS25_REG_BUCK_SR3M_OUT1	0xA9
#define S2MPS25_REG_BUCK_SR3M_OUT2	0xAA
#define S2MPS25_REG_LDO1M_CTRL          0xAD
#define S2MPS25_REG_LDO2M_CTRL          0xAF
#define S2MPS25_REG_LDO3M_CTRL          0xB1
#define S2MPS25_REG_LDO4M_CTRL          0xB2
#define S2MPS25_REG_LDO5M_CTRL          0xB4
#define S2MPS25_REG_LDO6M_CTRL          0xB6
#define S2MPS25_REG_LDO7M_CTRL          0xB8
#define S2MPS25_REG_LDO8M_CTRL          0xBA
#define S2MPS25_REG_LDO9M_CTRL          0xBC
#define S2MPS25_REG_LDO10M_CTRL         0xBE
#define S2MPS25_REG_LDO11M_CTRL         0xC0
#define S2MPS25_REG_LDO12M_CTRL         0xC2
#define S2MPS25_REG_LDO13M_CTRL         0xC3
#define S2MPS25_REG_LDO14M_CTRL         0xC4
#define S2MPS25_REG_LDO15M_CTRL         0xC6
#define S2MPS25_REG_LDO16M_CTRL         0xC7
#define S2MPS25_REG_LDO17M_CTRL         0xC8
#define S2MPS25_REG_LDO18M_CTRL         0xC9
#define S2MPS25_REG_LDO19M_CTRL         0xCA
#define S2MPS25_REG_LDO20M_CTRL         0xCC
#define S2MPS25_REG_LDO21M_CTRL         0xCD
#define S2MPS25_REG_LDO22M_CTRL         0xCE
#define S2MPS25_REG_LDO23M_CTRL         0xCF
#define S2MPS25_REG_LDO24M_CTRL         0xD0
#define S2MPS25_REG_LDO25M_CTRL         0xD1
#define S2MPS25_REG_LDO26M_CTRL         0xD2
#define S2MPS25_REG_LDO27M_CTRL         0xD3
#define S2MPS25_REG_LDO28M_CTRL         0xD4
#define S2MPS25_REG_LDO29M_CTRL         0xD5
#define S2MPS25_REG_LDO30M_CTRL         0xD6
#define S2MPS25_REG_LDO_DSCH3		0xD9

/* S2MPS26 COMMON Address */
#define S2MPS26_REG_CHIP_ID             0x0E

/* S2MPS26 Register(PM1) Address */
#define S2MPS26_REG_CTRL1               0x17
#define S2MPS26_REG_CTRL3               0x19

#define S2MPS26_REG_BUCK1S_CTRL         0x1F
#define S2MPS26_REG_BUCK2S_CTRL         0x2B
#define S2MPS26_REG_BUCK3S_CTRL         0x37
#define S2MPS26_REG_BUCK4S_CTRL         0x43
#define S2MPS26_REG_BUCK5S_CTRL         0x4F
#define S2MPS26_REG_BUCK6S_CTRL         0x5B
#define S2MPS26_REG_BUCK7S_CTRL         0x67
#define S2MPS26_REG_BUCK8S_CTRL         0x73
#define S2MPS26_REG_BUCK9S_CTRL         0x7F
#define S2MPS26_REG_BUCK10S_CTRL        0x8B
#define S2MPS26_REG_BUCK11S_CTRL        0x97
#define S2MPS26_REG_BUCK12S_CTRL        0xA3
#define S2MPS26_REG_SR1S_CTRL           0xAF
#define S2MPS26_REG_SR1S_OUT1		0xB0
#define S2MPS26_REG_SR1S_OUT2		0xB1
#define S2MPS26_REG_BB_CTRL             0xB4

#define S2MPS26_REG_LDO1S_CTRL          0xC6
#define S2MPS26_REG_LDO2S_CTRL          0xC8
#define S2MPS26_REG_LDO3S_CTRL          0xCD
#define S2MPS26_REG_LDO4S_CTRL          0xCF
#define S2MPS26_REG_LDO5S_CTRL          0xD1
#define S2MPS26_REG_LDO6S_CTRL          0xD3
#define S2MPS26_REG_LDO7S_CTRL          0xD8
#define S2MPS26_REG_LDO8S_CTRL          0xDA
#define S2MPS26_REG_LDO9S_CTRL          0xDC
#define S2MPS26_REG_LDO10S_CTRL         0xE1
#define S2MPS26_REG_LDO11S_CTRL         0xE3
#define S2MPS26_REG_LDO12S_CTRL         0xE5
#define S2MPS26_REG_LDO13S_CTRL         0xE7
#define S2MPS26_REG_LDO14S_CTRL         0xE9
#define S2MPS26_REG_LDO15S_CTRL         0xEA
#define S2MPS26_REG_LDO16S_CTRL         0xEB
#define S2MPS26_REG_LDO17S_CTRL         0xEC
#define S2MPS26_REG_LDO18S_CTRL         0xED
#define S2MPS26_REG_LDO19S_CTRL         0xEE
#define S2MPS26_REG_LDO20S_CTRL         0xEF

#endif /*__S2MPS_25_26_PMIC_H__*/
