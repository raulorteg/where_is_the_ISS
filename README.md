# Where the ISS at?
<img src = "files/satellite_img.png" height="150"/>

## About
Where is the ISS at? C project with Arduino/Raspberry Pi that gets the instant position (latitude, longitude, altitude) of the International Space Station, and points towards its location in the sky depending on the user location (latitude, longitude, altitude=0).

## Requirements
Project requires `ljson-c` and `curl`. 

## Usage
Makefile takes care of dependencies and generates executable `curlapp`.
- Clean previous outfiles `make clean`
- Build executable `make`
- Run executable `./curlapp UserCity` (if argument is ommited uses default, Copenhaguen)   

The executable returns User coordinates, Iss current position in coordinates and formatted text, returns distance in flat projection, chord length, and true distance with bearing and elevation angle.

## Structure
* curlapp.c: gets user coords, gets Iss coords, computes distance between, bearing and elevation angles. Main program.
* /files: media for README.md

