# _Project File

1 - menuconfig using idf.py menuconfig and then head to Component config -> FAT Filesystem support -> Long filename support -> Long filename buffer {in heap|on stack}.

2 - menuconfig using idf.py menuconfig and then head to Partition Table -> Partition Table -> Custom partition CSV file -> Change the name of partition  file .

3 - While using Fatfs Change the Flash Size in menuconfig to 4MB.

4 - Webserver details 
    SSID:- ESP32_webserver
    Password:- esp12345678
