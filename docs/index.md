---
layout: single
author_profile: false
title: SiENA
gallery:
  - image_path: /assets/img/showcase_car_con.png
    alt: "battery consumption"
  - image_path: /assets/img/showcase_car_charge.png
    alt: "battery charge"
gallery2:
  - image_path: /assets/img/network_tree.png
    alt: "net"
  - image_path: /assets/img/showcase_com.png
    alt: "com"
gallery3:
  - image_path: /assets/img/showcase_powergrid.png
    alt: "grid"
  - image_path: /assets/img/showcase_neplan.png
    alt: "neplan"
---
## Simulator for Energy Network Application combining Power, Heat and Communication

**The goal of SiENA is to offer a dynamic approach to evaluate new DEM algorithms and applications in a realistic smart grid scenario.**
{: .notice--warning}

Want to take a quick look at some simulations? [Try it out](installation) with Docker!
{: .notice--info}

### Scope
1. Simulation of complex smart grid scenarios consisting of variable amounts of devices
2. A realistic data-basis of household appliances and future energy prosumers based on real-world measurements
3. Simulation of power, heat, and communication networks
4. Modular architecture for effortless integration of novel components and algorithms
5. Scalability for large scenarios
6. Automated evaluation of results
7. Provision of performance indicators of DSM

## Examples

### Consumption Plot
Average consumption of different device classes in a standard scenario with 100 households

![consumption]({{ site.url }}{{ site.baseurl }}/assets/img/showcase_consumption.png)
{: .full}

### Device Stats
Power consumption, generation and charge level of five battery storages in the simulation above

{% include gallery id="gallery" class="full" %}

### Household Types
Comparison of conventional, smart and grid-connected households

![types]({{ site.url }}{{ site.baseurl }}/assets/img/showcase_types.png)
{: .full}

### Device Stats
Sent and received data of all households and the server in one energy management interval grouped in one second steps

{% include gallery id="gallery2" class="full" %}

### Device Stats
Load level of power cables for 20 households with electric vehicles

{% include gallery id="gallery3" class="full" %}
