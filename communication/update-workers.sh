#!/bin/bash

function update() {
	sshpass -p cubie ssh cubie@$1 << END
	cd haec/communication/
	git pull
	rm worker
	make worker
END
}

for i in 21 22 23 41 42 43; do
	update 192.168.1.$i &
done
