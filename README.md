# baatvakt

Part of the Båtvakt project. Code for Veslefrikk so far

To read current code from Arduino use
avrdude -c arduino -P /dev/ttyACM0 -U flash:r:veslefrikk.bin:hex (or flash:r:xxx.bin:r ???)

To write the backup binary file to Arduino
avrdude -c arduino -P /dev/ttyACM0 -U flash:w:veslefrikk.bin:hex  