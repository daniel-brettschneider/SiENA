msg.trap <- capture.output(suppressMessages(library(ggplot2)))
msg.trap <- capture.output(suppressMessages(library(reshape)))

args <- commandArgs(trailingOnly = TRUE)
id <- args[1]
interval <- as.integer(args[2])
startday <- 1
if(length(args) > 2)
  startday <- as.integer(args[3])

start <- (startday - 1) * 1440 / interval + 1
data <- read.csv(paste("results/", id, "_consumption_grouped.csv", sep=""))
data <- data[start:dim(data)[1],]
data <- apply(data, 1, sum)
duration <- length(data)

compare <- read.csv("data/dsm/quality_compare.csv")
compare <- compare[start:dim(compare)[1],]
goal_tmp <- read.csv("data/dsm/quality_goal.csv")
goal <- c(1:duration)
for(i in 0:(duration-1)) {
  goal[i + 1] <- goal_tmp[(i %% 1440) + 1, 1]
}

labels <- seq(0, duration*interval / 60, 2) %% 24
breaks <- seq(0,duration,120/interval)
pdfSize <- 7 + round(duration*interval/1440) * 5

df <- data.frame(compare, goal, data)
colnames(df) <- c("without DSM", "goal", "simulated DSM")
df$x <- c(1:duration)
df <- melt(df, id="x")

p <- ggplot(df, aes(x=x, y=value)) + xlab("time of day / h") + ylab("consumption / W") +
  geom_step(aes(colour=variable)) +
  scale_x_continuous(expand=c(0,0), breaks=breaks, labels=labels) +
  theme_bw() +
  theme(legend.title=element_blank()) +
  scale_colour_brewer(palette="Set1")
pdf(file="results/quality.pdf", width=pdfSize, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))