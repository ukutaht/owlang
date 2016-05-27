use std::ffi::{CStr, CString};
use std::os::raw::c_char;

use ast;
use bytecode;
use parser;
use std;
use std::fs::File;
use std::path::{Path, PathBuf};


static SOURCE_EXTENSION: &'static str = "owl";
static TARGET_EXTENSION: &'static str = "owlc";

#[repr(C)]
#[no_mangle]
pub struct Module {
    pub size: usize,
    pub code: *mut i8,
}

#[no_mangle]
pub extern "C" fn compile_file_to_memory(c_input: *const c_char) -> *mut Module {
    let input = unsafe { CStr::from_ptr(c_input).to_str().unwrap() };

    let code_vec = compile_to_binary(&PathBuf::from(input));
    let size = code_vec.len();
    let code = unsafe { CString::from_vec_unchecked(code_vec).into_raw() };

    let module = Module {
        size: size,
        code: code
    };

    unsafe { std::mem::transmute(Box::new(module)) }
}

#[no_mangle]
#[allow(unused_variables)]
pub extern "C" fn free_module(binary: *mut Module) {
    let module: Box<Module> = unsafe { std::mem::transmute(binary) };
    let code = unsafe { CString::from_raw(module.code) };
    // Drop
}

pub fn compile_to_file(inp: &PathBuf, out: &PathBuf) {
    std::fs::create_dir_all(&out).unwrap();

    if inp.is_file() {
        if !has_source_extension(inp) { return };

        let file = File::open(inp).ok().expect(&format!("Failed to open file: {}", &inp.to_str().unwrap()));
        parser::parse(file, |module| generate_to_file(module, out))
    } else if inp.is_dir() {
        for file in inp.read_dir().unwrap() {
            compile_to_file(&file.unwrap().path(), out);
        }
    } else {
        panic!("Cannot read {:?}. Expected a file or a directory", inp);
    }
}

pub fn compile_to_stdout(inp: &PathBuf) {
    let file  = File::open(inp).ok().expect(&format!("Failed to open file: {}", &inp.to_str().unwrap()));

    parser::parse(file, |expr| generate_to_stdout(expr))
}

pub fn compile_to_binary(inp: &PathBuf) -> Vec<u8> {
    let file  = File::open(inp).ok().expect(&format!("Failed to open file: {}", &inp.to_str().unwrap()));
    let mut output = Vec::new();

    parser::parse(file, |expr| output.append(&mut generate_to_binary(expr)));
    output
}

fn generate_to_file(expr: &ast::Module, out: &PathBuf) {
    let out_filename = PathBuf::from(expr.name).with_extension(TARGET_EXTENSION);
    let out_name = out.join(Path::new(out_filename.file_name().unwrap()));
    let mut out_buffer = File::create(out_name).unwrap();
    let bytecode = bytecode::generate(expr);

    bytecode.emit(&mut out_buffer);
}

fn generate_to_stdout(expr: &ast::Module) {
    let bytecode = bytecode::generate(expr);
    let mut writer = std::io::BufWriter::new(std::io::stdout());

    bytecode.emit_human_readable(&mut writer);
}

fn generate_to_binary(expr: &ast::Module) -> Vec<u8> {
    let bytecode = bytecode::generate(expr);
    let mut writer = std::io::BufWriter::new(Vec::new());

    bytecode.emit(&mut writer);
    writer.into_inner().unwrap()
}

fn has_source_extension(path: &PathBuf) -> bool {
    let extension = path.extension()
        .map(|os_string| os_string.to_str())
        .unwrap_or(None)
        .unwrap_or("");

    path.is_file() && extension == SOURCE_EXTENSION
}
