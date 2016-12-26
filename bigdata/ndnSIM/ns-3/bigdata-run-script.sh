replication=40
nbStorages=50
dimension=10
nbSegments=10
nbCores=4
nbCoreSto=5
nbCoreDim=4
nbCoreSeg=4
for i in `seq 1 $nbCores $replication` #replication
do
    for j in `seq 0 $((nbCores -1))`
	do
		echo "Starting Experiment Replication $((i + j + 1))" #replication
		
			for a in `seq 1 $nbCoreSto $nbStorages` #storage
			do
			    for b in `seq 0 $((nbCoreSto -1))`
				do
					 if test "$((i + j +1))" -gt "$((a + b))"; then #replication factor should be less or eq to the number of storage
						continue
 					 fi

					echo "Starting Experiment Replication $((i + j + 1)) | Storage $((a + b))" #storage
		
					for c in `seq 1 $nbCoreDim $dimension` #dimension
					do
					    for d in `seq 0 $((nbCoreDim -1))`
						do
							 if test "$(((c + d-2)  * (c + d-2)))" -lt "$((a + b))"; then #replication factor should be less or eq to the number of storage
								continue
							 fi

							 if test "$(( ((c + d-2) * (c + d-2)) - (3*(c + d-2)) ))" -lt "$((a + b))"; then #the number of storage should be dimension^2 -3*dimension > nbStorge
								continue
							 fi

							echo "Starting Experiment Replication $((i + j + 1)) | Storage $((a + b)) |  Dimension $((c + d -2))"


							for e in `seq 1 $nbCoreSeg $nbSegments`
							do
							    for f in `seq 0 $((nbCoreSeg -1))`
								do
									echo "Starting Experiment Replication $((i + j + 1)) | Storage $((a + b)) |  Dimension $((c + d -2)) | Segment $((e + f))"
									./waf --run="ndn-bigdata --replication=$((i + j +1)) --nbStorages=$((a + b)) --dimension=$((c + d -2)) --segments=$((e+f)) " & 
									sleep 10
								done #f
							done #e

						done #d
					done #c


				done #b
			done #a

	done #j

done #i
