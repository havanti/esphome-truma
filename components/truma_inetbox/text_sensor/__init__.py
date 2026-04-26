from esphome.components import text_sensor
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID, ENTITY_CATEGORY_DIAGNOSTIC
from .. import truma_inetbox_ns

DEPENDENCIES = ["truma_inetbox"]
CODEOWNERS = ["@havanti"]

TrumaVersionTextSensor = truma_inetbox_ns.class_(
    "TrumaVersionTextSensor", text_sensor.TextSensor, cg.Component
)

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon="mdi:tag",
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(TrumaVersionTextSensor),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)
