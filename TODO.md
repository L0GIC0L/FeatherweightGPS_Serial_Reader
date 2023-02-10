# TODO.md

These are the objectives that must be accomplished before first release:

### Todo

- [X] Implement the native serial libraries instead of boost libraries.
  - [X] Fundamental research.
  - [X] Implementation.
  - [ ] Reliablilty testing

- [ ] Implement a cmake file for easier cross-platform compilation.

- [ ] Interface and display data on a GUI interface (iMgui?)
  - [X] Fundamental understanding of iMgui established.
  - [ ] Create a basic window with a plot showing accelerometer data. 

### In Progress

- [ ] Sort and break up the recieved string.
  - [X] Serial connection established.
  - [X] Data is saved into a string.
  - [ ] Data is broken up into a string and separated.
  Parsed String Example:
  
"@ GPS_STAT 202 0000 00 00 00:43:45.329 CRC_OK  TRK tracker     Alt 000000 lt +00.00000 ln +00.00000 Vel +0000 +000 +0000 Fix 0 #  0  0  0  0 000_00_00 000_00_00 000_00_00 000_00_00 000_00_00 CRC: FD6F"

  - [ ] Data is stored in a "map" or some other equivalent.
  - [ ] Store these function into a separate file for easy use in the GUI program.  

### Done ✓

- [x] Connect to Featherweight over serial (again)
- [X] Automate build workflow for Linux
- [ ] Automate build workflow for Windows
