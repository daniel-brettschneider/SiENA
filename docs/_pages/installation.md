---
title: Installation
permalink: /installation
---
We provide a *Dockerfile* for easy installation and tryout. Just follow these steps:

1. Install [Docker](https://www.docker.com/)
2. Download the [Dockerfile](/assets/data/Dockerfile)
3. Create the siena container with `docker build -t siena /path/to/Dockerfile` (this may take some time, it installs all dependencies and loads the code)
4. Run the siena container with `docker run -ti siena` (you should now see a command prompt located at `SiENA/ns-3.26`)
5. Simulate
   * Edit `data/config/` (alternative algorithms etc. are stored in `data/config_unused`)
   * Run a simulation with `./waf --run siena`
   * Take a look at the results in `results/`

## Programming
If you want to extend the code of SiENA, you need to install a few more things related to ns-3. Take a look at their [installation instructions](https://www.nsnam.org/wiki/Installation).

Next, follow the steps of the Dockerfile:

```
FROM ubuntu

# install dependecies
RUN apt-get update && apt-get install -y gcc g++ build-essential python libxml2 libxml2-dev git libjsoncpp-dev libboost-all-dev r-base

# load r packages for plots
RUN R -e "install.packages(c('ggplot2','reshape','plyr'), repos='http://cran.rstudio.com/')"

# get code from github
RUN git clone https://github.com/daniel-brettschneider/SiENA.git

# build siena
RUN cd SiENA/BRITE && make
WORKDIR SiENA/ns-3.26
RUN ./waf-configure
RUN ./waf
```

Proceed at the [Developing New Modules](/develope) page.
