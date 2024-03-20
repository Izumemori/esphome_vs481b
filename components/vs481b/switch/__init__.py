import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID
from .. import vs481b_ns, VS481BBaseSchema
from ..const import (
    CONF_VS481B_ID
)

DEPENDENCIES = ['vs481b']

goto_switch = vs481b_ns.class_('GotoSwitch', switch.Switch, cg.Component)
output_switch = vs481b_ns.class_('OutputSwitch', switch.Switch, cg.Component)
hpd_switch = vs481b_ns.class_('HPDSwitch', switch.Switch, cg.Component)

CONF_GOTO = "goto"
CONF_OUTPUT = "output"
CONF_HPD = "hpd"

CONFIG_SCHEMA = VS481BBaseSchema.extend({
    cv.Optional(CONF_GOTO): switch.switch_schema().extend({
        cv.GenerateID(): cv.declare_id(goto_switch),
    }),
    cv.Optional(CONF_OUTPUT): switch.switch_schema().extend({
        cv.GenerateID(): cv.declare_id(output_switch)
    }),
    cv.Optional(CONF_HPD): switch.switch_schema().extend({
        cv.GenerateID(): cv.declare_id(hpd_switch)
    })
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_VS481B_ID])

    if CONF_GOTO in config:
        var = cg.new_Pvariable(config[CONF_GOTO][CONF_ID])
        await cg.register_component(var, config)
        await switch.register_switch(var, config[CONF_GOTO])


        cg.add(var.set_parent(parent))

    if CONF_OUTPUT in config:
        var = cg.new_Pvariable(config[CONF_OUTPUT][CONF_ID])
        await cg.register_component(var, config)
        await switch.register_switch(var, config[CONF_OUTPUT])

        cg.add(var.set_parent(parent))

    if CONF_HPD in config:
        var = cg.new_Pvariable(config[CONF_HPD][CONF_ID])
        await cg.register_component(var, config)
        await switch.register_switch(var, config[CONF_HPD])

        cg.add(var.set_parent(parent))
