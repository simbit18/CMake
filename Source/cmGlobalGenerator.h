/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmBuildOptions.h"
#include "cmCustomCommandLines.h"
#include "cmDuration.h"
#include "cmExportSet.h"
#include "cmLocalGenerator.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetDepend.h"
#include "cmValue.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include <cm3p/json/value.h>

#  include "cmFileLockPool.h"
#endif

#define CMAKE_DIRECTORY_ID_SEP "::@"

enum class cmDepfileFormat;
enum class codecvt_Encoding;

class cmDirectoryId;
class cmExportBuildFileGenerator;
class cmExternalMakefileProjectGenerator;
class cmGeneratorTarget;
class cmInstallRuntimeDependencySet;
class cmLinkLineComputer;
class cmMakefile;
class cmOutputConverter;
class cmQtAutoGenGlobalInitializer;
class cmSourceFile;
class cmState;
class cmStateDirectory;
class cmake;

namespace detail {
inline void AppendStrs(std::vector<std::string>&)
{
}
template <typename T, typename... Ts>
inline void AppendStrs(std::vector<std::string>& command, T&& s, Ts&&... ts)
{
  command.emplace_back(std::forward<T>(s));
  AppendStrs(command, std::forward<Ts>(ts)...);
}

struct GeneratedMakeCommand
{
  // Add each argument as a separate element to the vector
  template <typename... T>
  void Add(T&&... args)
  {
    // iterate the args and append each one
    AppendStrs(this->PrimaryCommand, std::forward<T>(args)...);
  }

  // Add each value in the iterators as a separate element to the vector
  void Add(std::vector<std::string>::const_iterator start,
           std::vector<std::string>::const_iterator end)
  {
    cm::append(this->PrimaryCommand, start, end);
  }

  std::string Printable() const { return cmJoin(this->PrimaryCommand, " "); }
  std::string QuotedPrintable() const;

  std::vector<std::string> PrimaryCommand;
  bool RequiresOutputForward = false;
};
}
namespace Json {
class StreamWriter;
}

/** \class cmGlobalGenerator
 * \brief Responsible for overseeing the generation process for the entire tree
 *
 * Subclasses of this class generate makefiles for various
 * platforms.
 */
class cmGlobalGenerator
{
public:
  using LocalGeneratorVector = std::vector<std::unique_ptr<cmLocalGenerator>>;

  //! Free any memory allocated with the GlobalGenerator
  cmGlobalGenerator(cmake* cm);
  virtual ~cmGlobalGenerator();

  virtual std::unique_ptr<cmLocalGenerator> CreateLocalGenerator(
    cmMakefile* mf);

  //! Get the name for this generator
  virtual std::string GetName() const { return "Generic"; }

  /** Check whether the given name matches the current generator.  */
  virtual bool MatchesGeneratorName(std::string const& name) const
  {
    return this->GetName() == name;
  }

  /** Get encoding used by generator for makefile files */
  virtual codecvt_Encoding GetMakefileEncoding() const;

#if !defined(CMAKE_BOOTSTRAP)
  /** Get a JSON object describing the generator.  */
  virtual Json::Value GetJson() const;
#endif

  /** Tell the generator about the target system.  */
  virtual bool SetSystemName(std::string const&, cmMakefile*) { return true; }

  /** Set the generator-specific instance.  Returns true if supported.  */
  virtual bool SetGeneratorInstance(std::string const& i, cmMakefile* mf);

  /** Set the generator-specific platform name.  Returns true if platform
      is supported and false otherwise.  */
  virtual bool SetGeneratorPlatform(std::string const& p, cmMakefile* mf);

  /** Set the generator-specific toolset name.  Returns true if toolset
      is supported and false otherwise.  */
  virtual bool SetGeneratorToolset(std::string const& ts, bool build,
                                   cmMakefile* mf);

  /** Read any other cache entries needed for cmake --build. */
  virtual bool ReadCacheEntriesForBuild(cmState const& /*state*/)
  {
    return true;
  }

  /**
   * Create LocalGenerators and process the CMakeLists files. This does not
   * actually produce any makefiles, DSPs, etc.
   */
  virtual void Configure();

  virtual bool InspectConfigTypeVariables() { return true; }

