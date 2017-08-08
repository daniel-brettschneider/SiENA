---
title: Dependencies
permalink: /dependencies
---
SiENA has been developed as a module for ns-3, which offers an event-based simulation environment and extensive communication capabilities.
The following software is currently in use:

* ns-3.27  
  SiENA integrates into ns-3 as a module. All capabilities of ns-3 can be utilised. SiENA ships a full version of ns-3.27 to minimise installation issues. ns-3 itself requires some prerequisites, make sure to check the [installation instructions](/installation).
* [pandapower](https://www.uni-kassel.de/eecs/fachgebiete/e2n/software/pandapower.html)  
  Python-based power system analysis. Can be installed optionally to analyse power flow.
* [BRITE](https://www.cs.bu.edu/brite/)  
  Internet topology generator. Shipped with SiENA.
* [PLC](https://github.com/ns3-plc-module/plc)  
  ns-3 module for power line communication. Shipped with SiENA.
* [ggplot2](http://ggplot2.org/)  
  Used for plotting. Requires [R](https://www.r-project.org/).
* Linux with libjsoncpp  
  SiENA is intended to be run on a current version of Ubuntu. libjsoncpp is used to parse json files. Other operating systems might be used, but may require some adjustments.

<i class="fa fa-thumbs-o-up"></i> Thanks to all this open source software and their maintainers!
