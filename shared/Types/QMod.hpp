#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <memory>
#include <mutex>

#include "cpp-semver/shared/cpp-semver.hpp"

#include "qmod-utils/shared/Types/Dependency.hpp"
#include "qmod-utils/shared/Types/FileCopy.hpp"
#include "qmod-utils/shared/WebUtils.hpp"

#include "jni-utils/shared/JNIUtils.hpp"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/writer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/filewritestream.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/error/error.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/error/en.h"

#define ASSERT(condition, name, verbos)                                                                                                  \
	{                                                                                                                                    \
		if (!(condition))                                                                                                                \
		{                                                                                                                                \
			if (verbos)                                                                                                                  \
				getLogger().info("[%s] QMOD ASSERT [%s:%i]: Condition \"%s\" Failed!", name.c_str(), __FILE__, __LINE__, "" #condition); \
                                                                                                                                         \
			CleanupTempDir(name);                                                                                                        \
			CleanupTempDir(string_format("Downloads/%s.qmod", name.c_str()), true);                                                      \
                                                                                                                                         \
			m_Valid = false;                                                                                                             \
			return;                                                                                                                      \
		}                                                                                                                                \
	}

#define GET_STRING(value, parentObject) (parentObject.HasMember(value) && parentObject[value].IsString()) ? parentObject[value].GetString() : ""
#define GET_BOOL(value, parentObject) (parentObject.HasMember(value) && parentObject[value].IsBool()) ? parentObject[value].GetBool() : false

#define CREATE_STRING_VALUE(string)

#define ADD_MEMBER(name, value, object, allocator) \
	if (object.HasMember(name))                    \
	{                                              \
		object.RemoveMember(name);                 \
	}                                              \
	object.AddMember(name, value, allocator)

#define ADD_MEMBER_NULL(name, object, allocator) ADD_MEMBER(name, rapidjson::Value(rapidjson::Type::kNullType), object, allocator);

#define ADD_STRING_MEMBER(name, value, object, allocator)     \
	if (value == "")                                          \
	{                                                         \
		ADD_MEMBER_NULL(name, object, allocator);             \
	}                                                         \
	else                                                      \
	{                                                         \
		rapidjson::Value str;                                 \
		str.SetString(value.data(), value.size(), allocator); \
		ADD_MEMBER(name, str, object, allocator);             \
	}

#define GET_ARRAY(valueName, array, type, parentObject)                         \
	if (parentObject.HasMember(valueName) && parentObject[valueName].IsArray()) \
	{                                                                           \
		auto &value = parentObject[valueName];                                  \
                                                                                \
		for (rapidjson::SizeType i = 0; i < value.Size(); i++)                  \
		{                                                                       \
			array->push_back(value[i].Get##type());                             \
		}                                                                       \
	}

#define GET_DEPENDENCIES(valueName, dependencies, parentObject)                                   \
	if (parentObject.HasMember(valueName) && parentObject[valueName].IsArray())                   \
	{                                                                                             \
		auto &value = parentObject[valueName];                                                    \
                                                                                                  \
		for (rapidjson::SizeType i = 0; i < value.Size(); i++)                                    \
		{                                                                                         \
			if (value[i].IsObject())                                                              \
			{                                                                                     \
				const rapidjson::Value &dependencyValue = value[i];                               \
                                                                                                  \
				std::string id = GET_STRING("id", dependencyValue);                               \
				std::string version = GET_STRING("version", dependencyValue);                     \
				std::string downloadIfMissing = GET_STRING("downloadIfMissing", dependencyValue); \
                                                                                                  \
				dependencies->push_back({id, version, downloadIfMissing});                        \
			}                                                                                     \
		}                                                                                         \
	}

#define GET_FILE_COPIES(valueName, fileCopies, parentObject)                        \
	if (parentObject.HasMember(valueName) && parentObject[valueName].IsArray())     \
	{                                                                               \
		auto &value = parentObject[valueName];                                      \
                                                                                    \
		for (rapidjson::SizeType i = 0; i < value.Size(); i++)                      \
		{                                                                           \
			if (value[i].IsObject())                                                \
			{                                                                       \
				const rapidjson::Value &fileCopyValue = value[i];                   \
                                                                                    \
				std::string name = GET_STRING("name", fileCopyValue);               \
				std::string destination = GET_STRING("destination", fileCopyValue); \
                                                                                    \
				fileCopies->push_back({name, destination});                         \
			}                                                                       \
		}                                                                           \
	}

namespace QModUtils
{
	class QMod
	{
	public:
		QMod(std::string fileDir, bool verbos = true, bool cleanUpTempDir = true)
		{
			std::string tmpDir = GetTempDir(fileDir);

			// Create Temp Dir To Read the mod.json

			std::system(string_format("mkdir -p \"%s\"", tmpDir.c_str()).c_str());
			std::system(string_format("unzip \"%s\" mod.json -d \"%s\"", fileDir.c_str(), tmpDir.c_str()).c_str());

			// Read the mod.json

			std::ifstream qmodFile(string_format("%smod.json", tmpDir.c_str()).c_str());

			std::stringstream qmodJson;
			qmodJson << qmodFile.rdbuf();

			rapidjson::Document document;
			ASSERT(!document.Parse(qmodJson.str().c_str()).HasParseError(), GetFileName(fileDir), verbos);

			// Clean Up Temp Dirs
			if (cleanUpTempDir)
				CleanupTempDir(GetFileName(fileDir));

			// Get Values

			m_Name = GET_STRING("name", document);
			m_Id = GET_STRING("id", document);
			m_Description = GET_STRING("description", document);
			m_Author = GET_STRING("author", document);
			m_Porter = GET_STRING("porter", document);
			m_Version = GET_STRING("version", document);
			m_CoverImage = GET_STRING("coverImage", document);
			m_PackageId = GET_STRING("packageId", document);
			m_PackageVersion = GET_STRING("packageVersion", document);

			m_ModFiles = new std::vector<std::string>();
			GET_ARRAY("modFiles", m_ModFiles, String, document);

			m_LibraryFiles = new std::vector<std::string>();
			GET_ARRAY("libraryFiles", m_LibraryFiles, String, document);

			m_Dependencies = new std::vector<Dependency>();
			GET_DEPENDENCIES("dependencies", m_Dependencies, document);

			m_FileCopies = new std::vector<FileCopy>();
			GET_FILE_COPIES("fileCopies", m_FileCopies, document);

			m_Path = fileDir;

			m_IsLibrary = GET_BOOL("isLibrary", document);

			// Attempt to load BMBF Specific Data
			GetBMBFData(verbos);

			m_DownloadedQMods->insert({this->m_Id, this});
			m_Valid = true;
		}

		void Install(bool blocking = false, std::vector<std::string> *installedInBranch = new std::vector<std::string>())
		{
			std::optional<std::thread> thread = InstallAsync(installedInBranch);
			if (thread.has_value())
			{
				if (blocking)
					thread.value().join();
				else
					thread.value().detach();
			}
		}

		static void InstallFromUrl(std::string fileName, std::string url, bool blocking = false, std::vector<std::string> *installedInBranch = new std::vector<std::string>())
		{
			std::optional<std::thread> thread = InstallFromUrlAsync(fileName, url, installedInBranch);
			if (thread.has_value())
			{
				if (blocking)
					thread.value().join();
				else
					thread.value().detach();
			}
		}

		std::optional<std::thread> InstallAsync(std::vector<std::string> *installedInBranch = new std::vector<std::string>())
		{
			if (!m_Valid)
			{
				getLogger().info("Mod \"%s\" Is an invalid QMod!", m_Id.c_str());
				return std::nullopt;
			}

			Cache_AppPackageId();
			if (m_PackageId != m_AppPackageId)
			{
				getLogger().info("Mod \"%s\" Is not built for the package \"%s\", but instead is built for \"%s\"!", m_Id.c_str(), m_AppPackageId.c_str(), m_PackageId.c_str());
				return std::nullopt;
			}

			return std::thread(
				[this, installedInBranch]
				{
					if (m_Installed)
					{
						getLogger().info("Mod \"%s\" Already Installed!", m_Id.c_str());
						return;
					}

					getLogger().info("Installing mod \"%s\"", m_Id.c_str());

					// Add to the installed tree so that dependencies further down on us will trigger a recursive install error
					installedInBranch->push_back(m_Id);

					// We say that the mod is installed now to prevent multiple installs of the same mod If the install fails we can then can say its uninstalled later
					m_Installed = true;

					for (Dependency dependency : *m_Dependencies)
					{
						if (!PrepareDependency(dependency, installedInBranch))
						{
							getLogger().error("Failed to install \"%s\" as one of its dependencies (%s) also failed to install", m_Id.c_str(), dependency.id.c_str());

							m_Installed = false;
							return;
						}
					}

					// We only lock now so that the dependencies can install first without issues
					std::unique_lock guard(m_InstallLock);

					// Extract QMod so we can move the files
					ExtractQMod();

					std::string tmpDir = GetTempDir(m_Path);
					std::string modsExtractionPath = tmpDir + "Mods/";
					std::string libsExtractionPath = tmpDir + "Libs/";
					std::string fileCopiesExtractionPath = tmpDir + "FileCopies/";

					// Copy the Mods files to the Mods folder
					for (std::string mod : *m_ModFiles)
					{
						std::system(string_format("mv -f \"%s%s\" \"/sdcard/Android/data/com.beatgames.beatsaber/files/mods/\"", modsExtractionPath.c_str(), mod.c_str()).c_str());
					}

					// Copy the Libs files to the Libs folder
					for (std::string lib : *m_LibraryFiles)
					{
						std::system(string_format("mv -f \"%s%s\" \"/sdcard/Android/data/com.beatgames.beatsaber/files/libs/\"", libsExtractionPath.c_str(), lib.c_str()).c_str());
					}

					// Copy the File Copies to their respective destination folders
					for (FileCopy fileCopy : *m_FileCopies)
					{
						std::string desPath = fileCopy.destination.substr(0, fileCopy.destination.find_last_of("/\\"));

						std::system(string_format("mkdir -p \"%s\"", desPath.c_str()).c_str());
						std::remove(fileCopy.destination.c_str());

						std::system(string_format("mv -f \"%s%s\" \"%s\"", fileCopiesExtractionPath.c_str(), fileCopy.name.c_str(), fileCopy.destination.c_str()).c_str());
					}

					installedInBranch->erase(std::remove(installedInBranch->begin(), installedInBranch->end(), m_Id), installedInBranch->end());

					// If QMod is for Beat Saber, then Update its BMBF Data
					if (!strcmp(m_PackageId.c_str(), "com.beatgames.beatsaber"))
					{
						UpdateBMBFData();
					}

					getLogger().info("Successfully Installed \"%s\"!", m_Id.c_str());
					CleanupTempDir(GetFileName(m_Path));
				});
		}

		std::optional<std::thread> UninstallAsync(bool onlyDisable = true)
		{
			if (!m_Valid)
			{
				getLogger().info("Failed to uninstall \"%s\", Mod Is an invalid QMod!", m_Id.c_str());
				return std::nullopt;
			}

			if (!m_Uninstallable)
			{
				getLogger().warning("\"%s\" is marked as not being Uninstallable, this probably means you are uninstalling a core mod. Be careful!", m_Id.c_str());
			}

			return std::thread(
				[this, onlyDisable]
				{
					std::unique_lock guard(m_InstallLock);

					if (!m_Installed && onlyDisable)
					{
						// We only wanna return if we are only tryna disable the mod.
						// If were tryna remove it, it doesnt matter if its installed or not

						getLogger().info("Mod \"%s\" is already uninstalled!", m_Id.c_str());
						return;
					}

					getLogger().info("Uninstalling \"%s\"", m_Id.c_str());

					// Remove mod SOs so that the mod will not load
					for (std::string modFile : *m_ModFiles)
					{
						getLogger().info("Removing Mod file \"%s\" from mod \"%s\"", modFile.c_str(), m_Id.c_str());

						std::system(string_format("rm -f \"/sdcard/Android/data/com.beatgames.beatsaber/files/mods/%s\"", modFile.c_str()).c_str());
					}

					// Only Remove Libs if they are not needed elsewhere
					for (std::string libFile : *m_LibraryFiles)
					{
						bool isUsedElsewhere = false;
						for (std::pair<std::string, QMod *> modPair : *m_DownloadedQMods)
						{
							QMod *otherMod = modPair.second;
							if (otherMod == this || !otherMod->m_Installed)
								continue;

							if (std::count(otherMod->m_LibraryFiles->begin(), otherMod->m_LibraryFiles->end(), libFile))
							{
								getLogger().info("Lib File \"%s\" is used elsewhere, not removing", libFile.c_str());

								isUsedElsewhere = true;
								break;
							}
						}

						if (!isUsedElsewhere)
						{
							getLogger().info("Removing Library file \"%s\" from mod \"%s\"", libFile.c_str(), m_Id.c_str());

							std::system(string_format("rm -f \"/sdcard/Android/data/com.beatgames.beatsaber/files/libs/%s\"", libFile.c_str()).c_str());
						}
					}

					// Remove file copies
					for (FileCopy fileCopy : *m_FileCopies)
					{
						getLogger().info("Removing copied file \"%s\" from mod \"%s\"", fileCopy.destination.c_str(), m_Id.c_str());

						std::system(string_format("rm -f \"%s\"", fileCopy.destination.c_str()).c_str());
					}

					m_Installed = false;

					// If QMod is for Beat Saber, then Remove its BMBF Data
					if (!strcmp(m_PackageId.c_str(), "com.beatgames.beatsaber"))
					{
						if (onlyDisable)
							UpdateBMBFData();
						else
							RemoveBMBFData();
					}

					// This is for actually removing the qmod, not just disabling it
					if (!onlyDisable)
					{
						m_DownloadedQMods->erase(m_Id);

						std::system(string_format("rm -f \"sdcard/BMBFData/Mods/%s_%s\"", GetFileName(m_Path).c_str(), m_CoverImage.c_str()).c_str());
						std::system(string_format("rm -f \"%s\"", m_Path.c_str()).c_str());
					}

					guard.unlock();

					CleanDependentMods(true);
					if (!m_IsLibrary)
						CleanUnusedLibraries(true);

					CleanupTempDir(GetFileName(m_Path));
					getLogger().info("Successfully Uninstalled \"%s\"!", m_Id.c_str());
				});
		}

		void Uninstall(bool onlyDisable = true, bool blocking = false)
		{
			std::optional<std::thread> thread = UninstallAsync(onlyDisable);
			if (thread.has_value())
			{
				if (blocking)
					thread.value().join();
				else
					thread.value().detach();
			}
		}

		static std::optional<std::thread> InstallFromUrlAsync(std::string fileName, std::string url, std::vector<std::string> *installedInBranch = new std::vector<std::string>())
		{
			Cache_AppPackageId();

			return std::thread(
				[fileName, url, installedInBranch]
				{
					std::string downloadFileLoc = string_format("/sdcard/BMBFData/Mods/Temp/Downloads/%s", fileName.c_str());

					if (!WebUtils::DownloadFile(fileName, url, downloadFileLoc))
					{
						CleanupTempDir(string_format("Downloads/%s", fileName.c_str()).c_str(), true);
						return;
					}

					// NOTE: There is no clean up here because the cleanup will occur during the install
					QMod *downloadedMod = new QMod(downloadFileLoc);
					downloadedMod->m_Installed = false;

					downloadedMod->Install(true, installedInBranch);
				});
		}

		void UpdateBMBFData(bool verbos = true)
		{
			// Prevents multiple threads writing to the file at the same time
			std::unique_lock<std::mutex> guard(m_BmbfConfigLock);

			if (verbos)
				getLogger().info("Updating BMBF Info for \"%s\"", m_Id.c_str());

			// Read the config.json file
			std::ifstream configFile("/sdcard/BMBFData/config.json");
			ASSERT(configFile.good(), GetFileName(m_Path), verbos);

			std::stringstream configJson;
			configJson << configFile.rdbuf();

			rapidjson::Document document;

			ASSERT(!document.Parse(configJson.str().c_str()).HasParseError(), GetFileName(m_Path), verbos);
			ASSERT(document.HasMember("Mods") && document["Mods"].IsArray(), GetFileName(m_Path), verbos);

			const auto &mods = document["Mods"].GetArray();
			bool foundMod = false;

			std::string fileName = GetFileName(m_Path, false);
			std::string displayName = GetFileName(m_Path);

			// Move QMod
			std::system(string_format("mv -f \"%s\" \"/sdcard/BMBFData/Mods/%s\"", m_Path.c_str(), fileName.c_str()).c_str());
			m_Path = string_format("/sdcard/BMBFData/Mods/%s", fileName.c_str()).c_str();

			// Attempt To Install The Cover

			if (m_CoverImage != "")
			{
				std::string tmpDir = GetTempDir(m_Path);

				std::system(string_format("mkdir -p \"%s\"", tmpDir.c_str()).c_str());

				std::system(string_format("unzip \"%s\" \"%s\" -d \"%s\"", m_Path.c_str(), m_CoverImage.c_str(), tmpDir.c_str()).c_str());

				std::system(string_format("mv -f \"%s/%s\" \"sdcard/BMBFData/Mods/%s_%s\"", tmpDir.c_str(), m_CoverImage.c_str(), displayName.c_str(), m_CoverImage.c_str()).c_str());

				m_CoverImageFilename = string_format("%s_%s", displayName.c_str(), m_CoverImage.c_str());
			}

			// Try Find Our Mod in the BMBF Data
			for (auto &mod : mods)
			{
				// Find our mod id, then read the data

				std::string id = GET_STRING("Id", mod);

				if (id != m_Id)
					continue;
				foundMod = true;

				if (verbos)
					getLogger().info("Found existing BMBF Data for \"%s\", Updating It...", m_Id.c_str());

				mod.SetObject();
				UpdateBMBFJSONData(mod, document.GetAllocator());
			}

			// BMBF Data could not be found, create the object first
			if (!foundMod)
			{
				if (verbos)
					getLogger().info("No BMBF Data Found for \"%s\"! Creating It Now...", m_Id.c_str());

				rapidjson::Value modDataObject = rapidjson::Value(rapidjson::Type::kObjectType);

				UpdateBMBFJSONData(modDataObject, document.GetAllocator());

				std::string id = GET_STRING("Id", modDataObject);
				mods.PushBack(modDataObject, document.GetAllocator());
			}

			if (verbos)
				getLogger().info("Updated BMBF Data for \"%s\"! Saving...", m_Id.c_str());

			// Save To Buffer

			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

			document.Accept(writer);

			// Write To File

			std::ofstream out("/sdcard/BMBFData/config.json");
			out << buffer.GetString();
			out.close();

			if (verbos)
				getLogger().info("Saved BMBF Data for \"%s\"!", m_Id.c_str());
		}

		inline std::string Name() const { return m_Name; }
		inline std::string Id() const { return m_Id; }
		inline std::string Description() const { return m_Description; }
		inline std::string Author() const { return m_Author; }
		inline std::string Porter() const { return m_Porter; }
		inline std::string Version() const { return m_Version; }
		inline std::string CoverImage() const { return m_CoverImage; }

		inline std::string PackageId() const { return m_PackageId; }
		inline std::string PackageVersion() const { return m_PackageVersion; }

		inline std::vector<std::string> ModFiles() const { return *m_ModFiles; }
		inline std::vector<std::string> LibraryFiles() const { return *m_LibraryFiles; }
		inline std::vector<Dependency> Dependencies() const { return *m_Dependencies; }
		inline std::vector<FileCopy> FileCopies() const { return *m_FileCopies; }

		inline std::string Path() const { return m_Path; }
		inline std::string CoverImageFilename() const { return m_CoverImageFilename; }

		inline bool Uninstallable() const { return m_Uninstallable; }
		inline bool IsInstalled() const { return m_Installed; }
		inline bool IsLibrary() const { return m_IsLibrary; }
		inline bool Valid() const { return m_Valid; }

		inline std::string FileName() const { return GetFileName(m_Path, false, true); }

		static inline std::unordered_map<std::string, QMod *> *GetDownloadedQMods() { return m_DownloadedQMods; }
		static inline std::unordered_map<std::string, QMod *> *GetCoreMods() { return m_CoreMods; }

		void SetName(std::string val) { m_Name = val; }
		void SetId(std::string val) { m_Id = val; }
		void SetDescription(std::string val) { m_Description = val; }
		void SetAuthor(std::string val) { m_Author = val; }
		void SetPorter(std::string val) { m_Porter = val; }
		void SetVersion(std::string val) { m_Version = val; }
		void SetCoverImage(std::string val) { m_CoverImage = val; }

		void SetPackageId(std::string val) { m_PackageId = val; }
		void SetPackageVersion(std::string val) { m_PackageVersion = val; }

		void SetModFiles(std::vector<std::string> *val) { m_ModFiles = val; }
		void SetLibraryFiles(std::vector<std::string> *val) { m_LibraryFiles = val; }
		void SetDependencies(std::vector<Dependency> *val) { m_Dependencies = val; }
		void SetFileCopies(std::vector<FileCopy> *val) { m_FileCopies = val; }

		// We must called UpdateBMBFData when changing the path, cus it'll break many things if we dont move the qmod
		void SetPath(std::string val)
		{
			m_Path = val;
			UpdateBMBFData();
		}

		void SetUninstallable(bool val) { m_Uninstallable = val; }

		static std::optional<QMod *> GetDownloadedQMod(std::string id)
		{
			auto search = m_DownloadedQMods->find(id);
			if (search != m_DownloadedQMods->end())
				return search->second;

			return std::nullopt;
		}

		static bool IsCoreMod(QMod *qmod)
		{
			return qmod->IsCoreMod();
		}

		bool IsCoreMod() const
		{
			for (std::pair<std::string, QMod *> coreModPair : *m_CoreMods)
			{
				if (coreModPair.first == m_Id)
					return true;
			}

			return false;
		}

		std::vector<QMod *> FindModsDependingOn(bool onlyInstalledMods = false) const
		{
			std::vector<QMod *> dependingOn;

			for (std::pair<const std::string, QModUtils::QMod *> modPair : *m_DownloadedQMods)
			{
				bool isDependency = false;
				for (Dependency dependency : *modPair.second->m_Dependencies)
				{
					if (dependency.id == m_Id)
					{
						isDependency = true;
						break;
					}
				}

				if (isDependency && (!onlyInstalledMods || modPair.second->IsInstalled()))
				{
					dependingOn.push_back(modPair.second);
				}
			}

			return dependingOn;
		}

		static void DeleteTempDir()
		{
			std::system("rm -f -r \"/sdcard/BMBFData/Mods/Temp/\"");
		}

		// Used for std::map
		bool operator<(const QMod& rhs) const
		{
			std::string lhsStr = m_Id;
			std::string rhsStr = rhs.m_Id;

			std::transform(lhsStr.begin(), lhsStr.end(), lhsStr.begin(), [](unsigned char c){ return std::tolower(c); });
			std::transform(rhsStr.begin(), rhsStr.end(), rhsStr.begin(), [](unsigned char c){ return std::tolower(c); });

			return lhsStr < rhsStr;
		}
	private:
		inline static std::mutex m_InstallLock;
		inline static std::mutex m_BmbfConfigLock;
		inline static std::string m_AppPackageId = "";

		inline static std::unordered_map<std::string, QMod *> *m_DownloadedQMods = new std::unordered_map<std::string, QMod *>();
		inline static std::unordered_map<std::string, QMod *> *m_CoreMods = new std::unordered_map<std::string, QMod *>();

		void GetBMBFData(bool verbos = true)
		{
			if (strcmp(m_PackageId.c_str(), "com.beatgames.beatsaber"))
			{
				if (verbos)
					getLogger().info("Failed to get BMBF Data, QMod isn't for Beat Saber! (PackageId: %s)", m_PackageId.c_str());
				return;
			}

			// Read the config.json file

			std::ifstream configFile("/sdcard/BMBFData/config.json");
			ASSERT(configFile.good(), GetFileName(m_Path), verbos);

			std::stringstream configJson;
			configJson << configFile.rdbuf();

			rapidjson::Document document;

			ASSERT(!document.Parse(configJson.str().c_str()).HasParseError(), GetFileName(m_Path), verbos);
			ASSERT(document.HasMember("Mods") && document["Mods"].IsArray(), GetFileName(m_Path), verbos);

			const auto &mods = document["Mods"].GetArray();

			bool foundMod = false;
			for (rapidjson::SizeType i = 0; i < mods.Size(); i++)
			{
				// Find our mod id, then read the data

				auto &mod = mods[i];
				std::string id = GET_STRING("Id", mod);

				if (id != m_Id)
					continue;
				foundMod = true;

				m_CoverImageFilename = GET_STRING("CoverImageFilename", mod);
				m_Installed = GET_BOOL("Installed", mod);
				m_Uninstallable = GET_BOOL("Uninstallable", mod);
			}

			// Couldnt Find existing BMBF Data, So just set default values;
			if (!foundMod)
			{
				m_CoverImageFilename = "";
				m_Installed = false;
				m_Uninstallable = true;
			}
		}

		void ExtractQMod()
		{
			std::string tmpDir = GetTempDir(m_Path);
			std::string modsExtractionPath = tmpDir + "Mods/";
			std::string libsExtractionPath = tmpDir + "Libs/";
			std::string fileCopiesExtractionPath = tmpDir + "FileCopies/";

			// Create dirs
			std::system(string_format("mkdir -p \"%s\"", modsExtractionPath.c_str()).c_str());
			std::system(string_format("mkdir -p \"%s\"", libsExtractionPath.c_str()).c_str());
			std::system(string_format("mkdir -p \"%s\"", fileCopiesExtractionPath.c_str()).c_str());

			// Extract Mods
			for (std::string mod : *m_ModFiles)
			{
				std::system(string_format("unzip \"%s\" \"%s\" -d \"%s\"", m_Path.c_str(), mod.c_str(), modsExtractionPath.c_str()).c_str());
			}

			// Extract Libs
			for (std::string lib : *m_LibraryFiles)
			{
				std::system(string_format("unzip \"%s\" \"%s\" -d \"%s\"", m_Path.c_str(), lib.c_str(), libsExtractionPath.c_str()).c_str());
			}

			// Extract File Copies
			for (FileCopy fileCopy : *m_FileCopies)
			{
				std::system(string_format("unzip \"%s\" \"%s\" -d \"%s\"", m_Path.c_str(), fileCopy.name.c_str(), fileCopiesExtractionPath.c_str()).c_str());
			}
		}

		bool PrepareDependency(Dependency dependency, std::vector<std::string> *installedInBranch)
		{
			getLogger().info("Preparing dependency of %s version %s", dependency.id.c_str(), dependency.version.c_str());

			// Try to see if there's a recurssive dependency
			auto it = find(installedInBranch->begin(), installedInBranch->end(), dependency.id);
			if (it != installedInBranch->end())
			{
				// Log the recursive error
				int existingIndex = it - installedInBranch->begin();
				std::string errorMsg = "";

				for (std::string mod : *installedInBranch)
				{
					errorMsg += string_format("\"%s\" -> ", mod.c_str());
				}
				errorMsg += dependency.id;

				getLogger().error("Recursive dependency detected: %s", errorMsg.c_str());
				return false;
			}

			QMod *existing = nullptr;
			for (std::pair<std::string, QMod *> modPair : *m_DownloadedQMods)
			{
				if (modPair.first == dependency.id)
				{
					existing = modPair.second;
				}
			}

			if (existing != nullptr)
			{
				if (semver::satisfies(existing->m_Version, dependency.version))
				{
					getLogger().info("Dependency is already downloaded and fits the version range \"%s\"", dependency.version.c_str());

					if (!existing->IsInstalled())
					{
						getLogger().info("Installing Dependency...");

						existing->Install(true, installedInBranch);
					}

					return true;
				}

				if (dependency.downloadIfMissing == "")
				{
					getLogger().error("Dependency with ID \"%s\" is already installed but with an incorrect version (\"%s\" does not intersect \"%s\"). Upgrading was not possible as there was no download link provided", dependency.id.c_str(), existing->m_Version.c_str(), dependency.version.c_str());
					return false;
				}
				else
				{
					getLogger().warning("Dependency with ID \"%s\" is already installed but with an incorrect version (\"%s\" does not intersect \"%s\"). Attempting to upgrade now...", dependency.id.c_str(), existing->m_Version.c_str(), dependency.version.c_str());
				}
			}
			else if (dependency.downloadIfMissing == "")
			{
				getLogger().error("Dependency \"%s\" is not installed, and the mod depending on it does not specify a download path if missing", dependency.id.c_str());
				return false;
			}

			// If we didnt return, then the correct dependency version isnt installed and we have a url, so we attempt to download it now

			QMod *downloadedDependency = nullptr;
			std::string downloadFileLoc = string_format("/sdcard/BMBFData/Mods/Temp/Downloads/%s", dependency.id.c_str());

			// Putting cleanup function in lambda cus its messy and i dont wanna copy it everywhere
			auto CleanupFunction = [&]()
			{ CleanupTempDir(string_format("Downloads/%s", dependency.id.c_str()).c_str(), true); };

			if (!WebUtils::DownloadFile(dependency.id, dependency.downloadIfMissing, downloadFileLoc))
			{
				CleanupFunction();
				return false;
			}

			downloadedDependency = new QMod(downloadFileLoc);

			if (downloadedDependency == nullptr)
			{
				getLogger().error("Failed to parse QMod for dependency \"%s\"", dependency.id.c_str());

				CleanupFunction();
				return false;
			}

			// Sanity checks that the download link actually pointed to the right mod
			if (dependency.id != downloadedDependency->m_Id)
			{
				getLogger().error("Downloaded dependency had Id \"%s\", whereas the dependency stated ID \"%s\"", downloadedDependency->m_Id.c_str(), dependency.id.c_str());

				CleanupFunction();
				return false;
			}

			if (!semver::satisfies(downloadedDependency->m_Version, dependency.version))
			{
				getLogger().error("Downloaded dependency \"%s\" v%s was not within the version range stated in the dependency info (%s)", downloadedDependency->m_Id.c_str(), downloadedDependency->m_Version.c_str(), dependency.version.c_str());

				CleanupFunction();
				return false;
			}

			// Everything's looking good, time to install!
			// NOTE: There is no clean up here because the cleanup will occur during the install
			downloadedDependency->Install(true, installedInBranch);

			return true;
		}

		void UpdateBMBFJSONData(auto &mod, auto &allocator)
		{
			ADD_STRING_MEMBER("Id", m_Id, mod, allocator);
			ADD_STRING_MEMBER("Path", m_Path, mod, allocator);
			ADD_MEMBER("Installed", m_Installed, mod, allocator);
			ADD_MEMBER("TogglingOnSync", false, mod, allocator);
			ADD_MEMBER("RemovingOnSync", false, mod, allocator);
			ADD_STRING_MEMBER("Version", m_Version, mod, allocator);
			ADD_MEMBER("Uninstallable", m_Uninstallable, mod, allocator);
			ADD_STRING_MEMBER("CoverImageFilename", m_CoverImageFilename, mod, allocator);
			ADD_STRING_MEMBER("TargetBeatsaberVersion", m_PackageVersion, mod, allocator);
			ADD_STRING_MEMBER("Author", m_Author, mod, allocator);
			ADD_STRING_MEMBER("Porter", m_Porter, mod, allocator);
			ADD_STRING_MEMBER("Name", m_Name, mod, allocator);
			ADD_STRING_MEMBER("Description", m_Description, mod, allocator);
		}

		void RemoveBMBFData(bool verbos = true)
		{
			// Prevents multiple threads writing to the file at the same time
			std::unique_lock<std::mutex> guard(m_BmbfConfigLock);

			if (verbos)
				getLogger().info("Removing BMBF Info for \"%s\"", m_Id.c_str());

			// Read the config.json file
			std::ifstream configFile("/sdcard/BMBFData/config.json");
			ASSERT(configFile.good(), GetFileName(m_Path), verbos);

			std::stringstream configJson;
			configJson << configFile.rdbuf();

			rapidjson::Document document;

			ASSERT(!document.Parse(configJson.str().c_str()).HasParseError(), GetFileName(m_Path), verbos);
			ASSERT(document.HasMember("Mods") && document["Mods"].IsArray(), GetFileName(m_Path), verbos);

			const auto &mods = document["Mods"].GetArray();

			// Try Find Our Mod in the BMBF Data
			for (int i = 0; i < (int)mods.Size(); i++)
			{
				// Find our mod id, then read the data

				std::string id = GET_STRING("Id", mods[i]);

				if (id != m_Id)
					continue;

				if (verbos)
					getLogger().info("Found BMBF Data for \"%s\", Removing It...", m_Id.c_str());

				mods.Erase(mods.Begin() + i);
				break;
			}

			if (verbos)
				getLogger().info("Removed BMBF Data for \"%s\"! Saving...", m_Id.c_str());

			// Save To Buffer

			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

			document.Accept(writer);

			// Write To File

			std::ofstream out("/sdcard/BMBFData/config.json");
			out << buffer.GetString();
			out.close();

			if (verbos)
				getLogger().info("Saved BMBF Data for \"%s\"!", m_Id.c_str());
		}

		static void CleanUnusedLibraries(bool onlyDisable, bool forceUninstall = false)
		{
			bool actionPerformed = true;
			while (actionPerformed) // Keep attempting to remove libraries until none get removed this iteration
			{
				actionPerformed = false;

				for (std::pair<const std::string, QModUtils::QMod *> modPair : *m_DownloadedQMods)
				{
					QMod *mod = modPair.second;

					if (!mod->IsLibrary())
						continue;
					if (!forceUninstall && !mod->Uninstallable())
						continue;
					if (mod->FindModsDependingOn(onlyDisable).size() != 0)
						continue;

					if (mod->IsInstalled())
					{
						getLogger().info("\"%s\" is unused, %s", mod->Id().c_str(), onlyDisable ? "uninstalling" : "deleting");
						actionPerformed = true;

						mod->Uninstall(onlyDisable, true);
					}
				}
			}
		}

		void CleanDependentMods(bool onlyDisable, bool forceUninstall = false)
		{
			for (QMod *mod : FindModsDependingOn(onlyDisable))
			{
				if (!forceUninstall && !mod->Uninstallable())
					continue;

				getLogger().info("\"%s\" depends on \"%s\", %s", mod->Id().c_str(), m_Id.c_str(), onlyDisable ? "uninstalling" : "deleting");
				mod->Uninstall(onlyDisable, true);
			}
		}

		static void Cache_AppPackageId()
		{
			if (m_AppPackageId == "")
			{
				JNIEnv *env = JNIUtils::GetJNIEnv();
				m_AppPackageId = JNIUtils::ToString(JNIUtils::GetPackageName(env), env);
			}
		}

		const static std::string GetTempDir(std::string path)
		{
			return string_format("/sdcard/BMBFData/Mods/Temp/%s/", GetFileName(path).c_str());
		}

		const static void CleanupTempDir(std::string name, bool isFile = false)
		{
			if (name != "")
			{
				if (isFile)
				{
					std::system(string_format("rm -f \"/sdcard/BMBFData/Mods/Temp/%s\"", name.c_str()).c_str()); // Remove The file
				}
				else
				{
					std::system(string_format("rm -f -r \"/sdcard/BMBFData/Mods/Temp/%s/\"", name.c_str()).c_str()); // Remove This QMod's Temp Dir
				}
			}

			std::system("rmdir \"/sdcard/BMBFData/Mods/Temp/Downloads/\""); // Attempt To Remove the downloads Temp Dir, but only if it's empty
			std::system("rmdir \"/sdcard/BMBFData/Mods/Temp/\"");			// Attempt To Remove the entire Temp Dir, but only if it's empty
		}

		static const std::string GetFileName(std::string path, bool removeFileExtension = true, bool returnTrueName = false)
		{
			// Get the file name

			std::string fileName = path.substr(path.find_last_of("/\\") + 1);

			if (returnTrueName)
				return fileName; // This will just return the actual file name, with no modifications

			// Replace all spaces with underscores, as bmbf doesnt like qmods with spaces

			std::replace(fileName.begin(), fileName.end(), ' ', '_');

			// Remove the File Extension (if there is one)

			size_t lastindex = fileName.find_last_of(".");
			if (lastindex != std::string::npos)
				fileName = fileName.substr(0, lastindex);

			if (removeFileExtension)
				return fileName;

			// Add the .qmod extension

			fileName += ".qmod";

			return fileName;
		}

		std::string m_Name;
		std::string m_Id;
		std::string m_Description;
		std::string m_Author;
		std::string m_Porter;
		std::string m_Version;
		std::string m_CoverImage;

		std::string m_PackageId;
		std::string m_PackageVersion;

		std::string m_Path;

		std::vector<std::string> *m_ModFiles;
		std::vector<std::string> *m_LibraryFiles;
		std::vector<Dependency> *m_Dependencies;
		std::vector<FileCopy> *m_FileCopies;

		bool m_Valid;
		bool m_IsLibrary;

		// BMBF Stuff

		std::string m_CoverImageFilename;

		bool m_Installed;
		bool m_Uninstallable;
	};
}