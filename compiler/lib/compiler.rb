require 'opcodes'

class Compiler
  include OpCodes

  def initialize(code, out)
    @code = code
    @out = out
  end

  def compile
    code.each_line do |line|
      process(prepare(line))
    end
  end

  private
  attr_reader :out, :code

  def process(line)
    op, *args = line

    case op
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
      emit(extract_reg(args.first))
    else
      raise "Unkown operation: #{op}"
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
    reg.match(/#(\d+)/)[1].to_i
  end

  def extract_int(int)
    int.to_i
  end

  def emit(opcode)
    out.print([opcode].pack('C*'))
  end
end
