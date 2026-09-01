from libs.PipeLine import PipeLine
from media.sensor import Sensor
import config


def create_pipeline():
    pl = PipeLine(rgb888p_size=config.RGB888P_SIZE,
                  display_size=config.DISPLAY_SIZE,
                  display_mode=config.DISPLAY_MODE)
    pl.create(Sensor(width=config.SENSOR_SIZE[0], height=config.SENSOR_SIZE[1]))
    print("Display:", config.DISPLAY_SIZE)
    return pl, config.DISPLAY_SIZE
