#include "ArcadeUtils.h"
#include <nlohmann/json.hpp>
#include "string_format.h"
#include "StdStreamUtils.h"
#include "PS2VM.h"
#include "PS2VM_Preferences.h"
#include "AppConfig.h"
#include "BootablesProcesses.h"
#include "iop/ioman/McDumpDevice.h"

struct ARCADE_MACHINE_DEF
{
	std::string id;
	std::string dongleFileName;
	std::string boot;
};

ARCADE_MACHINE_DEF ReadArcadeMachineDefinition(const fs::path& arcadeDefPath)
{
	auto defString =
	    [&arcadeDefPath]() {
		    auto defStream = Framework::CreateInputStdStream(arcadeDefPath.native());
		    return defStream.ReadString();
	    }();

	auto defJson = nlohmann::json::parse(defString);
	ARCADE_MACHINE_DEF def;
	def.id = defJson["id"];
	def.dongleFileName = defJson["dongle"]["name"];
	def.boot = defJson["boot"];
	return def;
}

void ArcadeUtils::BootArcadeMachine(CPS2VM* virtualMachine, const fs::path& arcadeDefPath)
{
	auto def = ReadArcadeMachineDefinition(arcadeDefPath);

	//Reset PS2VM
	virtualMachine->Pause();
	virtualMachine->Reset();

	auto romArchiveFileName = string_format("%s.zip", def.id.c_str());

	fs::path arcadeRomPath = CAppConfig::GetInstance().GetPreferencePath(PREF_PS2_ARCADEROMS_DIRECTORY);
	fs::path arcadeRomArchivePath = arcadeRomPath / romArchiveFileName;

	if(!fs::exists(arcadeRomArchivePath))
	{
		throw std::runtime_error(string_format("Failed to find '%s' in arcade ROMs directory.", romArchiveFileName.c_str()));
	}

	std::vector<uint8> mcDumpContents;

	{
		auto inputStream = Framework::CreateInputStdStream(arcadeRomArchivePath.native());
		Framework::CZipArchiveReader archiveReader(inputStream);
		auto header = archiveReader.GetFileHeader(def.dongleFileName.c_str());
		if(!header)
		{
			throw std::runtime_error("Failed to get header from ZIP archive.");
		}
		auto fileStream = archiveReader.BeginReadFile(def.dongleFileName.c_str());
		mcDumpContents.resize(header->uncompressedSize);
		fileStream->Read(mcDumpContents.data(), header->uncompressedSize);
	}

	//Override mc0 device with special device reading directly from zip file
	{
		auto device = std::make_shared<Iop::Ioman::CMcDumpDevice>(std::move(mcDumpContents));
		auto iopBios = dynamic_cast<CIopBios*>(virtualMachine->m_iop->m_bios.get());
		iopBios->GetIoman()->RegisterDevice("mc0", device);
	}

	//Boot mc0:/BOOT (from def)
	virtualMachine->m_ee->m_os->BootFromVirtualPath(def.boot.c_str(), CPS2OS::ArgumentList());

	//Mount CHD DVD or HDD
	//Apply Patches

	TryUpdateLastBootedTime(arcadeDefPath);
}
