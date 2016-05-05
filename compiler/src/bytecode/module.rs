use std::io::Write;
use bytecode::function::Function;

#[derive(Debug, Eq, PartialEq)]
pub struct Module {
    pub name: String,
    pub functions: Vec<Function>,
}

impl Module {
    pub fn emit<T: Write>(&self, out: &mut T) {
        for function in self.functions.iter() {
            function.emit(out);
        }
    }

    pub fn emit_human_readable<T: Write>(&self, out: &mut T) {
        out.write(format!("module {}\n", self.name).as_bytes()).unwrap();

        for function in self.functions.iter() {
            function.emit_human_readable(out);
        }
        out.write("\n".as_bytes()).unwrap();
    }
}

