msg.trap <- capture.output(suppressMessages(library(ggplot2)))
msg.trap <- capture.output(suppressMessages(library(reshape)))

args <- commandArgs(trailingOnly = TRUE)
id <- args[1]
interval <- as.integer(args[2])

### time
data <- read.csv(paste("results/", id, "_time.csv", sep=""))
total_mean <- mean(apply(data, 1, mean))

duration <- dim(data)[1]
labels <- seq(0, duration*interval / 60, 2) %% 24
breaks <- seq(0,duration,120/interval)
pdfSize <- 7 + round(duration*interval/1440) * 5
                   
df <- melt(data)
p <- ggplot(df, aes(x=variable, y=value)) +
  geom_boxplot() +
  stat_summary(fun.y=mean, geom="line", colour="#1F78B4", aes(group=1))  + 
  stat_summary(fun.y=mean, geom="point", colour="#1F78B4") +
  geom_hline(yintercept=total_mean, colour="#E31A1C") +
  theme_bw() +
  ylab("time / ms") +
  xlab("clusters") +
  theme(axis.ticks = element_blank(), axis.text.x = element_blank())
pdf("results/time_box.pdf", width=5, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))

df <- data.frame(data, total_mean, x=c(0:(duration-1)))
df <- melt(df, "x")
p <- ggplot(df, aes(x=x, y=value)) + xlab("time of day / h") + ylab("time / ms") +
  geom_step(aes(group=variable, colour=variable)) +
  scale_x_continuous(expand=c(0,0), breaks=breaks, labels=labels) +
  theme_bw() +
  theme(legend.title=element_blank())
pdf("results/time_line.pdf", width=pdfSize, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))

### msgs
data <- read.csv(paste("results/", id, "_msgs.csv", sep=""))
total_mean <- mean(apply(data, 1, mean))

df <- melt(data)
p <- ggplot(df, aes(x=variable, y=value)) +
  geom_boxplot() +
  stat_summary(fun.y=mean, geom="line", colour="#1F78B4", aes(group=1))  + 
  stat_summary(fun.y=mean, geom="point", colour="#1F78B4") +
  geom_hline(yintercept=total_mean, colour="#E31A1C") +
  theme_bw() +
  ylab("number of messages") +
  xlab("clusters") +
  theme(axis.ticks = element_blank(), axis.text.x = element_blank())
pdf("results/msgs_box.pdf", width=5, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))

df <- data.frame(data, total_mean, x=c(0:(duration-1)))
df <- melt(df, "x")
p <- ggplot(df, aes(x=x, y=value)) + xlab("time of day / h") + ylab("number of messages") +
  geom_step(aes(group=variable, colour=variable)) +
  scale_x_continuous(expand=c(0,0), breaks=breaks, labels=labels) +
  theme_bw() +
  theme(legend.title=element_blank())
pdf("results/msgs_line.pdf", width=pdfSize, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))