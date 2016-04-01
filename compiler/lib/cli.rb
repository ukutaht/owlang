require 'compiler'

require 'stringio'
require 'optparse'
require 'ostruct'
require 'fileutils'

class CLI
  def run
    FileUtils.mkdir_p(options.output)

    options.files.each do |filename|
      code = File.read(filename)
      out_filename = File.join(options.output, filename)
      File.open(out_filename, 'wb') do |out|
        Compiler.new(code, out).compile
      end
    end
  end

  private

  def options
    @options ||= get_options
  end

  def get_options
    options = OpenStruct.new({
      output: 'target',
      files: []
    })

    OptionParser.new do |opts|
      opts.banner = "Usage: compiler [files] [options]"

      opts.on("-o OUT", "--out-dir", "Output directory (default: #{options.output})") do |d|
        options.output = d
      end

      opts.accept(Array)
    end.parse!

    options.files += ARGV
    options
  end
end
