# Light sensor demo: BH1750 readings drive a state, sent to the sketch over the Bridge.
from arduino.app_utils import *
import random
import time

NIGHT_THRESHOLD_LUX = 50
ACTIVE_STATES = ["idle", "listening", "speaking"]


def read_lux():
    """Read the BH1750 light sensor through the sketch. Returns lux, or None if no sensor."""
    lux = Bridge.call("read_lux")  # Call the "read_lux" function defined in the sketch
    if lux is None or lux < 0:
        return None
    return round(lux, 1)


def pick_state(lux):
    if lux < NIGHT_THRESHOLD_LUX:
        return "night"
    return random.choice(ACTIVE_STATES)


def loop():
    lux = read_lux()
    if lux is None:
        print("lux=n/a no BH1750 found, check wiring", flush=True)
        time.sleep(4)
        return
    state = pick_state(lux)
    print(f"lux={lux} state={state}", flush=True)
    Bridge.call("set_state", state)  # Call the "set_state" function defined in the sketch
    time.sleep(4)


App.run(user_loop=loop)
