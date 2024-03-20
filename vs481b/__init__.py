import esphome.codegen as cg
import esphome.config_validation as cv
import voluptuous as vol
from esphome.components import uart, select
from esphome.const import CONF_ID, ICON_EMPTY, UNIT_EMPTY

from .const import (
    CONF_VS481B_ID
)

Invalid = vol.Invalid

DEPENDENCIES = ['uart']

vs481b_ns = cg.esphome_ns.namespace('vs481b')
vs841b_class = vs481b_ns.class_('VS481B', cg.PollingComponent, uart.UARTDevice)

settings_struct = vs481b_ns.struct('Settings')
mode_enum = vs481b_ns.enum('Mode')

mode_mapping = {
    'NEXT': mode_enum.M_NEXT,
    'AUTO': mode_enum.M_AUTO,
    'DEFAULT': mode_enum.M_DEFAULT
}

CONF_PORTS = 'ports'
CONF_DEFAULTS = 'defaults'
CONF_DEFAULT_PORT = 'port'
CONF_DEFAULT_OUTPUT = 'output'
CONF_DEFAULT_MODE = 'mode'
CONF_DEFAULT_AUTO_PORT = 'auto_port'
CONF_DEFAULT_GOTO = 'goto'
CONF_DEFAULT_HPD = 'hpd'

CONFIG_SCHEMA = uart.UART_DEVICE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(vs841b_class),
    cv.Required(CONF_PORTS): cv.ensure_list(cv.string),
    cv.Optional(CONF_DEFAULTS): cv.Schema({
        cv.Optional(CONF_DEFAULT_PORT): cv.int_,
        cv.Optional(CONF_DEFAULT_OUTPUT): cv.boolean,
        cv.Optional(CONF_DEFAULT_MODE): cv.enum(mode_mapping, upper=True),
        cv.Optional(CONF_DEFAULT_AUTO_PORT): cv.int_,
        cv.Optional(CONF_DEFAULT_GOTO): cv.boolean,
        cv.Optional(CONF_DEFAULT_HPD, False): cv.boolean
    })
}).extend(cv.polling_component_schema("5s"))

VS481BBaseSchema = cv.Schema({
    cv.GenerateID(CONF_VS481B_ID): cv.use_id(vs841b_class)
}
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await uart.register_uart_device(var, config)
    await cg.register_component(var, config)
    
    defaults = config[CONF_DEFAULTS]
    
    default_settings = cg.StructInitializer(
        settings_struct,
        ('port', defaults[CONF_DEFAULT_PORT] if CONF_DEFAULT_PORT in defaults else None),
        ('output_enabled', defaults[CONF_DEFAULT_OUTPUT] if CONF_DEFAULT_OUTPUT in defaults else None),
        ('mode', defaults[CONF_DEFAULT_MODE] if CONF_DEFAULT_MODE in defaults else None),
        ('auto_port', defaults[CONF_DEFAULT_AUTO_PORT] if CONF_DEFAULT_AUTO_PORT in defaults else None),
        ('goto_enabled', defaults[CONF_DEFAULT_GOTO] if CONF_DEFAULT_GOTO in defaults else None),
        ('hpd_enabled', config[CONF_DEFAULTS][CONF_DEFAULT_HPD])
    )

    cg.add(var.register_ports(config[CONF_PORTS]))

    cg.add(var.register_defaults(default_settings))