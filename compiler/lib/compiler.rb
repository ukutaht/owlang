require 'opcodes'

CompileError = Class.new(StandardError)

class Compiler
  include OpCodes
  LABEL = /([_a-zA-Z][_a-zA-Z0-9]+):/

  def initialize(code, out)
    @code = code
    @out = out
    @output = []
    @instruction = 0
    @labels = {}
    @label_usages = []
  end

  def compile
    code.each_line do |line|
      process(prepare(line))
    end

    translate_labels
    out.print(@output.pack('C*'))
  end

  private
  attr_reader :out, :code

  def process(line)
    op, *args = line

    case op
    when LABEL
      label = op.match(LABEL)[1]
      @labels[label] = @instruction
    when "exit"
      emit(EXIT)
      emit(extract_int(args[0]))
    when "store"
      emit(INT_STORE)
      emit(extract_reg(args[0]))

      div, mod = extract_int(args[1]).divmod(256)
      emit(mod)
      emit(div)
    when "int_print"
      emit(INT_PRINT)
      emit(extract_reg(args[0]))
    when "cmp"
      emit(CMP)
      emit(extract_reg(args[0]))
      emit(extract_reg(args[1]))
    when "jmpz"
      emit(JMPZ)
      emit(extract_label(args[0]))
    else
      raise "Unkown operation: #{op}"
    end
  end

  def translate_labels
    @label_usages.each do |instr|
      @output[instr] = @labels[@output[instr]]
    end
  end

  def prepare(line)
    op, args = line.strip.split(' ', 2)
    if args
      [op] + args.split(',').map(&:strip)
    else
      [op]
    end
  end

  def extract_reg(reg)
    if match = reg.match(/%(\d+)/)
      match[1].to_i
    else
      raise CompileError.new('Expected register')
    end
  end

  def extract_int(int)
    if int && match = int.match(/^(\d+)/)
      match[1].to_i
    else
      raise CompileError.new('Expected integer')
    end
  end

  def extract_label(label)
    @label_usages << @instruction
    label
  end

  def emit(opcode)
    @instruction += 1
    @output << opcode
  end
end
