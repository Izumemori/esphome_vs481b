import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ID
from .. import vs481b_ns, VS481BBaseSchema
from ..const import (
    CONF_VS481B_ID
)

DEPENDENCIES = ['vs481b']

port_select = vs481b_ns.class_('PortSelect', select.Select, cg.Component)
mode_select = vs481b_ns.class_('ModeSelect', select.Select, cg.Component)
auto_port_select = vs481b_ns.class_('AutoPortSelect', select.Select, cg.Component)

CONF_PORTS = "ports"
CONF_MODE = "mode"
CONF_AUTO_PORT = "auto_port"

CONFIG_SCHEMA = VS481BBaseSchema.extend({
    cv.Optional(CONF_PORTS): select.select_schema().extend({
        cv.GenerateID(): cv.declare_id(port_select),
    }),
    cv.Optional(CONF_MODE): select.select_schema().extend({
        cv.GenerateID(): cv.declare_id(mode_select),
    }),
    cv.Optional(CONF_AUTO_PORT): select.select_schema().extend({
        cv.GenerateID(): cv.declare_id(auto_port_select),
    })
})

async def to_code(config):
    if CONF_PORTS in config:
        var = cg.new_Pvariable(config[CONF_PORTS][CONF_ID])
        await cg.register_component(var, config)
        await select.register_select(var, config[CONF_PORTS], options=[])

        parent = await cg.get_variable(config[CONF_VS481B_ID])

        cg.add(var.set_parent(parent))
    if CONF_MODE in config:
        var = cg.new_Pvariable(config[CONF_MODE][CONF_ID])
        await cg.register_component(var, config)
        await select.register_select(var, config[CONF_MODE], options=[])

        parent = await cg.get_variable(config[CONF_VS481B_ID])

        cg.add(var.set_parent(parent))
    if CONF_AUTO_PORT in config:
        var = cg.new_Pvariable(config[CONF_AUTO_PORT][CONF_ID])
        await cg.register_component(var, config)
        await select.register_select(var, config[CONF_AUTO_PORT], options=[])

        parent = await cg.get_variable(config[CONF_VS481B_ID])

        cg.add(var.set_parent(parent))
