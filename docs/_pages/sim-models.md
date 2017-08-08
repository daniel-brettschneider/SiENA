---
title: Simulation Models
permalink: /sim-models
---

The following simulation models are included in SiENA and can be extended easily.

## Households

Households are seperated into three different parts, representing different amount of *smartness*.

### Conventional Household

* Basic behaviour without any optimisation
* Follows the behaviour of the device simulation models

### Smart Home

* Household with smart devices
* Household optimises itself

### Grid Home

* A smart home which can communicate with other smart homes via a communciation technology
* Performs distributed energy management using one of the algorithms

## Devices

Common household devices are already integrated into the baseload of a household. The remaining devices, which offer additional flexibility, are divided into three types. A DEM algorithm has access to these devices.

All parameters can be adjusted freely in the configuration.

### Shiftable Devices

* Activation time can be shifted to a later point in time
* Washing machine, tumble drier and dishwasher
* Parameters
  * Activation time (by resident)
  * Deadline (device must have finished its cycle until then)

### Adaptable Devices

* The energy consumption and generation can be adapted freely
* Parameters
  * Desired consumption
  * Minimum consumption (might be negative)
  * Maximum consumption (might be negative)
  * The DEM algorithm can choose freely in between
* Battery Storage
  * Charging power can be reduced freely or even reversed to feed into the power grid
  * Desires to be charged to 50% to provide flexibility
  * Parameters
    * Capacity
    * Efficiency
    * Power
* Electric Vehicle
  * Battery storage with occasional trips (charge is reduced after trip)
  * Requires a minimum 90% charge level to guarantee user acceptance
  * Minimum and maximum adaption depends on state
  * Parameters
    * Capacity
    * Fuel economy
* Heating Rod
  * Acts as a backup for heating devices below 15% charge level
  * Can be activated to consume surplus power
  * Parameters
    * Power

### Switchable Devices

* Can be switched *on* or *off*
* Heat pump
  * Heats a water tank
  * Desires a 50% to 75% charge
  * Has to run for at least 2000 hours a year
  * Has to run for at least 15 minutes before being switched off to reduce wear
  * Parameters
    * COP
    * Power
* micro CHP
  * Heats a water tank while generating power
  * Desires a 50% to 75% charge
  * Has to run for at least 5000 hours a year
  * Has to run for at least 120 minutes before being switched off to reduce wear
  * Parameters
    * COP
    * Electrical power
    * Heating power

![flex]({{ site.url }}{{ site.baseurl }}/assets/img/device_flexibilities.png)