  enum class CxxModuleSupportQuery
  {
    // Support is expected at the call site.
    Expected,
    // The call site is querying for support and handles problems by itself.
    Inspect,
  };
  virtual bool CheckCxxModuleSupport(CxxModuleSupportQuery /*query*/)
  {
    return false;
  }

  virtual bool SupportsBuildDatabase() const { return false; }
  bool AddBuildDatabaseTargets();
  void AddBuildDatabaseFile(std::string const& lang, std::string const& config,
                            std::string const& path);

  virtual bool IsGNUMakeJobServerAware() const { return false; }

  bool Compute();
  virtual void AddExtraIDETargets() {}

  enum TargetTypes
  {
    AllTargets,
    ImportedOnly
  };

  void CreateImportedGenerationObjects(
    cmMakefile* mf, std::vector<std::string> const& targets,
    std::vector<cmGeneratorTarget const*>& exports);
  void CreateGenerationObjects(TargetTypes targetTypes = AllTargets);

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.
   */
  virtual void Generate();

  virtual std::unique_ptr<cmLinkLineComputer> CreateLinkLineComputer(
    cmOutputConverter* outputConverter,
    cmStateDirectory const& stateDir) const;

  std::unique_ptr<cmLinkLineComputer> CreateMSVC60LinkLineComputer(
    cmOutputConverter* outputConverter,
    cmStateDirectory const& stateDir) const;

  /**
   * Set/Get and Clear the enabled languages.
   */
  void SetLanguageEnabled(std::string const&, cmMakefile* mf);
  bool GetLanguageEnabled(std::string const&) const;
  void ClearEnabledLanguages();
  void GetEnabledLanguages(std::vector<std::string>& lang) const;
  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void EnableLanguage(std::vector<std::string> const& languages,
                              cmMakefile*, bool optional);

  /**
   * Resolve the CMAKE_<lang>_COMPILER setting for the given language.
   * Intended to be called from EnableLanguage.
   */
  void ResolveLanguageCompiler(std::string const& lang, cmMakefile* mf,
                               bool optional) const;

  /**
   * Try to determine system information, get it from another generator
   */
  void EnableLanguagesFromGenerator(cmGlobalGenerator* gen, cmMakefile* mf);

  /**
   * Try running cmake and building a file. This is used for dynamically
   * loaded commands, not as part of the usual build process.
   */
  int TryCompile(int jobs, std::string const& srcdir,
                 std::string const& bindir, std::string const& projectName,
                 std::string const& targetName, bool fast, std::string& output,
                 cmMakefile* mf);

  /**
   * Build a file given the following information. This is a more direct call
   * that is used by both CTest and TryCompile. If target name is NULL or
   * empty then all is assumed. clean indicates if a "make clean" should be
   * done first.
   */
  int Build(int jobs, std::string const& srcdir, std::string const& bindir,
            std::string const& projectName,
            std::vector<std::string> const& targetNames, std::ostream& ostr,
            std::string const& makeProgram, std::string const& config,
            cmBuildOptions const& buildOptions, bool verbose,
            cmDuration timeout, cmSystemTools::OutputOption outputMode,
            std::vector<std::string> const& nativeOptions =
              std::vector<std::string>());

  /**
   * Open a generated IDE project given the following information.
   */
  virtual bool Open(std::string const& bindir, std::string const& projectName,
                    bool dryRun);

  struct GeneratedMakeCommand final : public detail::GeneratedMakeCommand
  {
  };

  virtual std::vector<GeneratedMakeCommand> GenerateBuildCommand(
    std::string const& makeProgram, std::string const& projectName,
    std::string const& projectDir, std::vector<std::string> const& targetNames,
    std::string const& config, int jobs, bool verbose,
    cmBuildOptions const& buildOptions = cmBuildOptions(),
    std::vector<std::string> const& makeOptions = std::vector<std::string>());

  virtual void PrintBuildCommandAdvice(std::ostream& os, int jobs) const;

  /**
   * Generate a "cmake --build" call for a given target, config and parallel
   * level.
   */
  std::string GenerateCMakeBuildCommand(std::string const& target,
                                        std::string const& config,
                                        std::string const& parallel,
                                        std::string const& native,
                                        bool ignoreErrors);

  //! Get the CMake instance
  cmake* GetCMakeInstance() const { return this->CMakeInstance; }

