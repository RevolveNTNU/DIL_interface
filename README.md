# DIL_interface

This repository uses the ``libevdev`` library, which essentially is a ``read(2)`` on steroids for reading events from ``/dev/input/eventX`` devices on the Linux kernel. A custom kernel driver, [**LKDF**](https://github.com/gotzl/hid-fanatecff/tree/master) is necessary to manage the low-level USB HID connection. Follow compilation and installation steps depending on your Linux distro. Generated events are categorized into: 

- **EV_ABS**: Axis signals (steering, throttle, etc.)
- **EV_KEY**: Press/release from buttons.

See ``/usr/include/linux/input-event-codes.h`` for details.

### Install ``libevdev``
#### Ubuntu:
```bash
sudo apt-get install libevdev-dev
```
#### Dockerfile:
```Docker
RUN apt-get update && apt-get install -y libevdev-dev
```
### How to find correct ``/dev/input/eventX`` device:
Connect racing gear and run the following.
```bash
sudo apt-get install evtest #install evtest
sudo evtest
```
All available devices should be listed with names and event number. Select **Fanatec FANATEC CSL Elite Wheel Base**, and data from said device should be logged when e.g. turning the wheel.