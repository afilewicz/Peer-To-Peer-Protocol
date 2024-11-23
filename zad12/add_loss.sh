apt-get install -y iproute2
docker exec z16_client tc qdisc add dev eth0 root netem delay 1000ms 100ms loss 50%
