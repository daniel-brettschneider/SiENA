---
title: PowerMatcher
permalink: /algo-pm
---

Kok, J. K., C. J. Warmer, and I. G. Kamphuis. “PowerMatcher: Multiagent Control in the Electricity Infrastructure.” In Proceedings of the Fourth International Joint Conference on Autonomous Agents and Multiagent Systems, 75–82. AAMAS ’05. New York, NY, USA: ACM, 2005. [doi:10.1145/1082473.1082807](doi:10.1145/1082473.1082807).
{: .notice}

PowerMatcher is a well-known method for supply demand matching. In PowerMatcher, households send bids to an auctioneer, who determines an energy price depending on the accumulated bids and a load target. The participating agents form a tree network. PowerMatcher is not a distributed system in terms of agents performing necessary calculations by themselves, like PrivADE or COHDA, but a distributed market-based control algorithm. However, it is still chosen as a comparison to PrivADE and COHDA due to its simple architecture and small communication footprint.

![powermatcher]({{ site.url }}{{ site.baseurl }}/assets/img/powermatcher.png)

Adaptable devices of a household are represented by device agents. Device agents are tasked with creating bids for the associated device and handing it over to the household which serves as an aggregator. Different device types and device states result in different bids. A device agent connected to a HP adapts the bid according to the charge level of an associated heat storage. If the charge level is high, the HP does not necessarily need to run, thus, high energy prices are not accepted. As opposed to the step function of a HP bid, a BS bid matches a monotonically decreasing function.

![bid]({{ site.url }}{{ site.baseurl }}/assets/img/bid.png)

A bid represents a set of coordinates with linear calculation in between as opposed to a function with a specific step size. Thus, the amount of data is reduced, because only key points need to be transmitted, for example, \{(0.2€; 2 kW), (0.2€; 0 kW\} for a HP.

PowerMatcher proceeds as follows:

1. Device agents of a household create bids. The household accumulates the bids of its device agents and sends it to the upper layer. The upper layer might consist of a concentrator or an auctioneer agent.
2. Concentrators accumulate all bids of their associated agents and forward the resulting bid.
3. Upon receiving all bids, the auctioneer agent calculates the intersection of all bids and the load target \\( \mu \\). A value on the ordinate of the intersection is the energy price to achieve \\( \mu \\). This price is sent back through the concentrators to all device agents.
4. The device agents receive the energy price and set their energy consumption accordingly.
