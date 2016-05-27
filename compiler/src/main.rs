extern crate owlc;
extern crate getopts;
extern crate chomp;

use owlc::*;
use getopts::Options;
use std::env;
use std::path::{PathBuf};

fn print_usage(program: &str, opts: Options) {
    let brief = format!("Usage: {} FILE/DIR [options]", program);
    print!("{}", opts.usage(&brief));
}

fn main() {
    let args: Vec<String> = env::args().collect();

    let mut opts = Options::new();
    opts.optopt("o", "output", "Output directory(default: current directory)", "NAME");
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
        compiler::compile_to_stdout(&input);
    } else {
        let current_dir = std::env::current_dir().unwrap();
        let output = matches.opt_str("o")
                            .map(|s| PathBuf::from(s))
                            .unwrap_or(current_dir);

        compiler::compile_to_file(&input, &output);
    }
}
