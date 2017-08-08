msg.trap <- capture.output(suppressMessages(library(ggplot2)))
msg.trap <- capture.output(suppressMessages(library(reshape)))

args <- commandArgs(trailingOnly = TRUE)
id <- args[1]
interval <- as.integer(args[2])

data <- read.csv(paste("results/", id, "_powergrid.csv", sep=""), fileEncoding="latin1")

duration <- dim(data)[1]
labels <- seq(0, duration*interval / 60, 2) %% 24
breaks <- seq(0,duration,120/interval)
pdfSize <- 7 + round(duration*interval/1440) * 5

# load
df <- data.frame(x=c(0:(duration-1)), data[,grep("line|trafo", colnames(data))])
df <- melt(df, id="x")
p <- ggplot() + xlab("time of day / h") + ylab("load level / %") +
  geom_step(data=df, aes(x=x, y=value, colour=variable)) +
  scale_x_continuous(expand=c(0,0), breaks=breaks, labels=labels) +
  theme_bw() +
  theme(legend.position="none")
pdf(file="results/powergrid_load.pdf", width=pdfSize, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))

# voltage
#df <- data.frame(x=c(0:(duration-1)), data[,grep("vm", colnames(data))])
#df <- data.frame(x=c(0:(duration-1)), data$bus_street_1_house_50_vm * 100 - 100)
df <- data.frame(x=c(0:(duration-1)), data$bus_trans_vm * 100 - 100)
df <- melt(df, id="x")
p <- ggplot() + xlab("time of day / h") + ylab("voltage deviation / %") +
  geom_step(data=df, aes(x=x, y=value, colour=variable)) +
  geom_hline(yintercept=10, linetype="dashed") +
  geom_hline(yintercept=-10, linetype="dashed") +
  scale_x_continuous(expand=c(0,0), breaks=breaks, labels=labels) +
  theme_bw() +
  theme(legend.position="none")
pdf(file="results/powergrid_voltage.pdf", width=pdfSize, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))