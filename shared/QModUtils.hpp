#pragma once

#include "cpp-semver/shared/cpp-semver.hpp"

#include "qmod-utils/shared/Types/QMod.hpp"
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

	/**
	 * @brief Get all the files that are contained in a specified directory
	 * 
	 * @param dirPath The Path to get the contents of
	 * @return A list of all the files in the directory
	 */
	inline std::list<std::string> GetDirContents(std::string dirPath);

	/**
	 * @brief Get all of the QMods that are currently downloaded
	 * 
	 * @return A List of all downloaded QMods 
	 */
	inline std::unordered_map<std::string, QModUtils::QMod *>* GetDownloadedQMods();

	/**
	 * @brief Get all of the QMods that are currently installed
	 * 
	 * @return A List of all installed QMods 
	 */
	inline std::unordered_map<std::string, QModUtils::QMod *>* GetInstalledQMods();

	/**
	 * @brief Get all of the QMods that are currently uninstalled
	 * 
	 * @return A List of all uninstalled QMods 
	 */
	inline std::unordered_map<std::string, QModUtils::QMod *>* GetUninstalledQMods();

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
	 * @param name The name of the mod to test for an error
	 * @return Returns the error if there was one, else returns null
	 */
	inline std::optional<std::string> GetModError(std::string name);

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
	 * @brief Restarts The Current Game
	 */
	inline void RestartGame();

	// Private shit dont use >:(

	inline void Init();

	inline void CollectGameVersion();
	inline void CollectPackageName();
	inline void CollectCoreMods();
	inline void CollectDownloadedMods();

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

	std::unordered_map<std::string, QModUtils::QMod *>* GetDownloadedQMods() {
		Init();

		return QMod::DownloadedQMods;
	}

	std::unordered_map<std::string, QModUtils::QMod *>* GetInstalledQMods() {
		Init();

		std::unordered_map<std::string, QModUtils::QMod *>* installedQMods = new std::unordered_map<std::string, QModUtils::QMod *>();
		for (std::pair<std::string, QMod*> qmodPair : *QMod::DownloadedQMods) {
			if (qmodPair.second->IsInstalled()) installedQMods->insert(qmodPair);
		}

		return installedQMods;
	}

	std::unordered_map<std::string, QModUtils::QMod *>* GetUninstalledQMods() {
		Init();

		std::unordered_map<std::string, QModUtils::QMod *>* uninstalledQMods = new std::unordered_map<std::string, QModUtils::QMod *>();
		for (std::pair<std::string, QMod*> qmodPair : *QMod::DownloadedQMods) {
			if (!qmodPair.second->IsInstalled()) uninstalledQMods->insert(qmodPair);
		}

		return uninstalledQMods;
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
		for (std::pair<std::string, const Mod> modPair : Modloader::getMods()) {
			if (modPair.second.name == fileName) return true;
		}

		return false;
	}

	// Thanks for Laurie for the original code snippet: 
	std::optional<std::string> GetModError(std::string fileName) {
		std::string filePath = Modloader::getDestinationPath() + fileName;
		
		dlerror(); // Clear Existing Errors
		dlopen(filePath.c_str(), RTLD_LOCAL | RTLD_NOW);

		char* error = dlerror();
		return error ? std::optional(std::string(error).substr(15)) : std::nullopt;
	}

	std::string GetGameVersion() {
		return m_GameVersion;
	}

	std::string GetPackageName() {
		return m_PackageName;
	}

	void RestartGame() {
		Init();
		getLogger().info("-- STARTING RESTART --");

		JNIEnv* env = JNIUtils::GetJNIEnv();

		jstring packageName = JNIUtils::GetPackageName(env);

		// Get Activity
		jobject appContext = JNIUtils::GetAppContext(env);

		// Get Package Manager
		CALL_JOBJECT_METHOD(env, packageManager, appContext, "getPackageManager", "()Landroid/content/pm/PackageManager;");

		// Get Intent
		CALL_JOBJECT_METHOD(env, intent, packageManager, "getLaunchIntentForPackage", "(Ljava/lang/String;)Landroid/content/Intent;", packageName);

		// Set Intent Flags
		CALL_JOBJECT_METHOD(env, setFlagsSuccess, intent, "setFlags", "(I)Landroid/content/Intent;", 536870912);

		// Get Component Name
		CALL_JOBJECT_METHOD(env, componentName, intent, "getComponent", "()Landroid/content/ComponentName;");

		// Create Restart Intent
		GET_JCLASS(env, intentClass, "android/content/Intent");
		CALL_STATIC_JOBJECT_METHOD(env, restartIntent, intentClass, "makeRestartActivityTask", "(Landroid/content/ComponentName;)Landroid/content/Intent;", componentName);

		// Restart Game
		CALL_VOID_METHOD(env, appContext, "startActivity", "(Landroid/content/Intent;)V", restartIntent);

		GET_JCLASS(env, processClass, "android/os/Process");

		CALL_STATIC_JINT_METHOD(env, pid, processClass, "myPid", "()I");
		CALL_STATIC_VOID_METHOD(env, processClass, "killProcess", "(I)V", pid);
	}

	void CollectCoreMods() {
		std::string coreModsData = WebUtils::GetData("https://raw.githubusercontent.com/BMBF/resources/master/com.beatgames.beatsaber/core-mods.json");
		bool useLocalCopy = false;

		if (coreModsData == "") {
			getLogger().warning("Failed to get the list of core mods from questboard, reverting to local copy...");
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

		getLogger().info("Collecting Core Mods...");

		if (coreModsDoc.HasMember(m_GameVersion)) {
			const rapidjson::Value& versionInfo = coreModsDoc[m_GameVersion];
			const rapidjson::Value& coreModsList = versionInfo["mods"];

			bool shouldRestart = false;

			for (rapidjson::SizeType i = 0; i < coreModsList.Size(); i++) { // rapidjson uses SizeType instead of size_t.
				const rapidjson::Value& coreModInfo = coreModsList[i];

				std::string id = coreModInfo["id"].GetString();

				QMod* coreQMod = nullptr;
				for (std::pair<std::string, QMod*> qmodPair : *QMod::DownloadedQMods) {
					if (qmodPair.first == id) {
						coreQMod = qmodPair.second;
						break;
					}
				}

				if (coreQMod != nullptr) {
					std::string latestVersion = coreModInfo["version"].GetString();

					if (semver::satisfies(coreQMod->Version(), "<" + latestVersion)) {
						getLogger().warning("Warning! Core mod \"%s\" is outdated! (CurrentVer: \"%s\", LatestVer: \"%s\") Attempting to download the latest version now...", id.c_str(), coreQMod->Version().c_str(), latestVersion.c_str());
						shouldRestart = true;

						coreQMod->Uninstall(false, true);

						QMod::InstallFromUrl(coreModInfo["filename"].GetString(), coreModInfo["downloadLink"].GetString(), true);
						std::optional<QMod*> coreQModOpt = QMod::GetDownloadedQMod(id);

						if (coreQModOpt.has_value()) {
							coreQMod = coreQModOpt.value();

							coreQMod->SetUninstallable(false);
							coreQMod->UpdateBMBFData();

							getLogger().info("Updated Core Mod \"%s\"", coreQMod->Id().c_str());
							QMod::CoreQMods->emplace(coreQMod->Id(), coreQMod);
						} else {
							coreQMod = nullptr;
						}

					} else {
						getLogger().info("Found Core Mod \"%s\"", coreQMod->Id().c_str());
						QMod::CoreQMods->emplace(coreQMod->Id(), coreQMod);
					}
				} else {
					getLogger().warning("Warning! Core mod \"%s\" not found! Attempting to download it now...", id.c_str());
					shouldRestart = true;

					QMod::InstallFromUrl(coreModInfo["filename"].GetString(), coreModInfo["downloadLink"].GetString(), true);
					std::optional<QMod*> coreQModOpt = QMod::GetDownloadedQMod(id);

					if (coreQModOpt.has_value()) {
						coreQMod = coreQModOpt.value();

						coreQMod->SetUninstallable(false);
						coreQMod->UpdateBMBFData();

						getLogger().info("Downloaded Core Mod \"%s\"", coreQMod->Id().c_str());
						QMod::CoreQMods->emplace(coreQMod->Id(), coreQMod);
					} else {
						coreQMod = nullptr;
					}
				}	
			}

			// Installed 1 or more coremods, so the game should restart
			if (shouldRestart) RestartGame();
		} else {
			getLogger().error("ERROR! No Core Mods Found For This Version!");
		}

		getLogger().info("Finished Collecting Core Mods!");
	}

	void CollectPackageName() {
		getLogger().info("Collecting Package Name...");
		JNIEnv* env = JNIUtils::GetJNIEnv();

		jstring packageName = JNIUtils::GetPackageName(env);
		m_PackageName = JNIUtils::ToString(env, packageName);

		getLogger().info("Got Package Name \"%s\"!", m_PackageName.c_str());
	}

	void CollectGameVersion() {
		getLogger().info("Collecting Game Version...");
		JNIEnv* env = JNIUtils::GetJNIEnv();

		jstring gameVersion = JNIUtils::GetGameVersion(env);
		m_GameVersion = JNIUtils::ToString(env, gameVersion);

		getLogger().info("Got Game Version \"%s\"!", m_GameVersion.c_str());
	}

	void CollectDownloadedMods() {
		getLogger().info("Collecting Downloaded QMods...");

		QMod::DownloadedQMods->clear();
		std::list<std::string> fileNames = GetDirContents(m_QModPath);

		for (std::string file : fileNames) {
			std::string filePath = m_QModPath + file;
			QMod* qmod = new QMod(filePath, false, false);

			if (qmod->Valid()) {
				getLogger().info("Found QMod File \"%s\"", file.c_str());
				QMod::DownloadedQMods->insert({qmod->Id(), qmod});
			}
		}

		QMod::DeleteTempDir();

		getLogger().info("Finished Collecting Downloaded QMods!");
	}

	void Init() {
		if (m_HasInitialized) return;
		m_HasInitialized = true;
		
		m_QModPath = "/sdcard/BMBFData/Mods/";

		CollectPackageName();
		CollectGameVersion();

		CollectDownloadedMods();
		CollectCoreMods();
	}
};