#include "ConfigDefines.h"
#include "OptionParser.h"
#include "Common/FEXServerClient.h"
#include "git_version.h"
#include <FEXCore/Config/Config.h>

#include <filesystem>
#include <stdio.h>
#include <string>

int main(int argc, char **argv, char **envp) {
  FEXCore::Config::Initialize();
  FEXCore::Config::AddLayer(FEXCore::Config::CreateMainLayer());
  // No FEX arguments passed through command line
  FEXCore::Config::AddLayer(FEXCore::Config::CreateEnvironmentLayer(envp));
  FEXCore::Config::Load();

  // Load the arguments
  optparse::OptionParser Parser = optparse::OptionParser()
    .description("Simple application to get a couple of FEX options");

  Parser.add_option("--install-prefix")
    .action("store_true")
    .help("Print the FEX install prefix");

  Parser.add_option("--app")
    .help("Load an application profile for this application if it exists");

  Parser.add_option("--current-rootfs")
    .action("store_true")
    .help("Print the directory that contains the FEX rootfs. Mounted in the case of squashfs");

  Parser.add_option("--version")
    .action("store_true")
    .help("Print the installed FEX-Emu version");

  optparse::Values Options = Parser.parse_args(argc, argv);

  if (Options.is_set_by_user("app")) {
    // Load the application config if one was provided
    auto ProgramName = std::filesystem::path(Options["app"]).filename();
    FEXCore::Config::AddLayer(FEXCore::Config::CreateAppLayer(ProgramName, true));
    FEXCore::Config::AddLayer(FEXCore::Config::CreateAppLayer(ProgramName, false));
  }

  // Reload the meta layer
  FEXCore::Config::ReloadMetaLayer();

  if (Options.is_set_by_user("version")) {
    fprintf(stdout, GIT_DESCRIBE_STRING "\n");
  }

  if (Options.is_set_by_user("install_prefix")) {
    fprintf(stdout, FEX_INSTALL_PREFIX "\n");
  }

  if (Options.is_set_by_user("current_rootfs")) {
    int ServerFD = FEXServerClient::ConnectToServer();
    if (ServerFD != -1) {
      auto RootFS = FEXServerClient::RequestRootFSPath(ServerFD);
      if (!RootFS.empty()) {
        fprintf(stdout, "%s\n", RootFS.c_str());
      }
    }
  }

  return 0;
}
