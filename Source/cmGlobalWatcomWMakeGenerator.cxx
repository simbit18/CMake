/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGlobalWatcomWMakeGenerator.h"

#include <ostream>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmake.h"

cmGlobalWatcomWMakeGenerator::cmGlobalWatcomWMakeGenerator(cmake* cm)
  : cmGlobalUnixMakefileGenerator3(cm)
{
  this->FindMakeProgramFile = "CMakeFindWMake.cmake";
#ifdef _WIN32
  this->ForceUnixPaths = false;
#endif
  this->ToolSupportsColor = true;
  this->NeedSymbolicMark = true;
  this->EmptyRuleHackCommand = "@%null";
#ifdef _WIN32
  cm->GetState()->SetWindowsShell(true);
#endif
  cm->GetState()->SetWatcomWMake(true);
  this->IncludeDirective = "!include";
  this->LineContinueDirective = "&\n";
  this->DefineWindowsNULL = true;
  this->UnixCD = false;
  this->MakeSilentFlag = "-h";
}

void cmGlobalWatcomWMakeGenerator::EnableLanguage(
  std::vector<std::string> const& l, cmMakefile* mf, bool optional)
{
  // pick a default
  mf->AddDefinition("WATCOM", "1");
  mf->AddDefinition("CMAKE_QUOTE_INCLUDE_PATHS", "1");
  mf->AddDefinition("CMAKE_MANGLE_OBJECT_FILE_NAMES", "1");
  mf->AddDefinition("CMAKE_MAKE_SYMBOLIC_RULE", ".SYMBOLIC");
  mf->AddDefinition("CMAKE_GENERATOR_CC", "wcl386");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "wcl386");
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf, optional);
}

bool cmGlobalWatcomWMakeGenerator::SetSystemName(std::string const& s,
                                                 cmMakefile* mf)
{
  if (mf->GetSafeDefinition("CMAKE_SYSTEM_PROCESSOR") == "I86"_s) {
    mf->AddDefinition("CMAKE_GENERATOR_CC", "wcl");
    mf->AddDefinition("CMAKE_GENERATOR_CXX", "wcl");
  }
  return this->cmGlobalUnixMakefileGenerator3::SetSystemName(s, mf);
}

cmDocumentationEntry cmGlobalWatcomWMakeGenerator::GetDocumentation()
{
  return { cmGlobalWatcomWMakeGenerator::GetActualName(),
           "Generates Watcom WMake makefiles." };
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalWatcomWMakeGenerator::GenerateBuildCommand(
  std::string const& makeProgram, std::string const& projectName,
  std::string const& projectDir, std::vector<std::string> const& targetNames,
  std::string const& config, int /*jobs*/, bool verbose,
  cmBuildOptions const& buildOptions,
  std::vector<std::string> const& makeOptions)
{
  return this->cmGlobalUnixMakefileGenerator3::GenerateBuildCommand(
    makeProgram, projectName, projectDir, targetNames, config,
    cmake::NO_BUILD_PARALLEL_LEVEL, verbose, buildOptions, makeOptions);
}

std::string cmGlobalWatcomWMakeGenerator::GetShortBinaryOutputDir() const
{
  return "_o";
}

void cmGlobalWatcomWMakeGenerator::PrintBuildCommandAdvice(std::ostream& os,
                                                           int jobs) const
{
  if (jobs != cmake::NO_BUILD_PARALLEL_LEVEL) {
    // wmake does not support parallel build level

    /* clang-format off */
    os <<
      "Warning: Watcom's WMake does not support parallel builds. "
      "Ignoring parallel build command line option.\n";
    /* clang-format on */
  }

  this->cmGlobalUnixMakefileGenerator3::PrintBuildCommandAdvice(
    os, cmake::NO_BUILD_PARALLEL_LEVEL);
}
