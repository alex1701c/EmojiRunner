from time import sleep

import uinput

with uinput.Device([uinput.KEY_LEFTCTRL, uinput.KEY_V]) as device:
    sleep(0.5)
    device.emit_combo([uinput.KEY_LEFTCTRL, uinput.KEY_V])
