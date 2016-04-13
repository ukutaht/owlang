pub const EXIT: u8 = 0x00;
pub const STORE: u8 = 0x01;
pub const PRINT: u8 = 0x02;
#[allow(dead_code)]
pub const TEST_EQ: u8  = 0x03;
pub const TEST_GT: u8  = 0x04;
#[allow(dead_code)]
pub const TEST_GTE: u8 = 0x05;
#[allow(dead_code)]
pub const TEST_LT: u8  = 0x06;
#[allow(dead_code)]
pub const TEST_LTE: u8  = 0x07;
pub const ADD: u8 = 0x08;
pub const SUB: u8 = 0x09;
pub const CALL: u8 = 0x0a;
pub const RETURN: u8 = 0x0b;
pub const MOV: u8 = 0x0c;
#[allow(dead_code)]
pub const TAILCALL: u8 = 0x0d;
pub const JMP: u8 = 0x0e;
pub const TUPLE: u8 = 0x0f;
pub const TUPLE_NTH: u8 = 0x10;
