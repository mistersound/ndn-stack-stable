

# install.packages('ggplot2')
library(ggplot2)

library(plyr)

n = 9

df <- data.frame(NbPackets = numeric(n), ReplicationFactor  = numeric(n))
for(i in 1:n){
  pathElems <- c("/home/ndn/ndnSIM/ns-3/rate-trace-", ".txt")
  data = read.table(paste(pathElems, collapse=toString(i)), header=T)
  idata = subset(data, Type %in% c("InInterests"))
  idata = subset(idata, FaceDescr %in% c("appFace://"))
  
  df$NbPackets[i] <-  sum(idata$Packets)
  df$ReplicationFactor[i] <- i
}
df

ggplot(data=df, aes(x=ReplicationFactor, y=NbPackets, group=1)) +
  geom_line() +
  geom_point()




#data = read.table("rate-trace.txt", header=T)
data = rate.trace.6;

data$Time = factor(data$Time)
data$Type = factor(data$Type)
data$Node = factor(data$Node)

#data <- data[1:1500,]

# exlude irrelevant types
idata = subset(data, Type %in% c("InInterests"))

#tdata = subset(idata, Time %in% seq(0,5,0.1))

cdata <- ddply(idata, c("Time"), summarise,
               NbPackets    = sum(Packets)
)



# Map sex to color
ggplot(data=cdata, aes(x=Time, y=NbPackets, group=1)) +
  geom_line() +
  geom_point()

# Add points
ggplot(data=cdata, aes(x=Time, y=NbPackets,group=Node , fill=Node)) +
  geom_line() +
  geom_point()

# Bar graph, time on x-axis, color fill grouped by sex -- use position_dodge()
ggplot(data=cdata, aes(x=Time, y=NbPackets, fill=Node)) +
  geom_bar(stat="identity", position=position_dodge())