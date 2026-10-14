import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from .. import VS481BBaseSchema, vs481b_ns
from ..const import CONF_VS481B_ID

DEPENDENCIES = ["vs481b"]

firmware_sensor = vs481b_ns.class_(
    "FirmwareSensor", text_sensor.TextSensor, cg.Component
)

CONF_FIRMWARE = "firmware"

CONFIG_SCHEMA = VS481BBaseSchema.extend(
    {
        cv.Optional(CONF_FIRMWARE): text_sensor.text_sensor_schema(
            firmware_sensor
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


async def to_code(config):
    if CONF_FIRMWARE in config:
        var = await text_sensor.new_text_sensor(config[CONF_FIRMWARE])
        await cg.register_component(var, config[CONF_FIRMWARE])

        parent = await cg.get_variable(config[CONF_VS481B_ID])
        cg.add(var.set_parent(parent))
