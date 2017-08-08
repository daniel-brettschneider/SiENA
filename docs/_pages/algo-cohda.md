---
title: COHDA
permalink: /algo-cohda
---

Hinrichs, Christian, Sebastian Lehnhoff, and Michael Sonnenschein. “COHDA: A Combinatorial Optimization Heuristic for Distributed Agents.” In Agents and Artificial Intelligence, edited by Joaquim Filipe and Ana Fred, 23–39. Communications in Computer and Information Science 449. Springer Berlin Heidelberg, 2013. [http://link.springer.com/chapter/10.1007/978-3-662-44440-5_2](http://link.springer.com/chapter/10.1007/978-3-662-44440-5_2).
{: .notice }

Combinatorial Optimization Heuristic for Distributed Agents (COHDA) resembles a multi agent system. Households are represented by an agent and connected through a small world overlay network. The optimisation heuristic pursues the global goal to adhere to the load target \\( \mu \\) by minimising \\( \vert\omega-\mu\vert \\).

![cohda]({{ site.url }}{{ site.baseurl }}/assets/img/cohda_net.png)

COHDA proceeds as follows:

1. The server \\( s \\) sends the adaption target \\( \nu=\mu/\vert H \vert \\) to a random household \\( h \\).
2. Every household \\( h \\) maintains a knowledge base, consisting of the current configuration \\( \sum_h \\) (energy consumption of all households) of the system known to \\( h \\) and the best configuration \\( \sum_h^* \\).
3. Upon receiving information from a neighbour, a household updates its knowledge base. The household now chooses a configuration of its own devices, which corresponds to its own goals and considers \\( \sum_h \\). If \\( \sum_h^* \\) cannot be improved according to \\( \nu \\), \\( \sum_h^* \\) is chosen and \\( h \\) determines the states of its devices.
4. If \\( h \\) finds a better solution \\( \sum'_h \\), it publishes this new information. Thus, \\( h \\) updates \\( \sum_h \\) and \\( \sum_h^* \\) to \\( \sum'_h \\) and sends this data to its neighbours.
