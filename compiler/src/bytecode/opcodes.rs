pub const EXIT: u8        = 0x00;
pub const STORE: u8       = 0x01;
pub const PRINT: u8       = 0x02;
#[allow(dead_code)]
pub const TEST_EQ: u8     = 0x03;
#[allow(dead_code)]
pub const TEST_GT: u8     = 0x04;
#[allow(dead_code)]
pub const TEST_GTE: u8    = 0x05;
#[allow(dead_code)]
pub const TEST_LT: u8     = 0x06;
#[allow(dead_code)]
pub const TEST_LTE: u8    = 0x07;
pub const ADD: u8         = 0x08;
pub const SUB: u8         = 0x09;
pub const CALL: u8        = 0x0a;
pub const RETURN: u8      = 0x0b;
pub const MOV: u8         = 0x0c;
pub const JMP: u8         = 0x0d;
pub const TUPLE: u8       = 0x0e;
pub const TUPLE_NTH: u8   = 0xf;
pub const ASSERT_EQ: u8   = 0x10;
pub const VECTOR: u8      = 0x11;
pub const PUB_FN: u8      = 0x12;
pub const STORE_TRUE: u8  = 0x13;
pub const STORE_FALSE: u8 = 0x14;
pub const TEST: u8        = 0x15;
pub const EQ: u8          = 0x16;
pub const NOT_EQ: u8      = 0x17;
