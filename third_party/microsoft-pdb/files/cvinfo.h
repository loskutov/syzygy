///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef THIRD_PARTY_MICROSOFT_PDB_FILES_CVINFO_H_
#define THIRD_PARTY_MICROSOFT_PDB_FILES_CVINFO_H_

namespace Microsoft_Cci_Pdb {

enum CV_BuildInfo_e {
  CV_BuildInfo_CurrentDirectory    = 0,
  CV_BuildInfo_BuildTool           = 1,    // Cl.exe
  CV_BuildInfo_SourceFile          = 2,    // foo.cpp
  CV_BuildInfo_ProgramDatabaseFile = 3,    // foo.pdb
  CV_BuildInfo_CommandArguments    = 4,    // -I etc
  CV_BUILDINFO_KNOWN
};

// Extensions to SYM_ENUM_e.
// Ranges for en-registered symbol.
const uint16_t S_DEFRANGE_REGISTER = 0x1141;
// Range for stack symbol.
const uint16_t S_DEFRANGE_FRAMEPOINTER_REL = 0x1142;
// Ranges for en-registered field of symbol.
const uint16_t S_DEFRANGE_SUBFIELD_REGISTER = 0x1143;
// Range for stack symbol span valid full scope of function body, gap might
// apply. Provides the frame pointer offset for the S_LOCAL_VS2013 variables.
const uint16_t S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE = 0x1144;
// Range for symbol address as register + offset.
const uint16_t S_DEFRANGE_REGISTER_REL = 0x1145;
// Inlined function callsite.
const uint16_t S_INLINESITE = 0x114d;
const uint16_t S_INLINESITE_END = 0x114e;

// Extensions to TYPE_ENUM_e. These type leaves all end up in PDB stream 4, the
// ID stream. These are largely to do with inlining information.
// Global func ID.
const uint16_t LF_FUNC_ID          = 0x1601;
// Member func ID.
const uint16_t LF_MFUNC_ID         = 0x1602;
// Build info: tool, version, command line, src/pdb file
const uint16_t LF_BUILDINFO        = 0x1603;
// Similar to LF_ARGLIST, for list of sub strings
const uint16_t LF_SUBSTR_LIST      = 0x1604;
// String ID
const uint16_t LF_STRING_ID        = 0x1605;
// Source and line on where an UDT is defined only generated by compiler.
const uint16_t LF_UDT_SRC_LINE     = 0x1606;
// Module, source and line on where an UDT is defined only generated by linker.
const uint16_t LF_UDT_MOD_SRC_LINE = 0x1607;
// One greater than the last supported leaf record.
const uint16_t LF_ID_LAST          = 0x1608;
const uint16_t LF_ID_MAX           = LF_ID_LAST - 1;

}  // namespace Microsoft_Cci_Pdb

// All of the data structures below need to have tight alignment so that they
// can be overlaid directly onto byte streams.
#pragma pack(push, 1)

// Basic type definitions.
typedef uint32_t CV_typ_t;

// Represents an address range, used for optimized code debug info.
struct CvLvarAddrRange {
  uint32_t offStart;
  uint16_t isectStart;
  uint16_t cbRange;  // Length.
};
COMPILE_ASSERT_IS_POD_OF_SIZE(CvLvarAddrRange, 8);

// Represents the holes in overall address range, all address is pre-bbt.
// It is for compress and reduce the amount of relocations need.
struct CvLvarAddrGap {
  uint16_t gapStartOffset;  // Relative offset from beginning of live range.
  uint16_t cbRange;  // Length of gap.
};
COMPILE_ASSERT_IS_POD_OF_SIZE(CvLvarAddrGap, 4);

// Attributes of a variable's range.
union CvRangeAttr {
  uint16_t raw;
  struct {
    uint16_t maybe : 1;  // May have no user name on one of control flow path.
    uint16_t padding : 15;  // Padding for future use.
  };
};
// We coerce a stream of bytes to this structure, so we require it to be
// exactly 2 bytes in size.
COMPILE_ASSERT_IS_POD_OF_SIZE(CvRangeAttr, 2);