  void SetConfiguredFilesPath(cmGlobalGenerator* gen);
  std::vector<std::unique_ptr<cmMakefile>> const& GetMakefiles() const
  {
    return this->Makefiles;
  }
  LocalGeneratorVector const& GetLocalGenerators() const
  {
    return this->LocalGenerators;
  }

  std::vector<cmGeneratorTarget*> GetLocalGeneratorTargetsInOrder(
    cmLocalGenerator* lg) const;

  cmMakefile* GetCurrentMakefile() const
  {
    return this->CurrentConfigureMakefile;
  }

  void SetCurrentMakefile(cmMakefile* mf)
  {
    this->CurrentConfigureMakefile = mf;
  }

  void AddMakefile(std::unique_ptr<cmMakefile> mf);

  //! Set an generator for an "external makefile based project"
  void SetExternalMakefileProjectGenerator(
    std::unique_ptr<cmExternalMakefileProjectGenerator> extraGenerator);

  std::string GetExtraGeneratorName() const;

  void AddInstallComponent(std::string const& component);

  /** Mark the (absolute path to a) file as generated.  */
  void MarkAsGeneratedFile(std::string const& filepath);
  /** Determine if the absolute filepath belongs to a generated file.  */
  bool IsGeneratedFile(std::string const& filepath);

  std::set<std::string> const* GetInstallComponents() const
  {
    return &this->InstallComponents;
  }

  cmExportSetMap& GetExportSets() { return this->ExportSets; }

  cmValue GetGlobalSetting(std::string const& name) const;
  bool GlobalSettingIsOn(std::string const& name) const;
  std::string GetSafeGlobalSetting(std::string const& name) const;

  /** Add a file to the manifest of generated targets for a configuration.  */
  void AddToManifest(std::string const& f);

  void EnableInstallTarget();

  cmDuration TryCompileTimeout;

  bool GetForceUnixPaths() const { return this->ForceUnixPaths; }
  bool GetToolSupportsColor() const { return this->ToolSupportsColor; }

  //! return the language for the given extension
  std::string GetLanguageFromExtension(char const* ext) const;
  //! is an extension to be ignored
  bool IgnoreFile(char const* ext) const;
  //! What is the preference for linkers and this language (None or Preferred)
  int GetLinkerPreference(std::string const& lang) const;
  //! What is the object file extension for a given source file?
  std::string GetLanguageOutputExtension(cmSourceFile const&) const;
  //! What is the object file extension for a given language?
  std::string GetLanguageOutputExtension(std::string const& lang) const;

  //! What is the configurations directory variable called?
  virtual char const* GetCMakeCFGIntDir() const { return "."; }

  //! expand CFGIntDir for a configuration
  virtual std::string ExpandCFGIntDir(std::string const& str,
                                      std::string const& config) const;

  /** Get whether the generator should use a script for link commands.  */
  bool GetUseLinkScript() const { return this->UseLinkScript; }

  /** Get whether the generator should produce special marks on rules
      producing symbolic (non-file) outputs.  */
  bool GetNeedSymbolicMark() const { return this->NeedSymbolicMark; }

  /*
   * Determine what program to use for building the project.
   */
  virtual bool FindMakeProgram(cmMakefile*);

  //! Find a target by name by searching the local generators.
  cmTarget* FindTarget(std::string const& name,
                       cmStateEnums::TargetDomainSet domains = {
                         cmStateEnums::TargetDomain::NATIVE,
                         cmStateEnums::TargetDomain::ALIAS }) const;

  cmGeneratorTarget* FindGeneratorTarget(std::string const& name) const;

  void AddAlias(std::string const& name, std::string const& tgtName);
  bool IsAlias(std::string const& name) const;

