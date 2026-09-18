set -x

cat << END | fdisk $1
n
p
1

+1G
n
p
2

+14G
t
1
c
t
2
c
a
1
w
END
