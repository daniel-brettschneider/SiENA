msg.trap <- capture.output(suppressMessages(library(ggplot2)))
msg.trap <- capture.output(suppressMessages(library(reshape)))

args <- commandArgs(trailingOnly = TRUE)
id <- args[1]
interval <- as.integer(args[2])

data <- read.csv(paste("results/", id, "_consumption_grouped.csv", sep=""))

duration <- dim(data)[1]
labels <- seq(0, duration*interval / 60, 2) %% 24
breaks <- seq(0,duration,120/interval)
pdfSize <- 7 + round(duration*interval/1440) * 5

##### consumption
x <- c(0:(duration-1))
df_con <- apply(data, 1, sum)
df_con <- data.frame(x, total=df_con)
df_con <- rbind(df_con, c(duration, df_con$total[duration]))
df_con <- melt(df_con, id="x")
p <- ggplot(df_con, aes(x=x, y=value)) + xlab("time of day / h") + ylab("average consumption / W") +
  geom_step(aes(group=variable, colour=variable)) +
  scale_x_continuous(expand=c(0,0), breaks=breaks, labels=labels) +
  theme_bw() +
  theme(legend.title=element_blank()) +
  scale_color_brewer(palette="Set1")
pdf(file="results/consumption.pdf", width=pdfSize, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))

##### consumption grouped
# split adaptable in positive and negative
adapt_con <- rep(0,duration)
adapt_feed <- rep(0,duration)
other_con <- rep(0,duration)
other_feed <- rep(0,duration)
switch_con <- rep(0, duration)
switch_feed <- rep(0, duration)
for(i in 1:duration) {
  if(data$adaptable[i] < 0)
    adapt_feed[i] = data$adaptable[i]
  else
    adapt_con[i] = data$adaptable[i]
  if(data$other[i] < 0)
    other_feed[i] = data$other[i]
  else
    other_con[i] = data$other[i]
  if(data$switchable[i] < 0)
    switch_feed[i] = data$switchable[i]
  else
    switch_con[i] = data$switchable[i]
}
# base
y <- c(rep(data$base, each=2), 0, 0)
x <- c(0, rep(1:duration, each=2), 0)
d <- data.frame(x=x, y=y)
# movable
tmp <- data$base + data$movable
y2 <- c(rep(tmp, each=2), rev(y[1:(length(y)-2)]))
x2 <- c(x[1:(length(x)-2)], rev(x[1:(length(x)-2)]))
d2 <- data.frame(x=x2, y=y2)
# adaptable positive
tmp2 <- tmp + adapt_con
y3 <- c(rep(tmp2, each=2), rev(y2[1:(duration*2)]))
x3 <- x2
d3 <- data.frame(x=x3, y=y3)
# switchable positive
tmp3 <- tmp2 + switch_con
y7 <- c(rep(tmp3, each=2), rev(y3[1:(duration*2)]))
x7 <- x3
d7 <- data.frame(x=x7, y=y7)
# other positive
tmp5 <- tmp2 + other_con
y5 <- c(rep(tmp5, each=2), rev(y3[1:(duration*2)]))
x5 <- x3
d5 <- data.frame(x=x5, y=y5)
# other negative
y6 <- c(rep(other_feed, each=2), 0, 0)
x6 <- x
d6 <- data.frame(x=x6, y=y6)
# adaptable negative
tmp4 <- adapt_feed + other_feed
y4 <- c(rep(tmp4, each=2), rev(y6[1:(length(y6)-2)]))
x4 <- c(x6[1:(length(x6)-2)], rev(x6[1:(length(x6)-2)]))
d4 <- data.frame(x=x4, y=y4)
# switchable negative
tmp8 <- tmp4 + switch_feed
y8 <- c(rep(tmp8, each=2), rev(y4[1:(duration*2)]))
x8 <- x4
d8 <- data.frame(x=x8, y=y8)
# plot
p <- ggplot() + xlab("time of day / h") + ylab("average consumption / W") +
  geom_polygon(data=d, aes(x=x, y=y, fill="base"), alpha=0.75) +
  geom_polygon(data=d2, aes(x=x, y=y, fill="movable"), alpha=0.75) +
  geom_polygon(data=d3, aes(x=x, y=y, fill="adaptable"), alpha=0.75) +
  geom_polygon(data=d4, aes(x=x, y=y, fill="adaptable"), alpha=0.75) +
  geom_polygon(data=d7, aes(x=x, y=y, fill="switchable"), alpha=0.75) +
  geom_polygon(data=d8, aes(x=x, y=y, fill="switchable"), alpha=0.75) +
  geom_polygon(data=d5, aes(x=x, y=y, fill="other"), alpha=0.75) +
  geom_polygon(data=d6, aes(x=x, y=y, fill="other"), alpha=0.75) +
  geom_step(data=df_con, aes(x=x, y=value, color="total"), size=I(0.35)) +
  scale_x_continuous(expand=c(0,0), breaks=breaks, labels=labels) +
  theme_bw() +
  theme(legend.title=element_blank()) +
  scale_fill_manual(values=c("base"="#E31A1C", "movable"="#1F78B4", "adaptable"="#33A02C", "other"="#FF7F00", "switchable"="#984EA3")) +
  scale_color_manual(values=c("total"="black"))
pdf(file="results/consumption_grouped.pdf", width=pdfSize, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))