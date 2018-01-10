// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <sstream>

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/wire_format.h>

#include "src/lib/csharp_doc_comment.h"
#include "src/lib/csharp_helpers.h"
#include "src/lib/csharp_repeated_enum_field.h"

using namespace google::protobuf;

namespace zynga {
namespace protobuf {
namespace compiler {
namespace csharp {

RepeatedEnumFieldGenerator::RepeatedEnumFieldGenerator(
    const FieldDescriptor* descriptor, int fieldOrdinal, const Options *options)
    : FieldGeneratorBase(descriptor, fieldOrdinal, options) {
}

RepeatedEnumFieldGenerator::~RepeatedEnumFieldGenerator() {

}

void RepeatedEnumFieldGenerator::GenerateMembers(io::Printer* printer, bool isEventSourced) {
  printer->Print(
    variables_,
    "private static readonly pb::FieldCodec<$type_name$> _repeated_$name$_codec\n"
    "    = pb::FieldCodec.ForEnum($tag$, x => (int) x, x => ($type_name$) x);\n");
  printer->Print(variables_,
    "private readonly pbc::RepeatedField<$type_name$> $name$_ = new pbc::RepeatedField<$type_name$>();\n");
  WritePropertyDocComment(printer, descriptor_);
  if (isEventSourced) {
    printer->Print(
      variables_,
      "$access_level$ void Add$property_name$($type_name$ value) {\n"
      " AddEvent($number$, zpr.EventSource.EventAction.AddList, value);\n"
      " $name$_.Add(value);\n"
    "}\n");

    printer->Print(
      variables_,
      "$access_level$ void Remove$property_name$($type_name$ value) {\n"
      " AddEvent($number$, zpr.EventSource.EventAction.RemoveList, value);\n"
      " $name$_.Remove(value);\n"
    "}\n");

    printer->Print(
      variables_,
      "$access_level$ void Clear$property_name$() {\n"
      " AddEvent($number$, zpr.EventSource.EventAction.ClearList, -1);\n"
      " $name$_.Clear();\n"
    "}\n");

    // We will expose a readOnly list? 
    AddPublicMemberAttributes(printer);
    printer->Print(
      variables_,
      "#if !NET35\n"
      "$access_level$ IReadOnlyList<$type_name$> $property_name$ {\n"
      "  get { return $name$_; }\n"
      "}\n"
      "#endif\n");
  } 
  else {
    AddPublicMemberAttributes(printer);
    printer->Print(
      variables_,
      "$access_level$ pbc::RepeatedField<$type_name$> $property_name$ {\n"
      "  get { return $name$_; }\n"
      "}\n");
  }
}

void RepeatedEnumFieldGenerator::GenerateEventSource(io::Printer* printer) {
  printer->Print(
      variables_,
      "        if (e.Action == zpr.EventSource.EventAction.AddList) {\n"
      "          $name$_.Add(($type_name$)e.Data.U32);\n"
      "        } else if (e.Action == zpr.EventSource.EventAction.RemoveList) {\n"
      "          $name$_.Remove(($type_name$)e.Data.U32);\n"
      "        } else if (e.Action == zpr.EventSource.EventAction.ClearList) {\n"
      "          $name$_.Clear();\n"
      "        }\n");
}

void RepeatedEnumFieldGenerator::GenerateEventAdd(io::Printer* printer, bool isMap) {
  std::map<string, string> vars;
  vars["data_value"] = GetEventDataType(descriptor_);
  vars["type_name"] = variables_["type_name"];
  printer->Print(vars, "        return new zpr.EventSource.EventContent() { data_ = Convert.ToUInt32(data), dataCase_ = zpr.EventSource.EventContent.DataOneofCase.$data_value$ };\n");

}

void RepeatedEnumFieldGenerator::GenerateEventAddEvent(io::Printer* printer) {
  printer->Print(
    "        e.Path.AddRange(this.Path.$field_name$Path._path);\n",
    "field_name", GetPropertyName(descriptor_));
}

void RepeatedEnumFieldGenerator::GenerateCheckSum(io::Printer* printer) {
  if (checksum_exclude()) return;
}

void RepeatedEnumFieldGenerator::GenerateMergingCode(io::Printer* printer) {
  printer->Print(
    variables_,
    "$name$_.Add(other.$name$_);\n");
}

void RepeatedEnumFieldGenerator::GenerateParsingCode(io::Printer* printer) {
  printer->Print(
    variables_,
    "$name$_.AddEntriesFrom(input, _repeated_$name$_codec);\n");
}

void RepeatedEnumFieldGenerator::GenerateSerializationCode(io::Printer* printer) {
  printer->Print(
    variables_,
    "$name$_.WriteTo(output, _repeated_$name$_codec);\n");
}

void RepeatedEnumFieldGenerator::GenerateSerializedSizeCode(io::Printer* printer) {  
  printer->Print(
    variables_,
    "size += $name$_.CalculateSize(_repeated_$name$_codec);\n");
}

void RepeatedEnumFieldGenerator::WriteHash(io::Printer* printer) {
  printer->Print(
    variables_,
    "hash ^= $name$_.GetHashCode();\n");
}

void RepeatedEnumFieldGenerator::WriteEquals(io::Printer* printer) {
  printer->Print(
    variables_,
    "if(!$name$_.Equals(other.$name$_)) return false;\n");
}

void RepeatedEnumFieldGenerator::WriteToString(io::Printer* printer) {
  printer->Print(variables_,
    "PrintField(\"$descriptor_name$\", $name$_, writer);\n");
}

void RepeatedEnumFieldGenerator::GenerateCloningCode(io::Printer* printer) {
  printer->Print(variables_,
    "$name$_ = other.$name$_.Clone();\n");
}

void RepeatedEnumFieldGenerator::GenerateFreezingCode(io::Printer* printer) {
}

}  // namespace csharp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
