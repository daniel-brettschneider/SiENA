library(ggplot2)
library(reshape)

##### params
id <- "test"
interval <- 15
type <- "pcharge"
device <- "tank"

data <- read.csv(paste("results/", id, "_", type, ".csv", sep=""))
duration <- dim(data)[1]
labels <- seq(0, duration*interval / 60, 2) %% 24
breaks <- seq(0,duration,120/interval)
pdfSize <- 7 + round(duration*interval/1440) * 5
index <- grep(device, colnames(data))
df <- data.frame(data[,index])


df_temp <- data.frame(data[,0])
tmp <- c(0:(duration-1))
df_temp$max <- apply(df, 1, max)
df_temp$min <- apply(df, 1, min)
df_temp$mean <- apply(df, 1, mean)
df_temp$median <- apply(df, 1, median)
#df<-df_temp
#sum <- 0
#for(i in 1:duration) {
#  sum <- sum + df[i];
#  tmp[i] <- sum;
#}
#df <- data.frame(tmp)

df$x <- c(0:(duration-1))
df <- melt(df, id="x")
pdf("results/device.pdf", width=pdfSize, height=5)
p <- qplot(x, value, data=df, colour=variable, geom="step", xlab="time of day / h", ylab="") +
  scale_x_continuous(expand=c(0,0), breaks=breaks, labels=labels) #+
  #scale_y_continuous(expand=c(0,0), limit=c(0,20000), breaks=c(7000), labels=c(7000))
  #theme(legend.title=element_blank()) +
  #theme(legend.position="none")
p
dev.off()


#width <- pdfSize*100
#if (width > 32767) {width = 32767}
#png("results/device.png", width=width, height=500)
#p <- qplot(x, value, data=df, colour=variable, geom="step", xlab="time of day / h", ylab="") +
#  scale_x_continuous(expand=c(0,0), breaks=breaks, labels=labels) #+
#p
#dev.off()