  /** Determine if a name resolves to a framework on disk or a built target
      that is a framework. */
  bool NameResolvesToFramework(std::string const& libname) const;
  /** Split a framework path to the directory and name of the framework as well
   * as optional suffix.
   * Returns std::nullopt if the path does not match with framework format
   * when extendedFormat is true, required format is relaxed (i.e. extension
   * `.framework' is optional). Used when FRAMEWORK link feature is
   * specified */
  struct FrameworkDescriptor
  {
    FrameworkDescriptor(std::string directory, std::string name)
      : Directory(std::move(directory))
      , Name(std::move(name))
    {
    }
    FrameworkDescriptor(std::string directory, std::string version,
                        std::string name)
      : Directory(std::move(directory))
      , Version(std::move(version))
      , Name(std::move(name))
    {
    }
    FrameworkDescriptor(std::string directory, std::string version,
                        std::string name, std::string suffix)
      : Directory(std::move(directory))
      , Version(std::move(version))
      , Name(std::move(name))
      , Suffix(std::move(suffix))
    {
    }
    std::string GetLinkName() const
    {
      return this->Suffix.empty() ? this->Name
                                  : cmStrCat(this->Name, ',', this->Suffix);
    }
    std::string GetFullName() const
    {
      return cmStrCat(this->Name, ".framework/"_s, this->Name, this->Suffix);
    }
    std::string GetVersionedName() const
    {
      return this->Version.empty()
        ? this->GetFullName()
        : cmStrCat(this->Name, ".framework/Versions/"_s, this->Version, '/',
                   this->Name, this->Suffix);
    }
    std::string GetFrameworkPath() const
    {
      return this->Directory.empty()
        ? cmStrCat(this->Name, ".framework"_s)
        : cmStrCat(this->Directory, '/', this->Name, ".framework"_s);
    }
    std::string GetFullPath() const
    {
      return this->Directory.empty()
        ? this->GetFullName()
        : cmStrCat(this->Directory, '/', this->GetFullName());
    }
    std::string GetVersionedPath() const
    {
      return this->Directory.empty()
        ? this->GetVersionedName()
        : cmStrCat(this->Directory, '/', this->GetVersionedName());
    }

    std::string const Directory;
    std::string const Version;
    std::string const Name;
    std::string const Suffix;
  };
  enum class FrameworkFormat
  {
    Strict,
    Relaxed,
    Extended
  };
  cm::optional<FrameworkDescriptor> SplitFrameworkPath(
    std::string const& path,
    FrameworkFormat format = FrameworkFormat::Relaxed) const;

  cmMakefile* FindMakefile(std::string const& start_dir) const;
  cmLocalGenerator* FindLocalGenerator(cmDirectoryId const& id) const;

  /** Append the subdirectory for the given configuration.  If anything is
      appended the given prefix and suffix will be appended around it, which
      is useful for leading or trailing slashes.  */
  virtual void AppendDirectoryForConfig(std::string const& prefix,
                                        std::string const& config,
                                        std::string const& suffix,
                                        std::string& dir);

  /** Get the content of a directory.  Directory listings are cached
      and re-loaded from disk only when modified.  During the generation
      step the content will include the target files to be built even if
      they do not yet exist.  */
  std::set<std::string> const& GetDirectoryContent(std::string const& dir,
                                                   bool needDisk = true);

  void IndexTarget(cmTarget* t);
  void IndexGeneratorTarget(cmGeneratorTarget* gt);

  // Index the target using a name that is unique to that target
  // even if other targets have the same name.
  std::string IndexGeneratorTargetUniquely(cmGeneratorTarget const* gt);

  static bool IsReservedTarget(std::string const& name);

  virtual char const* GetAllTargetName() const { return "ALL_BUILD"; }
  virtual char const* GetInstallTargetName() const { return "INSTALL"; }
  virtual char const* GetInstallLocalTargetName() const { return nullptr; }
  virtual char const* GetInstallStripTargetName() const { return nullptr; }
  virtual char const* GetPreinstallTargetName() const { return nullptr; }
  virtual char const* GetTestTargetName() const { return "RUN_TESTS"; }
  virtual char const* GetPackageTargetName() const { return "PACKAGE"; }
  virtual char const* GetPackageSourceTargetName() const { return nullptr; }
  virtual char const* GetEditCacheTargetName() const { return nullptr; }
  virtual char const* GetRebuildCacheTargetName() const { return nullptr; }
  virtual char const* GetCleanTargetName() const { return nullptr; }

  // Lookup edit_cache target command preferred by this generator.
  virtual std::string GetEditCacheCommand() const { return ""; }

  // Default config to use for cmake --build
  virtual std::string GetDefaultBuildConfig() const { return "Debug"; }

