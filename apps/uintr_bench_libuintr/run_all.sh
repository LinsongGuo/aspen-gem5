# names=(base64+base64+base64+base64 base64+base64+base64 base64+base64 base64 base64+linpack base64+matmul base64+sum linpack+linpack+linpack+linpack linpack+linpack+linpack linpack+linpack linpack linpack+sum matmul+linpack matmul+matmul+matmul+matmul matmul+matmul+matmul matmul+matmul matmul matmul+sum sum+sum+sum sum+sum sum base64+matmul+linpack)
# names=(mcf matmul_int linpack sum mcf+matmul_int mcf+linpack linpack+matmul_int mcf+linpack+matmul_int mcf+sum linpack+sum matmul_int+sum mcf+linpack+matmul_int+sum matmul_int+matmul_int matmul_int+matmul_int+matmul_int matmul_int+matmul_int+matmul_int+matmul_int mcf+base64 base64+matmul_int mcf+base64+matmul_int base64+sum mcf+base64+matmul_int+sum base64)

# names=(mcf mcf+mcf mcf+mcf+mcf mcf+mcf+mcf+mcf 
# base64 base64+base64 base64+base64+base64 base64+base64+base64+base64
# matmul_int matmul_int+matmul_int matmul_int+matmul_int+matmul_int matmul_int+matmul_int+matmul_int+matmul_int
# mcf+base64 mcf+matmul_int base64+matmul_int mcf+base64+matmul_int
# sum mcf+sum base64+sum matmul_int+sum mcf+base64+sum mcf+matmul_int+sum base64+matmul_int+sum mcf+base64+matmul_int+sum
# )

# names=(sum+sum sum+sum+sum sum+sum+sum+sum)

names=(matmul_int matmul_int+matmul_int matmul_int+matmul_int+matmul_int matmul_int+matmul_int+matmul_int+matmul_int
mcf+base64 mcf+matmul_int base64+matmul_int mcf+base64+matmul_int
sum sum+sum sum+sum+sum sum+sum+sum
)

for name in "${names[@]}"
do
    echo "running for ${name}"
	./run.sh ${name}
done


