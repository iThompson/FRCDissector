@echo off
nmake -f Makefile.nmake all
IF NOT ERRORLEVEL 1 (copy /Y frc.dll ..\..\wireshark-gtk2\plugins\1.6.2-frcBuild &&..\..\wireshark-gtk2\wireshark.exe -r RBT.pcap )