// A live range of en-registed variable.
struct DefrangeSymRegister {
  uint16_t reg;             // Register to hold the value of the symbol
  CvRangeAttr attr;       // Attribute of the register range.
  CvLvarAddrRange range;  // Range of addresses where this program is valid.
  CvLvarAddrGap gaps[1];  // The value is not available in following gaps.
};
COMPILE_ASSERT_IS_POD_OF_SIZE(DefrangeSymRegister, 16);

// A live range of frame variable.
struct DefRangeSymFramePointerRel {
  int32_t offFramePointer;  // Offset to frame pointer.
  CvLvarAddrRange range;   // Range of addresses where this program is valid.
  CvLvarAddrGap gaps[1];   // The value is not available in following gaps.
};
COMPILE_ASSERT_IS_POD_OF_SIZE(DefRangeSymFramePointerRel, 16);

// Ranges for en-registered field of symbol.
struct DefRangeSymSubfieldRegister {
  uint16_t reg;             // Register to hold the value of the symbol
  CvRangeAttr attr;       // Attribute of the register range.
  uint32_t offParent : 12;  // Offset in parent variable.
  uint32_t padding : 20;    // Padding for future use.
  CvLvarAddrRange range;  // Range of addresses where this program is valid.
  CvLvarAddrGap gaps[1];  // The value is not available in following gaps.
};
COMPILE_ASSERT_IS_POD_OF_SIZE(DefRangeSymSubfieldRegister, 20);

// Inlined function callsite.
struct InlineSiteSym {
  uint32_t pParent;              // Pointer to the inliner.
  uint32_t pEnd;                 // Pointer to this block's end.
  uint32_t inlinee;              // CV_ItemId of inlinee.
  uint8_t binaryAnnotations[1];  // An array of compressed binary annotations.
};
COMPILE_ASSERT_IS_POD_OF_SIZE(InlineSiteSym, 13);

// Item Id is a stricter typeindex which may referenced from symbol stream.
// The code item always had a name.
typedef CV_typ_t CV_ItemId;

// ID of a global function. Its position in the ID stream is its ID.
// Only found in the ID stream of PDBs.
struct LeafFunctionId {
  CV_ItemId scopeId;  // Parent scope of the ID, 0 if global.
  CV_typ_t type;      // Function type.
  char name[1];       // Null terminated function name.
};
COMPILE_ASSERT_IS_POD_OF_SIZE(LeafFunctionId, 9);

// ID of a member function. Its position in the ID stream is its ID.
// Only found in the ID stream of PDBs.
struct LeafMemberFunctionId {
  CV_typ_t parentType;  // Type index of parent.
  CV_typ_t type;        // Function type.
  char name[1];         // Null terminated function name.
};
COMPILE_ASSERT_IS_POD_OF_SIZE(LeafMemberFunctionId, 9);

// ID of a string. Its position in the ID stream is its ID.
// Only found in the ID stream of PDBs.
struct LeafStringId {
  CV_ItemId id;  // ID to list of sub-string IDs.
  char name[1];  // Null terminated string.
};
COMPILE_ASSERT_IS_POD_OF_SIZE(LeafStringId, 5);

// Source line of a where a UDT is defined. Generated by the compiler.
// Only found in the ID stream of PDBs.
struct LeafUdtSourceLine {
  CV_typ_t type;  // UDT's type index.
  CV_ItemId src;  // Index to LF_STRING_ID record of source file name.
  uint32_t line;  // Line number.
};
COMPILE_ASSERT_IS_POD_OF_SIZE(LeafUdtSourceLine, 12);

// Module and source line of a where a UDT is defined. Generated by the linker.
// Only found in the ID stream of PDBs.
struct LeafUdtModuleSourceLine {
  CV_typ_t type;  // UDT's type index
  CV_ItemId src;  // index into string table where source file name is saved
  uint32_t line;  // line number
  uint16_t imod;  // module that contributes this UDT definition 
};
COMPILE_ASSERT_IS_POD_OF_SIZE(LeafUdtModuleSourceLine, 14);

// Type record for build information.
struct LeafBuildInfo {
  uint16_t count;    // Number of arguments.
  CV_ItemId arg[1];  // Arguments as CodeItemId.
};
COMPILE_ASSERT_IS_POD_OF_SIZE(LeafBuildInfo, 6);

#pragma pack(pop)

#endif  // THIRD_PARTY_MICROSOFT_PDB_FILES_CVINFO_H_