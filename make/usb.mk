# USB device configuration.
# USB_PRODUCT is set in each board's own makefile since it varies per board.
# Manufacturer name
USB_MANUFACTURER ?= "Teensyduino"
# Vendor ID
USB_VID ?= 0x16C0
# Product ID
USB_PID ?= 0x0483
# Device version number (0x1234 => ver 12.34)
USB_DEVICE_VERSION ?= 0x0100
# Max power draw in mA
USB_MAX_POWER ?= 500
# Number of supported configurations
USB_NUM_CONFIGURATIONS ?= 1
# USB protocol version supported (0x0200 => USB 2.00)
USB_VERSION ?= 0x0200
# USB device class, subclass, protocol
USB_DEVICE_CLASS ?= USB_CLASS_PER_INTERFACE
USB_DEVICE_SUBCLASS ?= 0
USB_DEVICE_PROTOCOL ?= 0
# Max endpoints we want to use. Must be at least 1, for the control endpoint,
# and no more than 16. We can save a bit of memory by reducing this if we don't
# want to use all 16.
USB_MAX_ENDPOINTS ?= 16

ifdef USB_SERIALNO
	# no default serial number.
	DEFS += -DUSB_SERIALNO=u\"$(USB_SERIALNO)\"
endif

ifeq ($(shell test $(USB_MAX_POWER) -gt 500; echo $$?),0)
# this can't be indented because reasons
$(warning Warning: USB_MAX_POWER=$(USB_MAX_POWER) violates USB specifications (maximum is 500mA))
endif

DEFS += -DUSB_PRODUCT=u\"$(USB_PRODUCT)\"
DEFS += -DUSB_MANUFACTURER=u\"$(USB_MANUFACTURER)\"
DEFS += -DUSB_VID=$(USB_VID)
DEFS += -DUSB_PID=$(USB_PID)
DEFS += -DUSB_DEVICE_VERSION=$(USB_DEVICE_VERSION)
DEFS += -DUSB_MAX_POWER=$(USB_MAX_POWER)
DEFS += -DUSB_NUM_CONFIGURATIONS=$(USB_NUM_CONFIGURATIONS)
DEFS += -DUSB_VERSION=$(USB_VERSION)
DEFS += -DUSB_DEVICE_CLASS=$(USB_DEVICE_CLASS)
DEFS += -DUSB_DEVICE_SUBCLASS=$(USB_DEVICE_SUBCLASS)
DEFS += -DUSB_DEVICE_PROTOCOL=$(USB_DEVICE_PROTOCOL)
DEFS += -DUSB_MAX_ENDPOINTS=$(USB_MAX_ENDPOINTS)

ifeq ($(shell test $(USB_MAX_ENDPOINTS) -gt 16; echo $$?),0)
$(error Error: USB_MAX_ENDPOINTS cannot be > 16)
endif
ifeq ($(shell test $(USB_MAX_ENDPOINTS) -lt 1; echo $$?),0)
$(error Error: USB_MAX_ENDPOINTS cannot be < 1)
endif
