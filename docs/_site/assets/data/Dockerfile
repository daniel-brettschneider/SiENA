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
