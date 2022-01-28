#pragma once

#include "cpp-semver/shared/cpp-semver.hpp"

#include "qmod-utils/shared/Types/QMod.hpp"
#include "qmod-utils/shared/Types/CoreModInfo.hpp"
#include "qmod-utils/shared/WebUtils.hpp"

#include "modloader/shared/modloader.hpp"

#include "beatsaber-hook/shared/rapidjson/include/rapidjson/rapidjson.h"

#include "jni-utils/shared/JNIUtils.hpp"

#include <list>
#include <dirent.h>
#include <jni.h>
#include <unordered_map>
#include <sstream>
#include <fstream>

Logger& getLogger();

namespace QModUtils {
	// Declerations
 
	inline bool m_HasInitialized;

	inline const char* m_QModPath;
 
	inline std::string m_GameVersion;
	inline std::string m_PackageName;

	inline std::vector<std::string>* m_LoadedLibs;
	inline std::unordered_map<std::string, CoreModInfo>* m_MissingCoreMods;
	inline std::unordered_map<QMod*, std::string>* m_ErrorMessages;
	inline std::unordered_map<std::string, QMod*>* m_InstalledQMods;
	inline std::unordered_map<std::string, QMod*>* m_UninstalledQMods;
	inline std::unordered_map<std::string, QMod*>* m_FailedToLoadMods;

	/**
	 * @brief Get all the files that are contained in a specified directory
	 * 
	 * @param dirPath The Path to get the contents of
	 * @return A list of all the files in the directory
	 */
	inline std::list<std::string> GetDirContents(std::string dirPath);

	/**
	 * @brief Get all of the QMods that are currently installed
	 * 
	 * @return A List of all installed QMods 
	 */
	inline std::unordered_map<std::string, QModUtils::QMod *> GetInstalledMods();

	/**
	 * @brief Get all of the QMods that are currently uninstalled
	 * 
	 * @return A List of all uninstalled QMods 
	 */
	inline std::unordered_map<std::string, QModUtils::QMod *> GetUninstalledMods();

	/**
	 * @brief Get all of the QMods that failed to load
	 * 
	 * @return A List of all the QMods that failed to load
	 */
	inline std::unordered_map<std::string, QModUtils::QMod *> GetFailedToLoadMods();

	/**
	 * @brief Sets the activity of a specific QMod
	 * 
	 * @param qmod The QMod to enable or disable
	 * @param active Whether to enable or disable the QMod
	 */
	inline void SetModActive(QMod* qmod, bool active);

	/**
	 * @brief Sets the activity of a list of QMods
	 * 
	 * @param qmods The list of QMods to enable or disable
	 * @param actives Whether to enable or disable each QMod. This should be the same size as the QMods list
	 * @param onSetActiveStart This function is ran before setting the activity of each QMod
	 */
	inline void SetModsActive(std::vector<QMod*>* qmods, std::vector<bool> actives, std::function<void(QMod*, bool)> onSetActiveStart);

	/**
	 * @brief Toggles the activity of a specific QMod to either enabled or diabled
	 * 
	 * @param qmod The QMod to toggle
	 */
	inline void ToggleMod(QMod* qmod);

	/**
	 * @brief Toggles a list of QMods on or off
	 * 
	 * @param qmods The list of QMods to be toggled
	 * @param onToggleStart This function is ran before toggling each mod
	 */
	inline void ToggleMods(std::vector<QMod*>* qmods, std::function<void(QMod*, bool)> onToggleStart);

	/**
	 * @brief Reloads a specific QMod
	 * 
	 * @param qmod The QMod to reload
	 */
	inline void ReloadMod(QMod* qmod);

	/**
	 * @brief Reloads a list of QMods
	 * 
	 * @param qmods The list of QMods to be reloaded
	 * @param onReloadStart This function is ran before reloading each mod
	 */
	inline void ReloadMods(std::vector<QMod*>* qmods, std::function<void(QMod*)> onReloadStart);

	/**
	 * @brief Returns true if a lib file (.so) is loaded. A lib is loaded if it has been dlopened by Modloader
	 * 
	 * @param fileName The filename of the lib to test
	 */
	inline bool IsModLibLoaded(std::string fileName);

