# EmmKiuTeeTi-Trials: Tests and Trials of MQTT and its Implementations

## Introdudction

This repository serves as a test bed for our MQTT trials. Here we test MQTT brokers (such as FlashMQ) and clients (such as Paho) in various means such as Fuzz and Stress Testing the perofrmance. We may also attempt to find various means of implementations ourselves. We will also implement MQTT, partly, so we can generate binary data for overhead-free testing of the implementations. This repo will span across several languages and methods. The aim is to arrive at a performant daemon for IoT.

**Notice**: This repository is under active attendance, files will be added, removed and modified periodically.

## Files in This Repository

Files in this repo [at the moment] are:

Status Legend (not all of these maybe used): 

* [N]: Near-Complete
* [*]: Revision Number *
* [O]: Ongoing
* [X]: Deprecated
* [S]: Started
* [A]: Abandoned
* [C]: Complete

Topic Legend:

* [t]: Test-Related
* [p]: Protocol-Related
* [r]: Report File
* [d]: Profiler Dump
* [u]: Utility Script
* [e]: External Script


* `mqtt-packgen.py` [Np]: This file generates packets for overhead-free testing. It complies to the latest OASIS-approved version of MQTT as specified [here](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718009). There is no plans, of course, to implement the whole protocol. Just the parts we need for testing. Please see the [Packet Generation](#packet-generation) section to find out how it works. It is not a proper implementation, keep that in mind.


* `mqtt-payloadgen.py` [1u]: For generating payloads for use with both ad-hoc static packets and Paho and other clients. It will generate a payload based on an evaluative expression. See [Payload Generation](#payload-generation) for more info.


* `zinteger.py` [Ce] : I had written ZInteger beforehand. It is basically a dynamically-generated wrapper for ctypes integers in Python. You can find the main Gist in my profile.

