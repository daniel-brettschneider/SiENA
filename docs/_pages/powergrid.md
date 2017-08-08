---
title: Power Grid
permalink: /powergrid
---
The power grid is simulated with pandapower. SiENA provides a generic street-based scenario, which is located at `data/pandapower`. Several street buses connect to a transformer.

After each simulation step, e.g. each minute, SiENA writes the consumption/generation to a CSV file in `data/pandapower`. Afterwards, it runs the stated scenario file, which performs a power flow analysis. The reported load levels of the cables and the transformer are written to the stats module and exported at the end.

![power]({{ site.url }}{{ site.baseurl }}/assets/img/pandapower.png)
