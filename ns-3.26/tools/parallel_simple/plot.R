library(ggplot2)
library(reshape)

id <- "test"
target <- "rounds"

data <- read.csv(paste("results/", id, "_", target, "_mean.csv", sep=""))
#data <- data.frame(run=data$run, data[,grep("PC", colnames(data))])
data$run <- data$run * 100
data <- melt(data, "run")

p <- ggplot(data=data, aes(x=run, y=value, colour=variable)) + xlab("device penetration / %") + ylab(target) +
  geom_line() +
  geom_point() +
  theme_bw() +
  theme(legend.title=element_blank())
pdf(file=paste("results/", id, "_", target, "_mean.pdf", sep=""), width=10, height=5)
p
dev.off()