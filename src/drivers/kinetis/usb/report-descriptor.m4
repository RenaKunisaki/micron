dnl ----------------------------------------------------------------------------
dnl -- This is a GNU M4 macro file.
dnl -- It's a complex set of (sometimes self-referential) macros which parse
dnl -- report-descriptor.c.m4 and output a proper C source file.
dnl -- This was needed because CPP macros can't expand to a variable number
dnl -- of elements.
dnl ----------------------------------------------------------------------------

dnl -- Extract specified byte from an integer.
dnl -- $1 = integer to extract
dnl -- $2 = byte #, 0 (LSB) to 3 (MSB)
define(`BYTE', `eval((($1 >> (8 * $2)) & 0xFF))')

dnl -- Expands to an integer with only the specified bit set.
dnl -- $1 = bit index, 0 (LSB) to 31
define(`BIT', `eval(1 << $1)')

dnl -- Expands to the logical OR of all its arguments.
define(`OR', `eval(translit(`$*',`,',`|'))')

dnl -- Expands an integer to 1, 2 or 4 individual bytes,
dnl -- as many as are needed to represent it.
define(`B8',  `BYTE($1, 0)')
define(`B16', `BYTE($1, 0), BYTE($1, 1)')
define(`B32', `BYTE($1, 0), BYTE($1, 1), BYTE($1, 2), BYTE($1, 3)')

dnl -- Expands to a USB HID report descriptor tag.
define(`TAG_BYTES',
  `ifelse(eval($2 > 32767 || $2 < -32768), `1', `eval($1 | 3), B32($2)',
    `ifelse(eval($2 > 127 || $2 < -128), `1', `eval($1 | 2), B16($2)',
      `eval($1 | 1), B8($2)')')')

dnl -- Used by meta-macros below to build the flag lists.
define(`TAG_OR', `TAG_BYTES($1, OR(shift($*)))')

dnl -- Meta-macro for defining tags.
dnl -- $1 = tag name.
dnl -- $2 = tag value.
dnl -- Defines the given name as a macro which expands to
dnl -- the given value followed by the logical OR of its parameters, if any.
dnl -- warning: awful hack. repeatedly uses `' in order to delay macro
dnl -- expansion, and + 0 to make OR() not barf with no arguments.
dnl -- The resulting macro can be used for tags like INPUT which take a series
dnl -- of flags as parameter, COLLECTION which takes a single integer parameter,
dnl -- or END_COLLECTION which takes no parameters. However it's possible to
dnl -- pass any number of parameters to all of those and they'll output the
dnl -- logical OR of those parameters and set the size bits in the tag byte
dnl -- appropriately. Make sure you're giving the correct parameters.
define(`TAG', `
	define($1,
		i`'felse(`$'`#',0,$2, `TA`'G_OR($2, $'`* + 0)') `,'
	)
')

dnl -- Macro for defining constants.
define(`CONST', `define(`$1', `$2')')

dnl -- Macro for defining flags.
dnl -- $1 = bit index of this flag, 0 (LSB) to 31
dnl -- $2 = name to use to set this flag
dnl -- $3 (optional) = name to use to not set this flag
dnl -- e.g. FLAG(7, FOO, BAR) expands to:
dnl -- define(FOO, BIT(7))
dnl -- define(BAR, 0)
dnl -- The BAR definition can help make your intentions more explicit.
define(`FLAG', `
	define(`$2', `BIT($1)')
	ifelse(`$#', 3, `define(`$3', 0)')
')

dnl -- Main item tags
TAG( INPUT,          0x80)
TAG( OUTPUT,         0x90)
TAG( FEATURE,        0xB0)
TAG( COLLECTION,     0xA0)
TAG( END_COLLECTION, 0xC0)

dnl -- Flags for INPUT, OUTPUT, FEATURE
dnl  bit     1                0
FLAG( 0, CONSTANT,           DATA)
FLAG( 1, VARIABLE,           ARRAY)
FLAG( 2, RELATIVE,           ABSOLUTE)
FLAG( 3, WRAP,               NO_WRAP)
FLAG( 4, NON_LINEAR,         LINEAR)
FLAG( 5, NO_PREFFERED_STATE, PREFFERED_STATE)
FLAG( 6, NULL_STATE,         NO_NULL_STATE)
FLAG( 7, VOLATILE,           NON_VOLATILE)
FLAG( 8, BUFFERED_BYTES,     BIT_FIELD)

dnl -- Usage pages
dnl -- For individual usage codes,
dnl -- refer to http://www.usb.org/developers/hidpage/Hut1_12v2.pdf
dnl -- There are a ton of them and I don't feel like copying them all here.
CONST( GENERIC_DESKTOP,    0x01)
CONST( SIM_CONTROL,        0x02)
CONST( VR_CONTROL,         0x03)
CONST( SPORT_CONTROL,      0x04)
CONST( GAME_CONTROL,       0x05)
CONST( GENERIC_CONTROL,    0x06)
CONST( KEYBOARD,           0x07)
CONST( LED,                0x08)
CONST( BUTTON,             0x09)
CONST( ORDINAL,            0x0A)
CONST( TELEPHONY,          0x0B)
CONST( CONSUMER,           0x0C)
CONST( DIGITIZER,          0x0D)
CONST( UNICODE,            0x10)
CONST( ALNUM_DISPLAY,      0x14)
CONST( MEDICAL_INSTRUMENT, 0x40)

dnl -- Collection types
CONST( PHYSICAL,       0x00)
CONST( APPLICATION,    0x01)
CONST( LOGICAL,        0x02)
CONST( REPORT,         0x03)
CONST( NAMED_ARRAY,    0x04)
CONST( USAGE_SWITCH,   0x05)
CONST( USAGE_MODIFIER, 0x06)

dnl -- Global items
TAG( USAGE_PAGE,       0x04)
TAG( LOGICAL_MINIMUM,  0x14)
TAG( LOGICAL_MAXIMUM,  0x24)
TAG( PHYSICAL_MINIMUM, 0x34)
TAG( PHYSICAL_MAXIMUM, 0x44)
TAG( UNIT_EXPONENT,    0x54)
TAG( UNIT,             0x64)
TAG( REPORT_SIZE,      0x74)
TAG( REPORT_ID,        0x84)
TAG( REPORT_COUNT,     0x94)
TAG( PUSH,             0xA4)
TAG( POP,              0xB4)

dnl -- Local items
TAG( USAGE,              0x08)
TAG( USAGE_MINIMUM,      0x18)
TAG( USAGE_MAXIMUM,      0x28)
TAG( DESIGNATOR_INDEX,   0x38)
TAG( DESIGNATOR_MINIMUM, 0x48)
TAG( DESIGNATOR_MAXIMUM, 0x58)
dnl -- 0x68 is conspicuously missing.
TAG( STRING_INDEX,       0x78)
TAG( STRING_MINIMUM,     0x88)
TAG( STRING_MAXIMUM,     0x98)
TAG( DELIMITER,          0xA8)
define(`DELIMITER_OPEN',  `DELIMITER(1)')
define(`DELIMITER_CLOSE', `DELIMITER(0)')


dnl -- Units
dnl -- Unit exponent, -8 to 7
define(`EXPONENT', `ifelse(eval($1 < 0), `1', eval($1 + 16), $1)')
define(`LENGTH',             `eval((EXPONENT($1) & 0xF) <<  4)')
define(`MASS',               `eval((EXPONENT($1) & 0xF) <<  8)')
define(`TIME',               `eval((EXPONENT($1) & 0xF) << 12)')
define(`TEMPERATURE',        `eval((EXPONENT($1) & 0xF) << 16)')
define(`CURRENT',            `eval((EXPONENT($1) & 0xF) << 20)')
define(`LUMINOUS_INTENSITY', `eval((EXPONENT($1) & 0xF) << 24)')

dnl -- Unit systems
CONST( SI_LINEAR,        1)
CONST( SI_ROTATION,      2)
CONST( ENGLISH_LINEAR,   3)
CONST( ENGLISH_ROTATION, 4)
CONST( CENTIMETER,       1)
CONST( RADIANS,          2)
CONST( INCH,             3)
CONST( DEGREES,          4)
CONST( GRAM,             1)
CONST( SLUG,             3)
CONST( SECONDS,          1)
CONST( KELVIN,           1)
CONST( FAHRENHEIT,       3)
CONST( AMPERE,           1)
CONST( CANDELA,          1)

dnl -- Bias values.
CONST( RIGHT_HAND,  1)
CONST( LEFT_HAND,   2)
CONST( BOTH_HANDS,  3)
CONST( EITHER_HAND, 4)

dnl -- Body part designators
CONST( HAND,          0x01)
CONST( EYEBALL,       0x02)
CONST( EYEBROW,       0x03)
CONST( EYELID,        0x04)
CONST( EAR,           0x05)
CONST( NOSE,          0x06)
CONST( MOUTH,         0x07)
CONST( UPPER_LIP,     0x08)
CONST( LOWER_LIP,     0x09)
CONST( JAW,           0x0A)
CONST( NECK,          0x0B)
CONST( UPPER_ARM,     0x0C)
CONST( ELBOW,         0x0D)
CONST( FOREARM,       0x0E)
CONST( WRIST,         0x0F)
CONST( PALM,          0x10)
CONST( THUMB,         0x11)
CONST( INDEX_FINGER,  0x12)
CONST( MIDDLE_FINGER, 0x13)
CONST( RING_FINGER,   0x14)
CONST( LITTLE_FINGER, 0x15)
CONST( HEAD,          0x16)
CONST( SHOULDER,      0x17)
CONST( HIP,           0x18)
CONST( WAIST,         0x19)
CONST( THIGH,         0x1A)
CONST( KNEE,          0x1B)
CONST( CALF,          0x1C)
CONST( ANKLE,         0x1D)
CONST( FOOT,          0x1E)
CONST( HEEL,          0x1F)
CONST( BALL_OF_FOOT,  0x20)
CONST( BIG_TOE,       0x21)
CONST( SECOND_TOE,    0x22)
CONST( THIRD_TOE,     0x23)
CONST( FOURTH_TOE,    0x24)
CONST( LITTLE_TOE,    0x25)
CONST( BROW,          0x26)
CONST( CHEEK,         0x27)

dnl -- Which side of the body
CONST( RIGHT_SIDE,  1)
CONST( LEFT_SIDE,   2)
CONST( BOTH_SIDES,  3)
CONST( EITHER_SIDE, 4)
CONST( CENTER,      5)

dnl -- Undefine helper macros so they can't possibly be triggered
dnl -- accidentally by the actual code.
undefine(`TAG')
undefine(`CONST')
undefine(`FLAG')
