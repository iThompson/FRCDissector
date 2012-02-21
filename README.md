FRC Protocol Dissectors for Wireshark
=====================================

This WIP [Wireshark](http://www.wireshark.org) plugin contains dissectors for the various protocols running in the [FIRST Robotics Competition](http://www.usfirst.org). Please note that some are incomplete. Currently, the plugin is compiled against release version 1.6.2 of Wireshark, but it should be forward-compatible.

Running
-------
The .dll file included here is compiled for 64-bit Wireshark. If you happen to be running 64-bit, just copy frc.dll into C:\Program Files\Wireshark\plugins\<version>. If you're running 32-bit, or are not on Windows, you'll need to compile the plugin yourself. For Windows, follow the [build instructions](http://www.wireshark.org/docs/wsdg_html_chunked/ChSetupWin32.html) for Wireshark. After downloading the wireshark sources, copy this repository into wireshark/plugins/frc. Make sure you do this BEFORE running distclean. After building wireshark, the 64-bit .dll will have been replaced with a 32-bit version.

Dissectors
----------
This plugin provides dissectors for many, but not yet all, of the protocols used. This is a list of what is providied.

### FRCDS ###
The packets sent from the Driver Station to the Robot. Mostly finished, some of the Enhanced I/O still needs to be added in

### NETCON ###
The NetConsole protocol. This is complete.

### CRRVISION ###
The protocol used by [Team 639](http://www.team639.org) to communicate between the vision processor and the cRIO. This was written for debugging purposes and is most likely not useful to other teams.
