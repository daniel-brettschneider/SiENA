---
title: Developing New Features
permalink: /develope
---
If you want to implement new features, simulation models or algorithms in SiENA, please read this manual. SiENA is fairly complex, but there is a good chance that you only need to change small parts of it.

## Overview
Before continuing with a detailed description of how to integrate new functionality, here is an overview of all important stuff.

1. If you have any remaining questions after reading this manual, please ask the maintainers. They will try to answer and update this manual.
2. Read the [installation](/installation) instructions carefully
3. Make yourselve familiar with ns-3, there is a chance that your *problem* might be related to ns-3, it's a complex software
4. Important files and folders
   1. `ns-3.27/src/siena` The source of SiENA inside ns-3; the folder structure should be self-explanatory
   2. `ns-3.27/data` Config files and data basis
   3. `ns-3.27/scratch/siena.cc` The script to start SiENA
   4. `ns-3.27/src/siena/wscript` Tells ns-3 about the module; update with `ns-3.27/tools/initProject.py`
   5. `ns-3.27/waf-configure` Init ns-3; e.g. set debug or optimized mode
5. Changing algorithms, etc. with the Registry
   1. Register your generic function in `siena/modules/Registry.h` with an unique *name*
   2. Use *name* in the config; e.g. `PrivADE_Plus.json` <i class="fa fa-angle-double-right"></i> `"server_handle_token":"my_fancy_function"`

## Eclipse
ns-3 and SiENA are intended to be used with Eclipse. Install the C++ version of Eclipse. Clone the github repository and add the project to Eclipse. It is already an Eclipse project which is indicated by the wizard. If you name the project *siena*, the building and running the project will already work. Otherwise:

* Project Properties
  * C/C++ Build
    * Builder Settings
      * Build command: ${workspace_loc:siena}/ns-3.27/waf
      * Generate Makefiles automatically: *No*
      * Build directory: ${workspace_loc:siena}/ns-3.27/build
    * Refresh Policy: Add *siena/ns-3.27*
* Run Configuration
  * Main
    * C/C++ Application: ns-3.27/build/scratch/siena
  * Arguments
    * Working directory: ${workspace_loc:siena/ns-3.27/}
  * Environment
    * Add `LD_LIBRARY_PATH` -  `${workspace_loc:siena/ns-3.27/build};${workspace_loc:siena/BRITE}`
