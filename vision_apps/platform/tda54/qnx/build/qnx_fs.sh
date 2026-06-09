set -x

cat << END | fdisk $1
n
p
1


t
c
w
END
