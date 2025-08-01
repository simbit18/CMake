/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <set>
#include <string>
#include <vector>

#include "cmsys/RegularExpression.hxx"

#include "cmUVProcessChain.h"

class cmXMLElement;

/** \class cmCTestLaunchReporter
 * \brief Generate CTest XML output for the 'ctest --launch' tool.
 */
class cmCTestLaunchReporter
{
public:
  // Initialize the launcher from its command line.
  cmCTestLaunchReporter();
  ~cmCTestLaunchReporter();

  cmCTestLaunchReporter(cmCTestLaunchReporter const&) = delete;
  cmCTestLaunchReporter& operator=(cmCTestLaunchReporter const&) = delete;

  // Methods to check the result of the real command.
  bool IsError() const;

  // Launcher options specified before the real command.
  std::string OptionOutput;
  std::string OptionSource;
  std::string OptionLanguage;
  std::string OptionTargetLabels;
  std::string OptionTargetName;
  std::string OptionTargetType;
  std::string OptionCurrentBuildDir;
  std::string OptionBuildDir;
  std::string OptionFilterPrefix;
  std::string OptionCommandType;
  std::string OptionRole;
  std::string OptionConfig;
  std::string OptionObjectDir;

  // The real command line appearing after launcher arguments.
  std::string CWD;

  // The real command line after response file expansion.
  std::vector<std::string> RealArgs;

  // A hash of the real command line is unique and unlikely to collide.
  std::string LogHash;
  void ComputeFileNames();

  bool Passthru;
  cmUVProcessChain::Status Status;
  int ExitCode;

  // Temporary log files for stdout and stderr of real command.
  std::string LogDir;
  std::string LogOut;
  std::string LogErr;

  // Labels associated with the build rule.
  std::set<std::string> Labels;
  void LoadLabels();
  bool SourceMatches(std::string const& lhs, std::string const& rhs);

  // Regular expressions to match warnings and their exceptions.
  std::vector<cmsys::RegularExpression> RegexWarning;
  std::vector<cmsys::RegularExpression> RegexWarningSuppress;
  bool Match(std::string const& line,
             std::vector<cmsys::RegularExpression>& regexps);
  bool MatchesFilterPrefix(std::string const& line) const;

  // Methods to generate the xml fragment.
  void WriteXML();
  void WriteXMLAction(cmXMLElement&) const;
  void WriteXMLCommand(cmXMLElement&);
  void WriteXMLResult(cmXMLElement&);
  void WriteXMLLabels(cmXMLElement&);
  void DumpFileToXML(cmXMLElement&, char const* tag, std::string const& fname);

  // Configuration
  std::string SourceDir;
};
