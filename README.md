# Where the ISS at?
<img src = "files/satellite_img.png" height="150"/>

## About
Where is the ISS at? C project with Arduino that does gets the instant position (latitude, longitude, altitude) of the International Space Station, and points towards its location in the sky depending on the user location (latitude, longitude, altitude ¿?)

## Requirements
Project requires `ljson-c` and `curl`. 

## Usage
Makefile takes care of dependencies and generates executable `curlapp`.
- Clean previous outfiles `make clean`
- Build executable `make`
- Rub executable `./curlapp`.

The executable returns the bytes collected and the parsed objects. 

## Structure
* curlapp.c: does the http get request and parses json response
* test.json: json example of response from http get request
* /files: media for README.md