	/**
	 * @brief Get's the error for a mod
	 * @details The mod is dlopened, and then then the error is fetched using dlerror
	 * 
	 * @param qmod The mod to get the error of
	 * @return Returns the error if there was one, else returns null
	 */
	inline std::optional<std::string> GetModError(QMod* qmod);

	/**
	 * @brief Check if a mod has an error
	 * 
	 * @param qmod The mod to check if theres an error
	 * @return Returns ture if the mod has an error
	 */
	inline bool ModHasError(QMod* qmod);

	/**
	 * @brief Get the version of the app that's currently running
	 * 
	 * @return A string that contains the game version
	 */
	inline std::string GetGameVersion();

	/**
	 * @brief Gets the package name for the current app, for example "com.beatgames.beatsaber"
	 * 
	 * @return The current app's package name
	 */
	inline std::string GetPackageName();

	/**
	 * @brief Returns some info about any mising / outdated core mods
	 * 
	 * @return A map containing the id as the key, and a "CoreModInfo" as the value. The value contains info about the latest core mod from the "core-mods.json"
	 */
	inline std::unordered_map<std::string, CoreModInfo> GetMissingCoreMods();

	/**
	 * @brief Gets the list of mod errors
	 * @details The mod is dlopened, and then then the error is fetched using dlerror
	 * 
	 * @return Returns the list of mod errors
	 */
	inline std::unordered_map<QMod*, std::string> GetModErrors();

	/**
	 * @brief Attempts to download and install missing core mods
	 * 
	 * @param restart Weather to restart the game after install or not
	 */
	inline void InstallMissingCoreMods(bool restart = true);

	/**
	 * @brief Should be called on Load
	 */
	inline void Init();

	// Private shit dont use >:(

	inline void CacheGameVersion();
	inline void CachePackageName();

	inline void CacheLoadedLibs();
	inline void CacheErrorMessages();

	inline void CacheInstalledMods();
	inline void CacheUninstalledMods();
	inline void CacheFailedToLoadMods();
	inline void CacheDownloadedMods();
	inline void CacheCoreMods();

	// Definitions

	std::list<std::string> GetDirContents(std::string dirPath) {
		DIR* dir = opendir(dirPath.c_str());
		dirent* dp;
		std::list<std::string> files; 

		if (dir == nullptr) return files;

		while ((dp = readdir(dir)) != NULL) {
			if (dp->d_type != DT_DIR) {
				files.emplace_front(std::string(dp->d_name));
			}
		}

		return files;
	}

	std::unordered_map<std::string, QModUtils::QMod *> GetInstalledMods() {
		Init();

		return *m_InstalledQMods;
	}

	std::unordered_map<std::string, QModUtils::QMod *> GetUninstalledMods() {
		Init();

		return *m_UninstalledQMods;
	}

	std::unordered_map<std::string, QModUtils::QMod *> GetFailedToLoadMods() {
		Init();
		
		return *m_FailedToLoadMods;
	}

	void SetModActive(QMod* qmod, bool active) {
		getLogger().info("%s QMod \"%s\"", active ? "Enabling" : "Disabling", qmod->Name().c_str());

		if (active) qmod->Install();
		else qmod->Uninstall();
	}

	inline bool qmodSortFunction(QMod* a, QMod* b) { return a->IsInstalled(); }
	inline bool boolSortFunction(bool a, bool b) { return !a; }

	void SetModsActive(std::vector<QMod*>* qmods, std::vector<bool> actives, std::function<void(QMod*, bool)> onSetActiveStart) {
		if (qmods->size() != actives.size()) {
			getLogger().error("Failed to set the activity of a list of QMods, Vector size mismatch!");
			return;
		}

		std::sort(qmods->begin(), qmods->end(), qmodSortFunction);
		std::sort(actives.begin(), actives.end(), boolSortFunction);

		for (int i = 0; i < qmods->size(); i++) {
			if (onSetActiveStart) onSetActiveStart(qmods->at(i), actives[i]);

			std::optional<std::thread> t;

			if (actives[i]) t = qmods->at(i)->InstallAsync();
			else t = qmods->at(i)->UninstallAsync();

			if (t.has_value()) {
				t.value().join();
			} else {
				getLogger().warning("Skipping QMod \"%s\", thread  was invalid!", qmods->at(i)->Id().c_str());
			}
		}
	}

