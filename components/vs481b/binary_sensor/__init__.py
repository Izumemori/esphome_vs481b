import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import DEVICE_CLASS_CONNECTIVITY, ENTITY_CATEGORY_DIAGNOSTIC

from .. import VS481BBaseSchema, vs481b_ns
from ..const import CONF_VS481B_ID

DEPENDENCIES = ["vs481b"]

connected_sensor = vs481b_ns.class_(
    "ConnectedSensor", binary_sensor.BinarySensor, cg.Component
)

CONF_CONNECTED = "connected"

CONFIG_SCHEMA = VS481BBaseSchema.extend(
    {
        cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
            connected_sensor,
            device_class=DEVICE_CLASS_CONNECTIVITY,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


async def to_code(config):
    if CONF_CONNECTED in config:
        var = await binary_sensor.new_binary_sensor(config[CONF_CONNECTED])
        await cg.register_component(var, config[CONF_CONNECTED])

        parent = await cg.get_variable(config[CONF_VS481B_ID])
        cg.add(var.set_parent(parent))
