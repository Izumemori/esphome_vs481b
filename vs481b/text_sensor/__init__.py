import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from .. import vs481b_ns, VS481BBaseSchema
from ..const import (
    CONF_VS481B_ID
)

DEPENDENCIES = ['vs481b']

port_select = vs481b_ns.class_('FirmwareSensor', text_sensor.TextSensor, cg.Component)

CONF_FIRMWARE = "firmware"

CONFIG_SCHEMA = VS481BBaseSchema.extend({
    cv.Optional(CONF_FIRMWARE): text_sensor.text_sensor_schema().extend({
        cv.GenerateID(): cv.declare_id(port_select),
    })
})

async def to_code(config):
    if CONF_FIRMWARE in config:
        var = cg.new_Pvariable(config[CONF_FIRMWARE][CONF_ID])
        await cg.register_component(var, config)
        await text_sensor.register_text_sensor(var, config[CONF_FIRMWARE])

        parent = await cg.get_variable(config[CONF_VS481B_ID])

        cg.add(var.set_parent(parent))