	void ToggleMod(QMod* qmod) {
		SetModActive(qmod, !qmod->IsInstalled());
	}

	void ToggleMods(std::vector<QMod*>* qmods, std::function<void(QMod*, bool)> onToggleStart) {
		getLogger().info("Toggling a list of QMods");

		std::vector<bool> actives;

		for (QMod* qmod : *qmods) {
			actives.emplace_back(!qmod->IsInstalled());
		}

		SetModsActive(qmods, actives, onToggleStart);
	}

	void ReloadMod(QMod* qmod) {
		getLogger().info("Reloading QMod \"%s\"", qmod->Id().c_str());

		qmod->Uninstall();
		qmod->Install();
	}

	void ReloadMods(std::vector<QMod*>* qmods, std::function<void(QMod*)> onReloadStart) {
		getLogger().info("Reloading A list of QMods");

		for (int i = 0; i < qmods->size(); i++) {
			if (onReloadStart) onReloadStart(qmods->at(i));

			std::optional<std::thread> tUninstall = qmods->at(i)->UninstallAsync();

			if (tUninstall.has_value()) {
				tUninstall.value().join();
			} else {
				getLogger().warning("Skipping QMod \"%s\", Uninstall thread  was invalid!", qmods->at(i)->Id().c_str());
				continue;
			}

			std::optional<std::thread> tInstall = qmods->at(i)->InstallAsync();

			if (tInstall.has_value()) {
				tInstall.value().join();
			} else {
				getLogger().warning("Skipping QMod \"%s\", Install thread  was invalid!", qmods->at(i)->Id().c_str());
				continue;
			}
		}
	}

	bool IsModLibLoaded(std::string fileName) {
		Init();

		return std::count(m_LoadedLibs->begin(), m_LoadedLibs->end(), fileName) != 0;
	}

	std::optional<std::string> GetModError(QMod* qmod) {
		Init();

		if (!m_ErrorMessages->contains(qmod)) return std::nullopt;
		
		return m_ErrorMessages->at(qmod);
	}

	bool ModHasError(QMod* qmod) {
		return m_ErrorMessages->contains(qmod);
	}

	std::string GetGameVersion() {
		return m_GameVersion;
	}

	std::string GetPackageName() {
		return m_PackageName;
	}

	std::unordered_map<std::string, CoreModInfo> GetMissingCoreMods() {
		return *m_MissingCoreMods;
	}

	std::unordered_map<QMod*, std::string> GetModErrors() {
		return *m_ErrorMessages;
	}

	void InstallMissingCoreMods(bool restart) {
		Init();
		
		getLogger().info("Installing missing/outdated core mods...");
		int installCount = 0;

		for (auto modInfo : *m_MissingCoreMods) {
			std::string id = modInfo.first;
			CoreModInfo coreModInfo = modInfo.second;

			// Sanity Check
			if (coreModInfo.filename == "" || coreModInfo.downloadLink == "")
			{
				getLogger().warning("Failed to download Core Mod \"%s\", invalid data!", id.c_str());
				continue;
			}

			// Attempts to get the core mod by id (in case it's just outdated)
			std::optional<QMod*> coreModOpt = QMod::GetDownloadedQMod(id);
			QMod* coreMod = nullptr;

			if (coreModOpt.has_value()) {
				coreMod = coreModOpt.value();
				coreMod->Uninstall(false, true);
			}

			QMod::InstallFromUrl(coreModInfo.filename, coreModInfo.downloadLink, true);
			coreModOpt = QMod::GetDownloadedQMod(id);

			if (coreModOpt.has_value()) {
				coreMod = coreModOpt.value();
				coreMod->SetUninstallable(false);
				coreMod->UpdateBMBFData();

				getLogger().info("Installed Core Mod \"%s\"", coreMod->Id().c_str());
				QMod::GetCoreMods()->emplace(coreMod->Id(), coreMod);

				installCount++;
			}
		}

		getLogger().info("Installed %i missing/outaded Core Mods!", installCount);

		if (restart && installCount != 0) JNIUtils::RestartApp();
	}