  virtual cmValue GetDebuggerWorkingDirectory(cmGeneratorTarget* gt) const;

  // Class to track a set of dependencies.
  using TargetDependSet = cmTargetDependSet;

  // what targets does the specified target depend on directly
  // via a target_link_libraries or add_dependencies
  TargetDependSet const& GetTargetDirectDepends(
    cmGeneratorTarget const* target);

  // Return true if target 'l' occurs before 'r' in a global ordering
  // of targets that respects inter-target dependencies.
  bool TargetOrderIndexLess(cmGeneratorTarget const* l,
                            cmGeneratorTarget const* r) const;

  std::map<std::string, std::vector<cmLocalGenerator*>> const& GetProjectMap()
    const
  {
    return this->ProjectMap;
  }

  // track files replaced during a Generate
  void FileReplacedDuringGenerate(std::string const& filename);
  void GetFilesReplacedDuringGenerate(std::vector<std::string>& filenames);

  void AddRuleHash(std::vector<std::string> const& outputs,
                   std::string const& content);

  /** Return whether the given binary directory is unused.  */
  bool BinaryDirectoryIsNew(std::string const& dir)
  {
    return this->BinaryDirectories.insert(dir).second;
  }

  /** Return true if the generated build tree may contain multiple builds.
      i.e. "Can I build Debug and Release in the same tree?" */
  virtual bool IsMultiConfig() const { return false; }

  virtual bool IsXcode() const { return false; }

  virtual bool IsVisualStudio() const { return false; }

  virtual bool IsVisualStudioAtLeast10() const { return false; }

  virtual bool IsNinja() const { return false; }

  /** Return true if we know the exact location of object files for the given
     cmTarget. If false, store the reason in the given string. This is
     meaningful only after EnableLanguage has been called.  */
  virtual bool HasKnownObjectFileLocation(cmTarget const&, std::string*) const
  {
    return true;
  }

  virtual bool UseFolderProperty() const;

  virtual bool IsIPOSupported() const { return false; }

  /** Return whether the generator can import external visual studio project
      using INCLUDE_EXTERNAL_MSPROJECT */
  virtual bool IsIncludeExternalMSProjectSupported() const { return false; }

  /** Return whether the generator should use EFFECTIVE_PLATFORM_NAME. This is
      relevant for mixed macOS and iOS builds. */
  virtual bool UseEffectivePlatformName(cmMakefile*) const { return false; }

  /** Return whether the "Resources" folder prefix should be stripped from
      MacFolder. */
  virtual bool ShouldStripResourcePath(cmMakefile*) const;

  virtual bool SupportsCustomCommandDepfile() const { return false; }
  virtual cm::optional<cmDepfileFormat> DepfileFormat() const
  {
    return cm::nullopt;
  }

  virtual bool SupportsLinkerDependencyFile() const { return false; }

  /** Generate an <output>.rule file path for a given command output.  */
  virtual std::string GenerateRuleFile(std::string const& output) const;

  virtual bool SupportsDefaultBuildType() const { return false; }
  virtual bool SupportsCrossConfigs() const { return false; }
  virtual bool SupportsDefaultConfigs() const { return false; }

  virtual std::string ConvertToOutputPath(std::string path) const
  {
    return path;
  }
  virtual std::string GetConfigDirectory(std::string const& config) const
  {
    if (!this->IsMultiConfig() || config.empty()) {
      return {};
    }
    return cmStrCat('/', config);
  }

  static std::string EscapeJSON(std::string const& s);

  void ProcessEvaluationFiles();

  std::map<std::string, cmExportBuildFileGenerator*>& GetBuildExportSets()
  {
    return this->BuildExportSets;
  }
  void AddBuildExportSet(cmExportBuildFileGenerator* gen);
  void AddBuildExportExportSet(cmExportBuildFileGenerator* gen);
  bool IsExportedTargetsFile(std::string const& filename) const;
  cmExportBuildFileGenerator* GetExportedTargetsFile(
    std::string const& filename) const;
  void AddCMP0068WarnTarget(std::string const& target);

  virtual bool SupportsShortObjectNames() const;
  bool UseShortObjectNames(
    cmStateEnums::IntermediateDirKind kind =
      cmStateEnums::IntermediateDirKind::ObjectFiles) const;
  virtual std::string GetShortBinaryOutputDir() const;
  std::string ComputeTargetShortName(std::string const& bindir,
                                     std::string const& targetName) const;

