#include "Common/ArgumentLoader.h"
#include "Common/Config.h"

#include <FEXCore/Config/Config.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <list>
#include <unordered_map>
#include <utility>
#include <json-maker.h>

namespace FEX::Config {
  static const std::map<FEXCore::Config::ConfigOption, std::string> ConfigToNameLookup = {{
#define OPT_BASE(type, group, enum, json, default) {FEXCore::Config::ConfigOption::CONFIG_##enum, #json},
#include <FEXCore/Config/ConfigValues.inl>
  }};

  void SaveLayerToJSON(const std::string& Filename, FEXCore::Config::Layer *const Layer) {
    char Buffer[4096];
    char *Dest{};
    Dest = json_objOpen(Buffer, nullptr);
    Dest = json_objOpen(Dest, "Config");
    for (auto &it : Layer->GetOptionMap()) {
			auto &Name = ConfigToNameLookup.find(it.first)->second;
			for (auto &var : it.second) {
				Dest = json_str(Dest, Name.c_str(), var.c_str());
			}
    }
    Dest = json_objClose(Dest);
    Dest = json_objClose(Dest);
    json_end(Dest);

		std::ofstream Output (Filename, std::ios::out | std::ios::binary);
		if (Output.is_open()) {
			Output.write(Buffer, strlen(Buffer));
			Output.close();
		}
  }

  std::string LoadConfig(
    bool NoFEXArguments,
    bool LoadProgramConfig,
    int argc,
    char **argv,
    char **const envp) {
    FEXCore::Config::Initialize();
    FEXCore::Config::AddLayer(FEXCore::Config::CreateMainLayer());

    if (NoFEXArguments) {
      FEX::ArgLoader::LoadWithoutArguments(argc, argv);
    }
    else {
      FEXCore::Config::AddLayer(std::make_unique<FEX::ArgLoader::ArgLoader>(argc, argv));
    }

    FEXCore::Config::AddLayer(FEXCore::Config::CreateEnvironmentLayer(envp));
    FEXCore::Config::Load();

    auto Args = FEX::ArgLoader::Get();

    if (LoadProgramConfig) {
      if (Args.empty()) {
        // Early exit if we weren't passed an argument
        return {};
      }

      std::string Program = Args[0];

      // These layers load on initialization
      auto ProgramName = std::filesystem::path(Program).filename();
      if (ProgramName == "wine" ||
          ProgramName == "wine64") {

        // If we are running wine or wine64 then we should check the second argument for the application name instead.
        // wine will change the active program name with `setprogname` or `prctl(PR_SET_NAME`.
        // Since FEX needs this data far earlier than libraries we need a different check.
        if (Args.size() > 1) {
          ProgramName = std::filesystem::path(Args[1]).filename();

          // If this was path separated with '\' then we need to check that.
          auto WinSeparator = ProgramName.string().find_last_of('\\');
          if (WinSeparator != ProgramName.string().npos) {
            // Used windows separators
            ProgramName = ProgramName.string().substr(WinSeparator + 1);
          }
        }
      }

      FEXCore::Config::AddLayer(FEXCore::Config::CreateAppLayer(ProgramName, true));
      FEXCore::Config::AddLayer(FEXCore::Config::CreateAppLayer(ProgramName, false));
      return Program;
    }
    return {};
  }
}
