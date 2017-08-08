---
title: Data Basis
permalink: /data-basis
---
The data basis provides real-world measurements and statistical data for simulated device models. The data is available for the years 2011 to 2015.

## Overview

A lot of data and traces is integrated:

* Consumption traces of devices
* Device Configurations
* Car statistics
  * Trip length and duration
* Household and building statistics
  * Residents
  * Living space
  * Energy efficiency
* Weather data
* Heating demand
* PV traces
* ...and a lot more

For further details check out the source code or the publications.

## Device Configurations

* Basic parameter sets of real devices
* Battery storages, electric vehicles, heat pumps, micro CHPs
* A simulation model automatically initialises itself with a random config...
* ...or a specific config can be chosen in the scenario config
* Example: `Tesla Model S 60; capacity=60kWh; fuel economy=211.7Wh/km`

## Buildings

* Contain individual households
* Multi- or single-family
* Energy efficiency
* Living space and number of residents are initialised using the following distributions

![residents]({{ site.url }}{{ site.baseurl }}/assets/img/residents_distribution.png)

## Heat

* Minute-based heat demand is created for households
* Based on German technical standard VDI 4655
  * Heating, depends on
    * Energy efficiency of building
    * Living space
    * Season and weather
  * Warm water, depends on
    * Household type, multi or single
    * Number of residents
    * Season and weather

## Photovoltaic Systems

* Real traces of 27 PV systems are included
  * 2011 to 2015
  * German postal code area 48 and 49
* Size distribution from 30000 real PV systems, see below

![pv]({{ site.url }}{{ site.baseurl }}/assets/img/pv_size.png)

## Electric Vehicles

* Number of EVs depends on the households
* Statistical data for
  * Number and time of trips
  * Trip distance
  * Trip duration
