import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client, sensor, binary_sensor, climate, switch
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_RUNNING,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)
from esphome.core import CORE

DEPENDENCIES = ["ble_client"]
AUTO_LOAD = ["sensor", "binary_sensor", "climate", "switch"]

truma_cooler_ns = cg.esphome_ns.namespace("truma_cooler")
TrumaCooler = truma_cooler_ns.class_(
    "TrumaCooler", cg.Component, ble_client.BLEClientNode
)
TrumaCoolerClimate = truma_cooler_ns.class_(
    "TrumaCoolerClimate", climate.Climate, cg.Parented.template(TrumaCooler)
)
TrumaCoolerSwitch = truma_cooler_ns.class_(
    "TrumaCoolerSwitch", switch.Switch, cg.Parented.template(TrumaCooler)
)

CONF_TEMPERATURE = "temperature"
CONF_AMBIENT_TEMPERATURE = "ambient_temperature"
CONF_COMPRESSOR_RUNNING = "compressor_running"
CONF_TURBO_RUNNING = "turbo_running"
CONF_DEVICE_ON = "device_on"
CONF_CONNECTED = "connected"
CONF_CLIMATE = "climate"
CONF_TURBO = "turbo"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(TrumaCooler),
            cv.Optional(CONF_CLIMATE): climate.climate_schema(TrumaCoolerClimate),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_AMBIENT_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_COMPRESSOR_RUNNING): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_RUNNING,
            ),
            cv.Optional(CONF_TURBO_RUNNING): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_RUNNING,
            ),
            cv.Optional(CONF_DEVICE_ON): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_CONNECTIVITY,
            ),
            cv.Optional(CONF_TURBO): switch.switch_schema(TrumaCoolerSwitch),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    if not (CORE.is_esp32 and not CORE.using_arduino):
        raise cv.Invalid("truma_cooler requires ESP32 with the ESP-IDF framework")

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    if CONF_CLIMATE in config:
        clim = cg.new_Pvariable(config[CONF_CLIMATE][CONF_ID])
        await climate.register_climate(clim, config[CONF_CLIMATE])
        cg.add(clim.set_parent(var))
        cg.add(var.set_climate(clim))

    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature_sensor(sens))

    if CONF_AMBIENT_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_AMBIENT_TEMPERATURE])
        cg.add(var.set_ambient_temperature_sensor(sens))

    if CONF_COMPRESSOR_RUNNING in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_COMPRESSOR_RUNNING])
        cg.add(var.set_compressor_running_sensor(sens))

    if CONF_TURBO_RUNNING in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_TURBO_RUNNING])
        cg.add(var.set_turbo_running_sensor(sens))

    if CONF_DEVICE_ON in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_DEVICE_ON])
        cg.add(var.set_device_on_sensor(sens))

    if CONF_CONNECTED in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_CONNECTED])
        cg.add(var.set_connected_sensor(sens))

    if CONF_TURBO in config:
        sw = await switch.new_switch(config[CONF_TURBO])
        cg.add(sw.set_parent(var))
        cg.add(var.set_turbo_switch(sw))
