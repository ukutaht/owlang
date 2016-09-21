pub const EXIT: u8            = 0x00;
pub const STORE_INT: u8       = 0x01;
pub const PRINT: u8           = 0x02;
pub const ADD: u8             = 0x03;
pub const SUB: u8             = 0x04;
pub const CALL: u8            = 0x05;
pub const RETURN: u8          = 0x06;
pub const MOV: u8             = 0x07;
pub const JMP: u8             = 0x08;
pub const TUPLE: u8           = 0x09;
pub const TUPLE_NTH: u8       = 0x0a;
pub const LIST: u8            = 0x0b;
pub const PUB_FN: u8          = 0x0c;
pub const STORE_TRUE: u8      = 0x0d;
pub const STORE_FALSE: u8     = 0x0e;
pub const TEST: u8            = 0x0f;
pub const EQ: u8              = 0x10;
pub const NOT_EQ: u8          = 0x11;
pub const NOT: u8             = 0x12;
pub const STORE_NIL: u8       = 0x13;
pub const GREATER_THAN: u8    = 0x14;
pub const LOAD_STRING: u8     = 0x15;
pub const FILE_PWD: u8        = 0x16;
pub const CONCAT: u8          = 0x17;
pub const FILE_LS: u8         = 0x18;
pub const CAPTURE: u8         = 0x19;
pub const CALL_LOCAL: u8      = 0x1a;
pub const LIST_NTH: u8        = 0x1b;
pub const LIST_COUNT: u8      = 0x1c;
pub const LIST_SLICE: u8      = 0x1d;
pub const STRING_SLICE: u8    = 0x1e;
pub const CODE_LOAD: u8       = 0x1f;
pub const FUNCTION_NAME: u8   = 0x20;
pub const STRING_COUNT: u8    = 0x21;
pub const STRING_CONTAINS: u8 = 0x22;
pub const TO_STRING: u8       = 0x23;
pub const ANON_FN: u8         = 0x24;
pub const GET_UPVAL: u8       = 0x25;
