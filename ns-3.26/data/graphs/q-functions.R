msg.trap <- capture.output(suppressMessages(library(ggplot2)))
msg.trap <- capture.output(suppressMessages(library(reshape)))

args <- commandArgs(trailingOnly = TRUE)
id <- args[1]
interval <- as.integer(args[2])

data <- read.csv(paste("results/", id, "_quality.csv", sep=""))
data <- data[, -c(10)] #delete weighting
data <- apply(data, 2, mean)
data <- data.frame(data)
data$name <- rownames(data)

df1 <- data
df1$x <- 1
p <- ggplot(df1, aes(x=name, y=x)) + ylab("quality value") + xlab("quality function") +
  geom_tile(aes(fill=data), colour="white") +
  scale_fill_gradientn(colours=c("#E31A1C", "white", "#33A02C"), limits=c(-1, 1), breaks=c(-1, 0, 1), labels=c(-1, 0, 1)) +
  geom_text(aes(fill=data, label=sprintf("%.03f", data))) +
  scale_x_discrete(expand=c(0,0)) +
  scale_y_discrete(expand=c(0,0), limits=c(1), labels=c("")) +
  theme_bw() +
  theme(axis.ticks=element_blank()) +
  theme(legend.position="right",legend.background=element_blank(),legend.title=element_blank()) +
  guides(fill = guide_colorbar(barheight=3.5))
pdf(file=paste("results/q-functions_tile.pdf", sep=""), width=7, height=1.5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))

df2 <- data
p <- ggplot(df2, aes(x=name, y=data)) + ylab("quality value") + xlab("quality function") +
  geom_bar(stat="identity", aes(fill=data)) +
  scale_y_continuous(limits=c(-1,1)) +
  scale_fill_gradientn(colours=c("#E31A1C", "#DBDBDB", "#33A02C"), limits=c(-1, 1), breaks=c(-1, 0, 1), labels=c(-1, 0, 1)) +
  theme_bw() +
  theme(legend.position="right",legend.background=element_blank(),legend.title=element_blank()) +
  guides(fill = guide_colorbar(barheight=3.5))
pdf(file=paste("results/q-functions_bar.pdf", sep=""), width=7, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))

#Q-Functions coord_polar
df3 <- data
p <- ggplot(df2, aes(x=name, y=data)) + ylab("") + xlab("") +
  geom_bar(stat="identity", aes(fill=data)) +
  coord_polar(theta = "x") +
  scale_y_continuous(limits=c(-1,1)) +
  scale_fill_gradientn(colours=c("#E31A1C", "#DBDBDB", "#33A02C"), limits=c(-1, 1), breaks=c(-1, 0, 1), labels=c(-1, 0, 1)) +
  theme_bw() +
  theme(legend.position="right",legend.background=element_blank(),legend.title=element_blank()) +
  guides(fill = guide_colorbar(barheight=3.5))
pdf(file=paste("results/q-functions_polar.pdf", sep=""), width=7, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))
  