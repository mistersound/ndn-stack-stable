replication=40
nbStorages=50
dimension=10
nbSegments=10
nbCores=4
for i in `seq 1 $nbCores $nbSegments`
do
    for j in `seq 0 $((nbCores -1))`
	do
		echo "Starting Experiment $((i + j))"
		#./waf --run=
echo "ndn-bigdata --replication=$replication --nbStorages=$nbStorages --dimension=$dimension --segments=$((i + j)) " #& 
		#sleep 10
	done
	echo "Starting Experiment $((i + nbCores))"
	#./waf --run=
echo "ndn-bigdata --replication=$replication --nbStorages=$nbStorages --dimension=$dimension --segments=$((i + j)) "  
	#sleep 10
done
