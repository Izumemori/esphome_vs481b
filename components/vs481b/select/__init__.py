import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select

from .. import VS481BBaseSchema, vs481b_ns
from ..const import CONF_VS481B_ID

DEPENDENCIES = ["vs481b"]

port_select = vs481b_ns.class_("PortSelect", select.Select, cg.Component)
mode_select = vs481b_ns.class_("ModeSelect", select.Select, cg.Component)
auto_port_select = vs481b_ns.class_("AutoPortSelect", select.Select, cg.Component)

CONF_PORTS = "ports"
CONF_MODE = "mode"
CONF_AUTO_PORT = "auto_port"

CONFIG_SCHEMA = VS481BBaseSchema.extend(
    {
        cv.Optional(CONF_PORTS): select.select_schema(port_select).extend(
            cv.COMPONENT_SCHEMA
        ),
        cv.Optional(CONF_MODE): select.select_schema(mode_select).extend(
            cv.COMPONENT_SCHEMA
        ),
        cv.Optional(CONF_AUTO_PORT): select.select_schema(auto_port_select).extend(
            cv.COMPONENT_SCHEMA
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_VS481B_ID])

    # Options are set at runtime in each select's setup(): the port names from the
    # parent's `ports` list, the mode names from mode_to_str() in mode.h
    for key in (CONF_PORTS, CONF_MODE, CONF_AUTO_PORT):
        if key not in config:
            continue
        var = await select.new_select(config[key], options=[])
        await cg.register_component(var, config[key])
        cg.add(var.set_parent(parent))
