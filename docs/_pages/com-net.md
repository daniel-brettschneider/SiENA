---
title: Communication Networks
permalink: /com-net
---
SiENA provides four different communication networks out of the box.

1. Wired broadband (DSL-like)
2. LTE
3. PLC
4. BRITE

Households and servers communicate with each other via the communication network. A communication link is an IP-based socket, based on UDP or TCP.

General Parameters:
* Network
* TCP or UDP
* Duplicate package check

## Wired broadband

![tree]({{ site.url }}{{ site.baseurl }}/assets/img/network_tree.png)
{: .half }

The wired broadband communication network resembles a tree topology. Households and servers are connected in random groups to the bottom layer of a tree of routers.

Parameters:
* Cluster size: 10
* Household/server data rate: 5 Mbps
* Household/server delay: 20 ms
* Router data rate: 1 Gbps
* Router delay: 2 ms
* Tree layers: 3, between root node and households

## LTE

![tree]({{ site.url }}{{ site.baseurl }}/assets/img/simulator_lte_topologie.png)
{: .half }

In the LTE topology households and servers are placed randomly in groups around a LTE mast.

Parameters:
* Nodes per enb node: 250
* Max distance to enb node: 500 m
* Distance between enb nodes: 1000000 m
* Delay min: 5000 ms
* Delay max: 10000 ms
* Minimum random placement angle: 0
* Maximum random placement angle: 3.1415
* random function: SQRT
* Download: 100
* Upload: 100

## PLC

![tree]({{ site.url }}{{ site.baseurl }}/assets/img/simulator_plc_topologie.png)
{: .half }

The PLC topology resembles a main street with access roads. The plc-module offers many parameters, which are initialised corresponding to the PRIME standard, which offers 128.6 kbps in the CENELEC-A band.

Parameters:
* Residents in a main street: 100
* Residents in an access road: 10
* A lot of stuff related to PRIME

## BRITE

![tree]({{ site.url }}{{ site.baseurl }}/assets/img/BRITE.png)
{: .half }

The BRITE topology creates a large representative topology of the Internet. Households and servers are connected to outer nodes. A standard config file of the BRITE tool can be used for configuration.