	void CacheLoadedLibs() {
		for (auto modPair : Modloader::getMods()) {
			m_LoadedLibs->push_back(modPair.second.name);
		}
	}

	void CacheCoreMods() {
		getLogger().info("Downloading the latest list of core mods...");
		
		std::string coreModsData = WebUtils::GetData("https://raw.githubusercontent.com/BMBF/resources/master/com.beatgames.beatsaber/core-mods.json");
		bool useLocalCopy = false;

		if (coreModsData == "") {
			getLogger().warning("Failed to get the list of core mods from online, reverting to local copy...");
			useLocalCopy = true;
		}

		rapidjson::Document coreModsDoc;

		if (!useLocalCopy && coreModsDoc.Parse(coreModsData.c_str()).HasParseError()) {
			getLogger().warning("Failed to parse the downloaded list of core mods, reverting to local copy...");
			useLocalCopy = true;
		}

		if (useLocalCopy) {
			// Revert to local copy of core mods

			std::ifstream coreModsFile("/sdcard/BMBFData/core-mods.json");
			std::stringstream coreModsSS;
			coreModsSS << coreModsFile.rdbuf();

			if (coreModsDoc.Parse(coreModsSS.str().c_str()).HasParseError()) {
				getLogger().error("Failed to parse local list of core mods!");
				return;
			}
		} else {
			// Downloaded File looks good, lets save it as a local copy

			std::ofstream localFile = std::ofstream("/sdcard/BMBFData/core-mods.json");
			localFile << coreModsData;
			localFile.close();
		}

		getLogger().info("Caching Core Mods...");

		if (coreModsDoc.HasMember(m_GameVersion) && coreModsDoc[m_GameVersion].IsObject()) {
			auto& versionInfo = coreModsDoc[m_GameVersion];

			if (!versionInfo.HasMember("mods") || !versionInfo["mods"].IsArray()) {
				getLogger().error("Failed to find the list of core mods");
				return;
			}

			auto& coreModsList = versionInfo["mods"];

			for (rapidjson::SizeType i = 0; i < coreModsList.Size(); i++) { // rapidjson uses SizeType instead of size_t.
				auto& coreModInfo = coreModsList[i];

				if (!coreModInfo.HasMember("id") || !coreModInfo["id"].IsString()) {
					getLogger().warning("Error found when reading mod info, skipping...");
					continue;
				}

				std::string id = coreModInfo["id"].GetString();

				QMod* coreMod = nullptr;
				for (std::pair<std::string, QMod*> qmodPair : *QMod::GetDownloadedQMods()) {
					if (qmodPair.first == id) {
						coreMod = qmodPair.second;
						break;
					}
				}

				if (coreMod != nullptr) {
					QMod::GetCoreMods()->emplace(coreMod->Id(), coreMod);
					getLogger().info("Found Core mod \"%s\"!", id.c_str());

					if (!coreModInfo.HasMember("version") || !coreModInfo["version"].IsString()) {
						getLogger().warning("Error found when reading mod info, unable to check for update!");
						continue;
					}
					
					std::string latestVersion = coreModInfo["version"].GetString();

					if (semver::satisfies(coreMod->Version(), "<" + latestVersion)) {
						getLogger().warning("Warning! Core mod \"%s\" is outdated! (CurrentVer: \"%s\", LatestVer: \"%s\")", id.c_str(), coreMod->Version().c_str(), latestVersion.c_str());

						m_MissingCoreMods->emplace(id, coreModInfo);
					}
				} else {
					getLogger().warning("Warning! Core mod \"%s\" not found!", id.c_str());

					m_MissingCoreMods->emplace(id, coreModInfo);
				}
			}
		} else {
			getLogger().error("No Core Mods Found For This Version!");
		}

		getLogger().info("Finished Caching Core Mods!");
	}

