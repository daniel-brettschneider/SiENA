msg.trap <- capture.output(suppressMessages(library(ggplot2)))
msg.trap <- capture.output(suppressMessages(library(reshape)))
msg.trap <- capture.output(suppressMessages(library(plyr)))

args <- commandArgs(trailingOnly = TRUE)
id <- args[1]
interval <- as.integer(args[2])

theme_thesis <- function(base_size = 12, base_family = "Helvetica") {
  theme_bw(base_size = base_size, base_family = base_family) %+replace% 
    theme(
      axis.ticks = element_line(colour = "black"), 
      axis.title.y = element_text(angle=90, margin=margin(0, 3, 0, 0, unit="mm")),
      panel.border = element_rect(fill = NA, colour = "black", size=0.3),
      panel.grid.major = element_line(colour = "grey75"),
      panel.grid.minor = element_line(colour = "grey90"),
      legend.position = "bottom",
      legend.direction = "horizontal",
      legend.title = element_blank(),
      legend.key.size = unit(0.4, "cm"),
      legend.key = element_blank(),
      legend.margin = unit(0, "cm")
    )
}

data <- read.csv(paste("results/", id, "_consumption.csv", sep="")) / 1000

duration <- dim(data)[1]
labels <- seq(0, duration*interval / 60, 2) %% 24
breaks <- seq(0,duration,120/interval)
pdfSize <- 7 + round(duration*interval/1440) * 5

get_columns <- function(data, names) {
  ids <- grep(names, colnames(data))
  if(length(ids) == 0) {
    df <- c(rep(0,dim(data)[1]))
    return(list(df, df))
  }
  df <- data[,ids]
  dfp <- df
  dfp[dfp < 0] <- 0
  dfp <- apply(dfp, 1, sum)
  df[df > 0] <- 0
  df <- apply(df, 1, sum)
  return(list(df, dfp))
}

base <- get_columns(data, "baseload")
shift <- get_columns(data, "dishwasher|drier|washingmachine")
switch <- get_columns(data, "heatpump|chps")
adapt <- get_columns(data, "car|battery")
pv <- get_columns(data, "pv")

all <- list(base, shift, switch, adapt, pv)
types <- c("base", "shiftable", "switchable", "adaptable", "pv")
df <- data.frame()
dfn <- data.frame()
x <- c(0:(duration-1))
for(i in 1:length(all)) {
  df_n <- data.frame(y=all[[i]][[1]], x=x, type2=types[i])
  df_p <- data.frame(y=all[[i]][[2]], x=x, type2=types[i])
  if(i == 1) {
    df <- df_p
    dfn <- df_n
  } else {
    df <- rbind(df, df_p)
    dfn <- rbind(dfn, df_n)
  }
}

df_con <- apply(data, 1, sum)
df_con <- data.frame(x, total=df_con)
df_con <- melt(df_con, id="x")

df2 <- rbind(
  df,
  transform(df[order(df$x),],
            x=x - 1e-9,  # required to avoid crazy steps
            y=ave(y, type2, FUN=function(z) c(z[[1]], head(z, -1L)))
) )
dfn2 <- rbind(
  dfn,
  transform(dfn[order(dfn$x),],
            x=x - 1e-9,  # required to avoid crazy steps
            y=ave(y, type2, FUN=function(z) c(z[[1]], head(z, -1L)))
) )

p <- ggplot(df2, aes(x=x, y=y)) + xlab("time of day / h") + ylab("consumption / kW") +
  geom_area(aes(fill=type2), position='stack', alpha=0.75) +
  geom_area(data=dfn2, aes(x=x, y=y, fill=type2), position='stack', alpha=0.75) +
  geom_step(data=df_con, aes(x=x, y=value, color="total"), size=I(0.35)) +
  scale_x_continuous(expand=c(0,0), breaks=breaks, labels=labels) +
  theme_thesis() +
  theme(legend.box="horizontal") +
  scale_fill_manual(values=c("base"="#E31A1C", "shiftable"="#1F78B4", "adaptable"="#33A02C", "pv"="#FF7F00", "switchable"="#984EA3")) +
  scale_color_manual(values=c("total"="black"))
pdf(file="results/consumption_grouped2.pdf", width=pdfSize, height=5)
p
msg.trap <- capture.output(suppressMessages(dev.off()))