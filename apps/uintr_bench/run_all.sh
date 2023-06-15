names=(base64+base64+base64+base64 base64+base64+base64 base64+base64 base64 base64+linpack base64+matmul base64+sum linpack+linpack+linpack+linpack linpack+linpack+linpack linpack+linpack linpack linpack+sum matmul+linpack matmul+matmul+matmul+matmul matmul+matmul+matmul matmul+matmul matmul matmul+sum sum+sum+sum sum+sum sum base64+matmul+linpack)

for name in "${names[@]}"
do
    echo "running for ${name}"
	./run.sh ${name}
done


