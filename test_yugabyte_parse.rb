#!/usr/bin/env ruby
require 'ffi'

module LibPgQuery
  extend FFI::Library
  ffi_lib File.expand_path('../libpg_query.a', __FILE__)

  class PgQueryError < FFI::Struct
    layout :message, :string,
           :funcname, :string,
           :filename, :string,
           :lineno, :int,
           :cursorpos, :int,
           :context, :string
  end

  class PgQueryProtobuf < FFI::Struct
    layout :len, :size_t,
           :data, :pointer
  end

  class PgQueryProtobufParseResult < FFI::Struct
    layout :parse_tree, PgQueryProtobuf,
           :stderr_buffer, :string,
           :error, :pointer
  end

  attach_function :pg_query_parse_protobuf, [:string], PgQueryProtobufParseResult.by_value
  attach_function :pg_query_free_protobuf_parse_result, [PgQueryProtobufParseResult.by_value], :void
end

# Test with YugabyteDB SQL
yugabyte_sql = "CREATE INDEX idx_test ON core.test (shop_id HASH, code) SPLIT INTO 4 TABLETS"

puts "Testing YugabyteDB SQL parsing with libpg_query..."
puts "SQL: #{yugabyte_sql}"
puts

result = LibPgQuery.pg_query_parse_protobuf(yugabyte_sql)

if result[:error].null?
  puts "✓ Success! Parse completed without error."
  puts "Parse tree length: #{result[:parse_tree][:len]} bytes"

  if result[:stderr_buffer] && !result[:stderr_buffer].empty?
    puts "stderr output: #{result[:stderr_buffer]}"
  end

  # Read the protobuf data
  if result[:parse_tree][:len] > 0
    data = result[:parse_tree][:data].read_bytes(result[:parse_tree][:len])
    puts "Protobuf data (first 100 bytes): #{data[0...100].inspect}"
  end
else
  error = LibPgQuery::PgQueryError.new(result[:error])
  puts "✗ Parse error:"
  puts "  Message: #{error[:message]}"
  puts "  Function: #{error[:funcname]}"
  puts "  File: #{error[:filename]}:#{error[:lineno]}"
  puts "  Cursor position: #{error[:cursorpos]}"
end

LibPgQuery.pg_query_free_protobuf_parse_result(result)

# Also test basic PostgreSQL SQL
puts "\n" + "="*80 + "\n"
basic_sql = "SELECT * FROM users WHERE id = 1"
puts "Testing basic PostgreSQL SQL parsing..."
puts "SQL: #{basic_sql}"
puts

result2 = LibPgQuery.pg_query_parse_protobuf(basic_sql)

if result2[:error].null?
  puts "✓ Success! Parse completed without error."
  puts "Parse tree length: #{result2[:parse_tree][:len]} bytes"
else
  error = LibPgQuery::PgQueryError.new(result2[:error])
  puts "✗ Parse error: #{error[:message]}"
end

LibPgQuery.pg_query_free_protobuf_parse_result(result2)