	void CachePackageName() {
		getLogger().info("Caching Package Name...");
		JNIEnv* env = JNIUtils::GetJNIEnv();

		jstring packageName = JNIUtils::GetPackageName(env);
		m_PackageName = JNIUtils::ToString(packageName, env);

		getLogger().info("Got Package Name \"%s\"!", m_PackageName.c_str());
	}

	void CacheGameVersion() {
		getLogger().info("Caching Game Version...");
		JNIEnv* env = JNIUtils::GetJNIEnv();

		jstring gameVersion = JNIUtils::GetGameVersion(env);
		m_GameVersion = JNIUtils::ToString(gameVersion, env);

		getLogger().info("Got Game Version \"%s\"!", m_GameVersion.c_str());
	}

	void CacheDownloadedMods() {
		getLogger().info("Caching Downloaded QMods...");

		QMod::GetDownloadedQMods()->clear();
		std::list<std::string> fileNames = GetDirContents(m_QModPath);

		for (std::string file : fileNames) {
			std::string filePath = m_QModPath + file;
			QMod* qmod = new QMod(filePath, false, false);

			if (qmod->Valid()) {
				getLogger().info("Found QMod File \"%s\"", file.c_str());
				QMod::GetDownloadedQMods()->insert({qmod->Id(), qmod});
			}
		}

		QMod::DeleteTempDir();

		getLogger().info("Finished Caching Downloaded QMods!");
	}

	void CacheErrorMessages() {
		getLogger().info("Caching Error Messages...");

		int errorCount = 0;
		for (std::pair<const std::string, QModUtils::QMod *> modPair : *QMod::GetDownloadedQMods()) {
			if (!modPair.second->IsInstalled()) continue;

			for (std::string mod : modPair.second->ModFiles()) {
				// Thanks for Laurie for the original code snippet: https://github.com/Lauriethefish/ModList/blob/main/src/library_utils.cpp#L8-L15
				std::string filePath = Modloader::getDestinationPath() + mod;
				
				dlerror(); // Clear Existing Errors
				dlopen(filePath.c_str(), RTLD_LOCAL | RTLD_NOW);

				char* error = dlerror();

				if (error) {
					m_ErrorMessages->emplace(modPair.second, std::string(error).substr(15));

					getLogger().warning("dlerror when dlopening \"%s\" in mod \"%s\": %s", mod.c_str(), modPair.second->Id().c_str(), error);
					errorCount++;

					break;
				}
			}
		}

		getLogger().info("Finished Caching Error Messages! (Found %i errors)", errorCount);
	}

	void CacheInstalledMods() {
		for (std::pair<std::string, QMod*> qmodPair : *QMod::GetDownloadedQMods()) {
			if (qmodPair.second->IsInstalled()) m_InstalledQMods->insert(qmodPair);
		}
	}

	void CacheUninstalledMods() {
		for (std::pair<std::string, QMod*> qmodPair : *QMod::GetDownloadedQMods()) {
			if (!qmodPair.second->IsInstalled()) m_UninstalledQMods->insert(qmodPair);
		}
	}

	void CacheFailedToLoadMods() {
		for (std::pair<std::string, QMod*> qmodPair : *QMod::GetDownloadedQMods()) {
			if (ModHasError(qmodPair.second)) m_FailedToLoadMods->insert(qmodPair);
		}
	}

	void Init() {
		// ORDER MATTERS! DONT FUCK WITH IT!

		if (m_HasInitialized) return;
		m_HasInitialized = true;
		
		m_QModPath = "/sdcard/BMBFData/Mods/";

		m_LoadedLibs = new std::vector<std::string>();
		m_MissingCoreMods = new std::unordered_map<std::string, CoreModInfo>();
		m_InstalledQMods = new std::unordered_map<std::string, QMod*>();
		m_UninstalledQMods = new std::unordered_map<std::string, QMod*>();
		m_FailedToLoadMods = new std::unordered_map<std::string, QMod*>();
		m_ErrorMessages = new std::unordered_map<QMod*, std::string>();

		CachePackageName();
		CacheGameVersion();

		CacheLoadedLibs();
		CacheDownloadedMods();
		CacheCoreMods();
		CacheErrorMessages();

		CacheInstalledMods();
		CacheUninstalledMods();
		CacheFailedToLoadMods();
	}
};