  virtual void ComputeTargetObjectDirectory(cmGeneratorTarget* gt) const;

  bool GenerateCPackPropertiesFile();

  void SetFilenameTargetDepends(
    cmSourceFile* sf, std::set<cmGeneratorTarget const*> const& tgts);
  std::set<cmGeneratorTarget const*> const& GetFilenameTargetDepends(
    cmSourceFile* sf) const;

#if !defined(CMAKE_BOOTSTRAP)
  cmFileLockPool& GetFileLockPool() { return this->FileLockPool; }
#endif

  std::string MakeSilentFlag;

  size_t RecursionDepth = 0;

  virtual void GetQtAutoGenConfigs(std::vector<std::string>& configs) const
  {
    configs.emplace_back("$<CONFIG>");
  }

  std::string const& GetRealPath(std::string const& dir);

  std::string NewDeferId();

  cmInstallRuntimeDependencySet* CreateAnonymousRuntimeDependencySet();

  cmInstallRuntimeDependencySet* GetNamedRuntimeDependencySet(
    std::string const& name);

  enum class StripCommandStyle
  {
    Default,
    Apple,
  };
  StripCommandStyle GetStripCommandStyle(std::string const& strip);

  std::string GetEncodedLiteral(std::string const& lit);
  virtual std::string& EncodeLiteral(std::string& lit) { return lit; }

  bool CheckCMP0171() const;

  void AddInstallScript(std::string const& file);
  void AddTestFile(std::string const& file);
  void AddCMakeFilesToRebuild(std::vector<std::string>& files) const;

  virtual std::set<std::string> const& GetDefaultConfigs() const
  {
    static std::set<std::string> configs;
    return configs;
  }

  bool ShouldWarnExperimental(cm::string_view featureName,
                              cm::string_view featureUuid);

protected:
  // for a project collect all its targets by following depend
  // information, and also collect all the targets
  void GetTargetSets(TargetDependSet& projectTargets,
                     TargetDependSet& originalTargets, cmLocalGenerator* root,
                     std::vector<cmLocalGenerator*>& generators);
  bool IsRootOnlyTarget(cmGeneratorTarget* target) const;
  void AddTargetDepends(cmGeneratorTarget const* target,
                        TargetDependSet& projectTargets);
  void SetLanguageEnabledFlag(std::string const& l, cmMakefile* mf);
  void SetLanguageEnabledMaps(std::string const& l, cmMakefile* mf);
  void FillExtensionToLanguageMap(std::string const& l, cmMakefile* mf);
  virtual bool CheckLanguages(std::vector<std::string> const& languages,
                              cmMakefile* mf) const;
  virtual void PrintCompilerAdvice(std::ostream& os, std::string const& lang,
                                   cmValue envVar) const;

  virtual bool ComputeTargetDepends();

#if !defined(CMAKE_BOOTSTRAP)
  void WriteJsonContent(std::string const& fname,
                        Json::Value const& value) const;
  void WriteInstallJson() const;
#endif

  virtual bool CheckALLOW_DUPLICATE_CUSTOM_TARGETS() const;

  bool ApplyCXXStdTargets();
  bool DiscoverSyntheticTargets();

  bool AddHeaderSetVerification();

  void CreateFileGenerateOutputs();
  bool AddAutomaticSources();

  std::string SelectMakeProgram(std::string const& makeProgram,
                                std::string const& makeDefault = "") const;

  // Fill the ProjectMap, this must be called after LocalGenerators
  // has been populated.
  void FillProjectMap();
  void CheckTargetProperties();
  bool IsExcluded(cmStateSnapshot const& root,
                  cmStateSnapshot const& snp) const;
  bool IsExcluded(cmLocalGenerator const* root,
                  cmLocalGenerator const* gen) const;
  bool IsExcluded(cmLocalGenerator const* root,
                  cmGeneratorTarget const* target) const;
  virtual void InitializeProgressMarks() {}

  struct GlobalTargetInfo
  {
    std::string Name;
    std::string Message;
    cmCustomCommandLines CommandLines;
    std::vector<std::string> Depends;
    std::string WorkingDir;
    bool UsesTerminal = false;
    cmTarget::PerConfig PerConfig = cmTarget::PerConfig::Yes;
    bool StdPipesUTF8 = false;
    std::string Role;
  };

