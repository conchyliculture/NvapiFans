
uninstall:
	rm /etc/modules-load.d/asus_fc2.conf
	systemctl stop asus_fc2
	systemctl disable asus_fc2
	rm /usr/local/bin/asus_fc2.sh
	rm /etc/systemd/system/asus_fc2.service
	systemctl daemon-reload


install:
	echo "i2c-dev" > /etc/modules-load.d/${modname}.conf
	modprobe i2c-dev
	cp asus_fc2.sh /usr/local/bin/
	chown root:root /usr/local/bin/asus_fc2.sh
	chmod u+x /usr/local/bin/asus_fc2.sh
	cp asus_fc2.service /etc/systemd/system/asus_fc2.service
	chmod 644 /etc/systemd/system/asus_fc2.service
	systemctl daemon-reload
	systemctl enable asus_fc2
	systemctl start asus_fc2
