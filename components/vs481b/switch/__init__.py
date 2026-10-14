import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch

from .. import VS481BBaseSchema, vs481b_ns
from ..const import CONF_VS481B_ID

DEPENDENCIES = ["vs481b"]

goto_switch = vs481b_ns.class_("GotoSwitch", switch.Switch, cg.Component)
output_switch = vs481b_ns.class_("OutputSwitch", switch.Switch, cg.Component)
hpd_switch = vs481b_ns.class_("HPDSwitch", switch.Switch, cg.Component)

CONF_GOTO = "goto"
CONF_OUTPUT = "output"
CONF_HPD = "hpd"

CONFIG_SCHEMA = VS481BBaseSchema.extend(
    {
        cv.Optional(CONF_GOTO): switch.switch_schema(
            goto_switch, default_restore_mode="DISABLED"
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_OUTPUT): switch.switch_schema(
            output_switch, default_restore_mode="DISABLED"
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_HPD): switch.switch_schema(
            hpd_switch, default_restore_mode="DISABLED"
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_VS481B_ID])

    for key in (CONF_GOTO, CONF_OUTPUT, CONF_HPD):
        if key not in config:
            continue
        var = await switch.new_switch(config[key])
        await cg.register_component(var, config[key])
        cg.add(var.set_parent(parent))
