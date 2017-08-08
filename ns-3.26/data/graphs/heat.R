msg.trap <- capture.output(suppressMessages(library(ggplot2)))
msg.trap <- capture.output(suppressMessages(library(reshape)))

args <- commandArgs(trailingOnly = TRUE)
id <- args[1]
interval <- as.integer(args[2])

data <- read.csv(paste("results/", id, "_heat.csv", sep=""))

duration <- dim(data)[1]
labels <- seq(0, duration*interval / 60, 2) %% 24
breaks <- seq(0,duration,120/interval)
pdfSize <- 7 + round(duration*interval/1440) * 5

# demand
index <- grep("_demand", colnames(data))
if(length(index) > 1) {
  tmp <- apply(data[,index], 1, mean)
} else {
  tmp <- data[,index]
}
y <- c(rep(tmp, each=2), 0, 0)
x <- c(0, rep(1:duration, each=2), 0)
d <- data.frame(x=x, y=y)
# unsatisfied
index <- grep("_unsatisfied", colnames(data))
if(length(index) > 1) {
  tmp2 <- apply(data[,index], 1, mean)
} else {
  tmp2 <- data[,index]
}
y2 <- c(rep(tmp2, each=2), 0, 0)
x2 <- c(0, rep(1:duration, each=2), 0)
d2 <- data.frame(x=x2, y=y2)
# total
total <- data.frame(x=c(0:(duration-1)), y=tmp+tmp2)
total <- melt(total, id="x")
# plot
p <- ggplot() + xlab("time of day / h") + ylab("average heat / W") +
  geom_polygon(data=d, aes(x=x, y=y, fill="demand"), alpha=0.75) +
  geom_polygon(data=d2, aes(x=x, y=y, fill="unsatisfied"), alpha=0.75) +
  #geom_step(data=total, aes(x=x, y=value, color="total"), size=I(0.35)) +
  scale_x_continuous(expand=c(0,0), breaks=breaks, labels=labels) +
  theme_bw() +
  theme(legend.title=element_blank()) +
  scale_fill_manual(values=c("demand"="#33A02C", "unsatisfied"="#E31A1C")) #+
  #scale_color_manual(values=c("total"="black"))
pdf(file="results/heat.pdf", width=pdfSize, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))