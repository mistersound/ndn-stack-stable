replication=5
nbStorages=10
dimension=5
nbSegments=210
for i in `seq 190 $nbSegments`
do
    echo "Starting Experiment $i"
	NS_LOG="ndn.Admin=info:ndn.Storage=info:nfd.BigDataStrategy=info" ./waf --run="ndn-bigdata --replication=$replication --nbStorages=$nbStorages --dimension=$dimension --segments=$i " & 

done