  void CreateDefaultGlobalTargets(std::vector<GlobalTargetInfo>& targets);

  void AddGlobalTarget_Package(std::vector<GlobalTargetInfo>& targets);
  void AddGlobalTarget_PackageSource(std::vector<GlobalTargetInfo>& targets);
  void AddGlobalTarget_Test(std::vector<GlobalTargetInfo>& targets);
  void AddGlobalTarget_EditCache(std::vector<GlobalTargetInfo>& targets) const;
  void AddGlobalTarget_RebuildCache(
    std::vector<GlobalTargetInfo>& targets) const;
  void AddGlobalTarget_Install(std::vector<GlobalTargetInfo>& targets);
  void CreateGlobalTarget(GlobalTargetInfo const& gti, cmMakefile* mf);

  void ReserveGlobalTargetCodegen();

  std::string FindMakeProgramFile;
  std::string ConfiguredFilesPath;
  cmake* CMakeInstance;
  std::vector<std::unique_ptr<cmMakefile>> Makefiles;
  LocalGeneratorVector LocalGenerators;

#ifndef CMAKE_BOOTSTRAP
  std::unique_ptr<cmQtAutoGenGlobalInitializer> QtAutoGen;
#endif

  cmMakefile* CurrentConfigureMakefile;
  // map from project name to vector of local generators in that project
  std::map<std::string, std::vector<cmLocalGenerator*>> ProjectMap;

  // Set of named installation components requested by the project.
  std::set<std::string> InstallComponents;
  // Sets of named target exports
  cmExportSetMap ExportSets;
  std::map<std::string, cmExportBuildFileGenerator*> BuildExportSets;
  std::map<std::string, cmExportBuildFileGenerator*> BuildExportExportSets;

  std::map<std::string, std::string> AliasTargets;

  cmTarget* FindTargetImpl(std::string const& name,
                           cmStateEnums::TargetDomainSet domains) const;

  cmGeneratorTarget* FindGeneratorTargetImpl(std::string const& name) const;

  std::string GetPredefinedTargetsFolder() const;

private:
  using TargetMap = std::unordered_map<std::string, cmTarget*>;
  using GeneratorTargetMap =
    std::unordered_map<std::string, cmGeneratorTarget*>;
  using MakefileMap = std::unordered_map<std::string, cmMakefile*>;
  using LocalGeneratorMap = std::unordered_map<std::string, cmLocalGenerator*>;
  // Map efficiently from target name to cmTarget instance.
  // Do not use this structure for looping over all targets.
  // It contains both normal and globally visible imported targets.
  TargetMap TargetSearchIndex;
  GeneratorTargetMap GeneratorTargetSearchIndex;

  // Map efficiently from source directory path to cmMakefile instance.
  // Do not use this structure for looping over all directories.
  // It may not contain all of them (see note in IndexMakefile method).
  MakefileMap MakefileSearchIndex;

  // Map efficiently from source directory path to cmLocalGenerator instance.
  // Do not use this structure for looping over all directories.
  // Its order is not deterministic.
  LocalGeneratorMap LocalGeneratorSearchIndex;

  void ComputeTargetOrder();
  void ComputeTargetOrder(cmGeneratorTarget const* gt, size_t& index);
  std::map<cmGeneratorTarget const*, size_t> TargetOrderIndex;

  cmMakefile* TryCompileOuterMakefile;
  // If you add a new map here, make sure it is copied
  // in EnableLanguagesFromGenerator
  std::map<std::string, bool> IgnoreExtensions;
  std::set<std::string> LanguagesReady; // Ready for try_compile
  std::set<std::string> LanguagesInProgress;
  std::map<std::string, std::string> OutputExtensions;
  std::map<std::string, std::string> LanguageToOutputExtension;
  std::map<std::string, std::string> ExtensionToLanguage;
  std::map<std::string, int> LanguageToLinkerPreference;

#if !defined(CMAKE_BOOTSTRAP)
  std::unique_ptr<Json::StreamWriter> JsonWriter;
#endif

#ifdef __APPLE__
  std::map<std::string, StripCommandStyle> StripCommandStyleMap;
#endif

