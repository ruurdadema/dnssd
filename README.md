# DNS Service Discovery for C++
This library provides an interface which allows to work with dns-sd in an easy way.

## Prerequisites

### MacOS

XCode shold be installed

### Windows

This library needs the Bonjour SDK to work and expects it to be in the default install location _(ie C:\Program Files\Bonjour SDK)_


## Terminology around DNS Service Discovery

|          | Description                                          | Wikipedia                                                          |
|----------|------------------------------------------------------|--------------------------------------------------------------------|
| dns-sd   | DNS Service Discovery                                | https://en.wikipedia.org/wiki/Zero-configuration_networking#DNS-SD |
| mDNS     | Multicast DNS, alternative for dns-sd                | https://en.wikipedia.org/wiki/Multicast_DNS                        |
| Zeroconf | Specification for automatic connections in a network | https://en.wikipedia.org/wiki/Zero-configuration_networking        |
| Bonjour  | Apple's implementation of the zeroconf spec          | https://en.wikipedia.org/wiki/Bonjour_(software)                   |
| Avahi    | Open source implementation of the zeroconf spec      | https://en.wikipedia.org/wiki/Avahi_(software)#cite_note-3         |