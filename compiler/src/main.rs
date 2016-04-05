extern crate iris_compiler;
extern crate getopts;
extern crate chomp;

use getopts::Options;
use chomp::buffer::{Source, Stream, StreamError};
use std::env;
use std::fs::File;
use std::path::PathBuf;

fn generate_to_file(expr: &iris_compiler::ast::Expr, file: &mut File) {
    let bytecode = iris_compiler::bytecode::generate(expr);

    bytecode.emit(file);
}

fn generate_to_stdout(expr: &iris_compiler::ast::Expr) {
    let bytecode = iris_compiler::bytecode::generate(expr);
    let mut writer = std::io::BufWriter::new(std::io::stdout());

    bytecode.emit_human_readable(&mut writer);
}

fn compile_to_file(inp: &PathBuf, out: &PathBuf) {
    let file  = File::open(inp).ok().expect(&format!("Failed to open file: {}", &inp.to_str().unwrap()));

    std::fs::create_dir_all(&out).unwrap();
    let out_name = out.join(inp.with_extension("irc"));
    let mut out_buffer = File::create(out_name).unwrap();

    let mut i = Source::new(file);
    loop {
        match i.parse(iris_compiler::parser::expr) {
            Ok(expr)                     => generate_to_file(&expr, &mut out_buffer),
            Err(StreamError::Retry)      => {},
            Err(StreamError::EndOfInput) => break,
            Err(e)                       => { panic!("{:?}", e); }
        }
    }
}

fn compile_to_stdout(inp: &PathBuf) {
    let file  = File::open(inp).ok().expect(&format!("Failed to open file: {}", &inp.to_str().unwrap()));
    let mut i = Source::new(file);
    loop {
        match i.parse(iris_compiler::parser::expr) {
            Ok(expr)                     => generate_to_stdout(&expr),
            Err(StreamError::Retry)      => {},
            Err(StreamError::EndOfInput) => break,
            Err(e)                       => { panic!("{:?}", e); }
        }
    }
}

fn print_usage(program: &str, opts: Options) {
    let brief = format!("Usage: {} FILE [options]", program);
    print!("{}", opts.usage(&brief));
}

fn main() {
    let args: Vec<String> = env::args().collect();

    let mut opts = Options::new();
    opts.optopt("o", "output", "Output directory(default: target)", "NAME");
    opts.optflag("p", "print", "Only print the bytecode");
    opts.optflag("h", "help", "Show help");

    let matches = match opts.parse(&args[1..]) {
        Ok(m) => { m }
        Err(f) => { panic!(f.to_string()) }
    };
    if matches.opt_present("h") || matches.free.is_empty() {
        print_usage(&args[0].clone(), opts);
        return;
    }
    let input = PathBuf::from(matches.free[0].clone());

    if matches.opt_present("p") {
        compile_to_stdout(&input);
    } else {
        let current_dir = env::current_dir().unwrap();
        let output = matches.opt_str("o")
                            .map(|s| PathBuf::from(s))
                            .unwrap_or(current_dir);

        compile_to_file(&input, &output);
    }
}
