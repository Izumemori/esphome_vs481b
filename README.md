An [Esphome](https://esphome.io/) external component for controlling the [ATEN VS481B (and similar)](https://www.aten.com/eu/en/products/professional-audiovideo/video-switches/vs481b/) HDMI Switch via its serial port.

- [Confirmed Working HDMI Switches](#confirmed-working-hdmi-switches)
- [Minimal Base Configuration](#minimal-base-configuration)
- [Features](#features)
  - [Port selection](#port-selection)
    - [Default configuration](#default-configuration)
    - [Select configuration](#select-configuration)
  - [Mode selection](#mode-selection)
      - [Next](#next)
      - [Default](#default)
      - [Auto](#auto)
    - [Default configuration](#default-configuration-1)
    - [Select configuration](#select-configuration-1)
  - [Auto port selection](#auto-port-selection)
    - [Default configuration](#default-configuration-2)
    - [Select configuration](#select-configuration-2)
  - [Goto](#goto)
    - [Default configuration](#default-configuration-3)
    - [Switch configuration](#switch-configuration)
  - [Output](#output)
    - [Default configuration](#default-configuration-4)
    - [Switch configuration](#switch-configuration-1)
  - [Hot Plug detect](#hot-plug-detect)
    - [Default configuration](#default-configuration-5)
    - [Switch configuration](#switch-configuration-2)
    - [Read Out Firmware Version](#read-out-firmware-version)
- [Full example](#full-example)
- [Contributing](#contributing)
- [Specification](#specification)


### Confirmed Working HDMI Switches
I currently only own a VS481B, but in theory a lot of (whitelabelled) ATEN switches should be compatible.

### Minimal Base Configuration
This component requires an existing `uart` component with proper values for communicating with the HDMI switch
```yaml
<standard esphome configuration ...>

# A uart component is required to be configured!
uart:
    baud_rate: 19200
    tx_pin:
        number: 16
        invert: false
    rx_pin:
        number: 17
        invert: false
    data_bits: 8
    stop_bits: 1
    parity: NONE

vs481b:
    # Simultaneously configures the amount and names for the switch ports
    # This is required to be set!
    ports: 
        - Laptop # Port 1
        - Nintendo Switch # Port 2
        - Xbox # Port 3
        - Hotplug # Port 4
```

### Features
#### Port selection
You can select which port should be active via this component

##### Default configuration
A default selected port can be set via `vs481b.defaults.port`
The value set cannot exceed 
```yaml
vs481b:
    <...>
    defaults:
        <...>
        port: 1
```
##### Select configuration
You can expose an [esphome select component](https://esphome.io/components/select/index.html) to control the selected port
```yaml
select:
  - platform: vs481b
    ports:
      name: "Ports"
      <... further select component configuration>
```

#### Mode selection
The switch supports 3 "Switch Modes"
###### Next
Automatically switches to the next port that has a new device connected to it
###### Default
Disables automatic switching
###### Auto
Switches to a pre-defined port and prevents changing of the port until the device is either disconnected or auto mode turned off.
This requires the auto port to be set via a default configuration and/or exposing a select for it.
You can see how [here](#auto-port-selection).

##### Default configuration
A default mode can be set via `vs481b.defaults.mode`
The value set cannot exceed 
```yaml
vs481b:
    <...>
    defaults:
        <...>
        mode: Default # also possible: Auto, Next
```
##### Select configuration
You can expose an [esphome select component](https://esphome.io/components/select/index.html) to control the mode
```yaml
select:
  - platform: vs481b
    mode:
      name: "Mode"
      <... further select component configuration>
```

#### Auto port selection
This is required to use the [auto](#auto) switch mode.
##### Default configuration
A default auto port can be set via `vs481b.defaults.goto`
```yaml
vs481b:
    <...>
    defaults:
        <...>
        auto_port: 1
```
##### Select configuration
You can expose an [esphome select component](https://esphome.io/components/select/index.html) to control the mode
```yaml
select:
  - platform: vs481b
    auto_port:
      name: "Auto Port"
      <... further select component configuration>
```

#### Goto
The switch has a goto function, which when the current source device goes offline, switches to the next port with an active signal
##### Default configuration
A default state for this feature can be set via `vs481b.defaults.goto`
```yaml
vs481b:
    <...>
    defaults:
        <...>
        # This is an optional boolean value
        goto: on
```
##### Switch configuration
The Goto feature can be exposed as an [esphome switch component](https://esphome.io/components/switch/index.html) using the `vs481b` platform
```yaml
switch:
    - platform: vs481b
      goto:
        name: "Goto"
        <... other esphome switch parameters>
```
#### Output
The switch allows for toggling the output to the display
##### Default configuration
A default state for this feature can be set via `vs481b.defaults.output`
```yaml
vs481b:
    ...
    defaults:
        ...
        # This is an optional boolean value
        output: on
```
##### Switch configuration
The display output state can be exposed as an [esphome switch component](https://esphome.io/components/switch/index.html) using the `vs481b` platform
```yaml
switch:
    - platform: vs481b
      output:
        name: "Output"
        ... other esphome switch parameters
```
#### Hot Plug detect
You can force source devices to read the EDID of the display connected to the switch.
Forcing devices to read the EDID information can be somewhat slower.
##### Default configuration
A default state for this feature can be set via `vs481b.defaults.output`
```yaml
vs481b:
    ...
    defaults:
        ...
        # This is an optional boolean value
        hpd: on
```
##### Switch configuration
You can expose the Hot Plug detect parameter as an [esphome switch component](https://esphome.io/components/switch/index.html) using the `vs481b` platform.  
:warning: I'd recommend against it! :warning:  
There is no way to read back the setting from the switch and as such the state of the switch will be lost after a reboot of the micro controller.
```yaml
switch:
    - platform: vs481b
      hpd:
        name: "Hot Plug detect"
        ... other esphome switch parameters
```

##### Read Out Firmware Version
To read out the firmware version you can add a [TextSensor](https://esphome.io/components/text_sensor/index.html)
```yaml
text_sensor:
  - platform: vs481b
    firmware:
      name: "Firmware version"
```

### Full example
An example can be found in [the example file](example_vs481b.yaml)

### Contributing
Contributions are always welcome

### Specification
This external component is designed around this [product manual](https://assets.aten.com/product/manual/vs481b-s_2014-10-09.pdf)