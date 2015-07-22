
blink=0

while [ "1" -eq "1" ]; do
	curl -H "Content-Type: application/json" -X POST -d "{\"id\":\"led\", \"token\":\"salut123\", \"value\":$blink}" http://andreiro-server.iot.wyliodrin.com/send
	blink=$((1-blink)) 
	sleep 0.5
done