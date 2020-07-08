# ATmega 328 Panasonic IR Remote

This is code to use the ATmega-328P as a Panasonic IR remote. Specifically, this
is to replace the remote for a PV-D4733S I got from Goodwill.

## Goals

The project has the following goals:

* _KISS (Keep It Stupid Simple)_ - This is a project with one purpose. To be a good remote for my Panasonic player. Anything not needed (parts or code) for this will be excluded.
* _Low Power_ - I want this to run for a long time off of just a couple AAA batteries.

In order to accomplish these, I am doing the following:

* _Use internal clock_ - I am using the internal clock with the CKDIV8 fuse bit. This removes the dependency on an external crystal as well as allowing the ATmega to run at 1.8V
* _Not using Arduino libraries_ - This project should be Arduino compatible since it is just a ATmega-328P, but the Arduino libraries add overhead that I don't need. Also, the reduced clock speed would likely cause issues with the Arduino libraries anyway.

# EQware

A big thanks to my employer EQware for sponsoring this pet project. EQware is a collaboration of engineers specializing in embedded software services. To learn about what EQware can do for you, please visit [www.eqware.net](https://www.eqware.net/)
