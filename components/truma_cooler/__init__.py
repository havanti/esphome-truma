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
TrumaCoolerC44 = truma_cooler_ns.class_("TrumaCoolerC44", TrumaCooler)
TrumaCoolerC69 = truma_cooler_ns.class_("TrumaCoolerC69", TrumaCooler)
TrumaCoolerClimate = truma_cooler_ns.class_(
    "TrumaCoolerClimate", climate.Climate, cg.Parented.template(TrumaCooler)
)
TrumaCoolerSwitch = truma_cooler_ns.class_(
    "TrumaCoolerSwitch", switch.Switch, cg.Parented.template(TrumaCooler)
)
TrumaCoolerPowerSwitch = truma_cooler_ns.class_(
    "TrumaCoolerPowerSwitch", switch.Switch, cg.Parented.template(TrumaCooler)
)

CONF_MODEL = "model"

# Shared entities (both models).
CONF_COMPRESSOR_RUNNING = "compressor_running"
CONF_DEVICE_ON = "device_on"
CONF_CONNECTED = "connected"

# C44 (single zone).
CONF_CLIMATE = "climate"
CONF_TEMPERATURE = "temperature"
CONF_AMBIENT_TEMPERATURE = "ambient_temperature"
CONF_TURBO_RUNNING = "turbo_running"
CONF_TURBO = "turbo"

# C69 (dual zone).
CONF_CLIMATE_ZONE1 = "climate_zone1"
CONF_CLIMATE_ZONE2 = "climate_zone2"
CONF_TEMPERATURE_ZONE1 = "temperature_zone1"
CONF_TEMPERATURE_ZONE2 = "temperature_zone2"
CONF_POWER = "power"


def _temperature_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _running_schema():
    return binary_sensor.binary_sensor_schema(device_class=DEVICE_CLASS_RUNNING)


# Keys shared by every model.
_SHARED = {
    cv.Optional(CONF_COMPRESSOR_RUNNING): _running_schema(),
    cv.Optional(CONF_DEVICE_ON): binary_sensor.binary_sensor_schema(),
    cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_CONNECTIVITY,
    ),
}

C44_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(TrumaCoolerC44),
            cv.Optional(CONF_CLIMATE): climate.climate_schema(TrumaCoolerClimate),
            cv.Optional(CONF_TEMPERATURE): _temperature_schema(),
            cv.Optional(CONF_AMBIENT_TEMPERATURE): _temperature_schema(),
            cv.Optional(CONF_TURBO_RUNNING): _running_schema(),
            cv.Optional(CONF_TURBO): switch.switch_schema(TrumaCoolerSwitch),
            **_SHARED,
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

C69_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(TrumaCoolerC69),
            cv.Optional(CONF_CLIMATE_ZONE1): climate.climate_schema(TrumaCoolerClimate),
            cv.Optional(CONF_CLIMATE_ZONE2): climate.climate_schema(TrumaCoolerClimate),
            cv.Optional(CONF_TEMPERATURE_ZONE1): _temperature_schema(),
            cv.Optional(CONF_TEMPERATURE_ZONE2): _temperature_schema(),
            cv.Optional(CONF_POWER): switch.switch_schema(TrumaCoolerPowerSwitch),
            **_SHARED,
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

CONFIG_SCHEMA = cv.typed_schema(
    {"c44": C44_SCHEMA, "c69": C69_SCHEMA},
    key=CONF_MODEL,
    default_type="c44",
    lower=True,
)


async def _new_climate(config, key, var, zone, manages_power=True):
    if key not in config:
        return None
    clim = cg.new_Pvariable(config[key][CONF_ID])
    await climate.register_climate(clim, config[key])
    cg.add(clim.set_parent(var))
    cg.add(clim.set_zone(zone))
    cg.add(clim.set_manages_power(manages_power))
    return clim


async def _to_code_c44(var, config):
    clim = await _new_climate(config, CONF_CLIMATE, var, 0)
    if clim is not None:
        cg.add(var.set_climate(clim))
    if CONF_TEMPERATURE in config:
        cg.add(var.set_temperature_sensor(await sensor.new_sensor(config[CONF_TEMPERATURE])))
    if CONF_AMBIENT_TEMPERATURE in config:
        cg.add(var.set_ambient_temperature_sensor(await sensor.new_sensor(config[CONF_AMBIENT_TEMPERATURE])))
    if CONF_TURBO_RUNNING in config:
        cg.add(var.set_turbo_running_sensor(await binary_sensor.new_binary_sensor(config[CONF_TURBO_RUNNING])))
    if CONF_TURBO in config:
        sw = await switch.new_switch(config[CONF_TURBO])
        cg.add(sw.set_parent(var))
        cg.add(var.set_turbo_switch(sw))


async def _to_code_c69(var, config):
    # Zone climates are COOL-only; the master power switch owns global on/off.
    clim1 = await _new_climate(config, CONF_CLIMATE_ZONE1, var, 1, manages_power=False)
    if clim1 is not None:
        cg.add(var.set_climate_zone1(clim1))
    clim2 = await _new_climate(config, CONF_CLIMATE_ZONE2, var, 2, manages_power=False)
    if clim2 is not None:
        cg.add(var.set_climate_zone2(clim2))
    if CONF_TEMPERATURE_ZONE1 in config:
        cg.add(var.set_temperature_zone1_sensor(await sensor.new_sensor(config[CONF_TEMPERATURE_ZONE1])))
    if CONF_TEMPERATURE_ZONE2 in config:
        cg.add(var.set_temperature_zone2_sensor(await sensor.new_sensor(config[CONF_TEMPERATURE_ZONE2])))
    if CONF_POWER in config:
        sw = await switch.new_switch(config[CONF_POWER])
        cg.add(sw.set_parent(var))
        cg.add(var.set_power_switch(sw))


async def to_code(config):
    if not (CORE.is_esp32 and not CORE.using_arduino):
        raise cv.Invalid("truma_cooler requires ESP32 with the ESP-IDF framework")

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    # Shared entities.
    if CONF_COMPRESSOR_RUNNING in config:
        cg.add(var.set_compressor_running_sensor(
            await binary_sensor.new_binary_sensor(config[CONF_COMPRESSOR_RUNNING])))
    if CONF_DEVICE_ON in config:
        cg.add(var.set_device_on_sensor(
            await binary_sensor.new_binary_sensor(config[CONF_DEVICE_ON])))
    if CONF_CONNECTED in config:
        cg.add(var.set_connected_sensor(
            await binary_sensor.new_binary_sensor(config[CONF_CONNECTED])))

    if config[CONF_MODEL] == "c44":
        await _to_code_c44(var, config)
    else:
        await _to_code_c69(var, config)