  // Deferral id generation.
  size_t NextDeferId = 0;

  // Record hashes for rules and outputs.
  struct RuleHash
  {
    char Data[32];
  };
  std::map<std::string, RuleHash> RuleHashes;
  void CheckRuleHashes();
  void CheckRuleHashes(std::string const& pfile, std::string const& home);
  void WriteRuleHashes(std::string const& pfile);

  void WriteSummary();
  void WriteSummary(cmGeneratorTarget* target);
  void FinalizeTargetConfiguration();

  virtual void ForceLinkerLanguages();

  void CheckTargetLinkLibraries() const;
  bool CheckTargetsForMissingSources() const;
  bool CheckTargetsForType() const;
  void MarkTargetsForPchReuse() const;

  void CreateLocalGenerators();

  void CheckCompilerIdCompatibility(cmMakefile* mf,
                                    std::string const& lang) const;

  void ComputeBuildFileGenerators();

  std::unique_ptr<cmExternalMakefileProjectGenerator> ExtraGenerator;

  // track files replaced during a Generate
  std::vector<std::string> FilesReplacedDuringGenerate;

  // Store computed inter-target dependencies.
  using TargetDependMap = std::map<cmGeneratorTarget const*, TargetDependSet>;
  TargetDependMap TargetDependencies;

  friend class cmake;
  void CreateGeneratorTargets(
    TargetTypes targetTypes, cmMakefile* mf, cmLocalGenerator* lg,
    std::map<cmTarget*, cmGeneratorTarget*> const& importedMap);
  void CreateGeneratorTargets(TargetTypes targetTypes);

  void ClearGeneratorMembers();

  bool CheckReservedTargetName(std::string const& targetName,
                               std::string const& reason) const;
  bool CheckReservedTargetNamePrefix(std::string const& targetPrefix,
                                     std::string const& reason) const;

  void IndexMakefile(cmMakefile* mf);
  void IndexLocalGenerator(cmLocalGenerator* lg);

  virtual char const* GetBuildIgnoreErrorsFlag() const { return nullptr; }

  bool UnsupportedVariableIsDefined(std::string const& name,
                                    bool supported) const;

  // Cache directory content and target files to be built.
  struct DirectoryContent
  {
    long LastDiskTime = -1;
    std::set<std::string> All;
    std::set<std::string> Generated;
  };
  std::map<std::string, DirectoryContent> DirectoryContentMap;

  // Set of binary directories on disk.
  std::set<std::string> BinaryDirectories;

  // track targets to issue CMP0068 warning for.
  std::set<std::string> CMP0068WarnTargets;

  std::unordered_set<std::string> WarnedExperimental;

  mutable std::map<cmSourceFile*, std::set<cmGeneratorTarget const*>>
    FilenameTargetDepends;

  std::map<std::string, std::string> RealPaths;

  std::unordered_set<std::string> GeneratedFiles;

  std::vector<std::unique_ptr<cmInstallRuntimeDependencySet>>
    RuntimeDependencySets;
  std::map<std::string, cmInstallRuntimeDependencySet*>
    RuntimeDependencySetsByName;

  std::vector<std::string> InstallScripts;
  std::vector<std::string> TestFiles;

#if !defined(CMAKE_BOOTSTRAP)
  // Pool of file locks
  cmFileLockPool FileLockPool;
#endif

  using PerLanguageModuleDatabases =
    std::map<std::string, std::vector<std::string>>;
  using PerConfigModuleDatabases =
    std::map<std::string, PerLanguageModuleDatabases>;
  PerConfigModuleDatabases PerConfigModuleDbs;
  PerLanguageModuleDatabases PerLanguageModuleDbs;

  enum class IntermediateDirStrategy
  {
    Full,
    Short,
  };
  IntermediateDirStrategy IntDirStrategy = IntermediateDirStrategy::Full;
  IntermediateDirStrategy QtAutogenIntDirStrategy =
    IntermediateDirStrategy::Full;

protected:
  float FirstTimeProgress;
  bool NeedSymbolicMark;
  bool UseLinkScript;
  bool ForceUnixPaths;
  bool ToolSupportsColor;
  bool InstallTargetEnabled;
  bool AllowGlobalTargetCodegen;
};
