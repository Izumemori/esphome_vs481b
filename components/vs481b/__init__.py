import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

from .const import (
    CONF_VS481B_ID
)

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

def unique_port_names(ports):
    # The port selects map names to port numbers, so a duplicate would select the wrong input
    seen = set()
    for name in ports:
        if name in seen:
            raise cv.Invalid(f"Port name '{name}' is used more than once")
        seen.add(name)
    return ports


def validate_defaults(config):
    defaults = config[CONF_DEFAULTS]
    port_count = len(config[CONF_PORTS])

    for key in (CONF_DEFAULT_PORT, CONF_DEFAULT_AUTO_PORT):
        if key in defaults and not 1 <= defaults[key] <= port_count:
            raise cv.Invalid(
                f"Must be between 1 and {port_count}, the number of configured ports",
                path=[CONF_DEFAULTS, key],
            )

    # Auto mode is entered by selecting the auto port, so it needs one to start from
    if defaults.get(CONF_DEFAULT_MODE) == 'AUTO' and CONF_DEFAULT_AUTO_PORT not in defaults:
        raise cv.Invalid(
            f"Mode AUTO requires '{CONF_DEFAULT_AUTO_PORT}' to be set as well",
            path=[CONF_DEFAULTS, CONF_DEFAULT_MODE],
        )

    return config


CONFIG_SCHEMA = cv.All(
    uart.UART_DEVICE_SCHEMA.extend({
        cv.GenerateID(): cv.declare_id(vs841b_class),
        cv.Required(CONF_PORTS): cv.All(
            cv.ensure_list(cv.string_strict), cv.Length(min=1), unique_port_names
        ),
        cv.Optional(CONF_DEFAULTS, default={}): cv.Schema({
            cv.Optional(CONF_DEFAULT_PORT): cv.int_,
            cv.Optional(CONF_DEFAULT_OUTPUT): cv.boolean,
            cv.Optional(CONF_DEFAULT_MODE): cv.enum(mode_mapping, upper=True),
            cv.Optional(CONF_DEFAULT_AUTO_PORT): cv.int_,
            cv.Optional(CONF_DEFAULT_GOTO): cv.boolean,
            cv.Optional(CONF_DEFAULT_HPD): cv.boolean
        })
    }).extend(cv.polling_component_schema("5s")),
    validate_defaults,
)

# Serial settings from the VS481B manual: 19200 baud, 8 data bits, no parity, 1 stop bit
FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    'vs481b',
    baud_rate=19200,
    require_tx=True,
    require_rx=True,
    data_bits=8,
    parity='NONE',
    stop_bits=1,
)

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
        ('hpd_enabled', defaults[CONF_DEFAULT_HPD] if CONF_DEFAULT_HPD in defaults else None)
    )

    cg.add(var.register_ports(config[CONF_PORTS]))

    cg.add(var.register_defaults(default_settings))