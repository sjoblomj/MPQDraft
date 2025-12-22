/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

#include "CommandParser.h"
#include "CLI11.hpp"
#include "version.h"
#include "../../core/GameData.h"
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// Custom formatter for better visual spacing in help output

class GroupedFormatter : public CLI::Formatter {
public:
	GroupedFormatter() : Formatter() {
		// Disable the "[Option Group: ...]" label
		label("REQUIRED", "(required)");
	}

	// Override to change how option groups are labeled
	std::string make_option_opts(const CLI::Option* opt) const override {
		// Simplified - just return type name if present
		std::stringstream out;
		std::string type_name = opt->get_type_name();
		if (!type_name.empty() && type_name != "BOOLEAN") {
			out << " " << type_name;
		}
		if (opt->get_required()) {
			out << " (required)";
		}
		return out.str();
	}

	std::string make_group(std::string group, bool is_positional, std::vector<const CLI::Option*> opts) const override {
		std::stringstream out;

		if (opts.empty()) {
			return "";
		}

		// Add visual separator and group header
		out << "\n";  // Blank line before group

		// Show group header with underline
		out << "  " << group << "\n";
		out << "  " << std::string(group.length(), '-') << "\n";

		for (const CLI::Option* opt : opts) {
			out << make_option(opt, is_positional);
		}

		return out.str();
	}

	std::string make_option(const CLI::Option* opt, bool is_positional) const override {
		std::stringstream out;

		// Get short and long option names
		std::string name_str;
		const auto& snames = opt->get_snames();
		const auto& lnames = opt->get_lnames();

		for (size_t i = 0; i < snames.size(); i++) {
			if (!name_str.empty()) name_str += ", ";
			name_str += "-" + snames[i];
		}
		for (size_t i = 0; i < lnames.size(); i++) {
			if (!name_str.empty()) name_str += ", ";
			name_str += "--" + lnames[i];
		}

		// Get description
		std::string desc = opt->get_description();

		// Build type info
		std::string type_info;
		std::string type_name = opt->get_type_name();
		if (!type_name.empty() && type_name != "BOOLEAN") {
			type_info = " " + type_name;
		}

		// Add required marker
		if (opt->get_required()) {
			type_info += " (required)";
		}

		// Format: indented option with description on next line
		out << "      " << name_str << type_info << "\n";
		if (!desc.empty()) {
			out << "          " << desc << "\n";
		}

		return out.str();
	}
};

// Helper for case-insensitive string comparison
static std::string toLower(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

// Static storage for persistent game data (needed for returning pointers)
static std::vector<SupportedGame> s_persistentGames;
static bool s_gamesInitialized = false;

// Find a game component by alias (case-insensitive)
// Returns true if found, with outGame and outComponent pointing to the found items
static bool findGameByAlias(const std::string& alias,
                            const SupportedGame** outGame,
                            const GameComponent** outComponent) {
	// Initialize persistent storage on first call
	if (!s_gamesInitialized) {
		s_persistentGames = getSupportedGames();
		s_gamesInitialized = true;
	}

	std::string lowerAlias = toLower(alias);

	for (const auto& game : s_persistentGames) {
		for (const auto& comp : game.components) {
			for (const auto& compAlias : comp.aliases) {
				if (toLower(compAlias) == lowerAlias) {
					*outGame = &game;
					*outComponent = &comp;
					return true;
				}
			}
		}
	}

	return false;
}

// Helper to build the list of supported games for help text
static std::string buildGameList() {
	std::ostringstream oss;
	oss << "Supported games (use alias with --game):\n";
	auto games = getSupportedGames();
	for (const auto& game : games) {
		for (const auto& comp : game.components) {
			oss << "  - " << game.gameName;
			if (game.components.size() > 1) {
				oss << " / " << comp.componentName;
			}
			// Show aliases
			if (!comp.aliases.empty()) {
				oss << "  [alias: " << comp.aliases[0];
				for (size_t i = 1; i < comp.aliases.size(); i++) {
					oss << ", " << comp.aliases[i];
				}
				oss << "]";
			}
			oss << "\n";
		}
	}
	return oss.str();
}

bool CommandParser::ParseCommandLine(int argc, char** argv)
{
	m_commandType = CommandType::None;
	m_patchCommand = PatchCommand();
	m_sempqCommand = SEMPQCommand();
	m_message.clear();
	m_helpRequested = false;
	m_versionRequested = false;

	// Create main app
	CLI::App app{"MPQDraft - Patch games with custom MPQ archives and plugins"};
	std::string versionStr = std::string("MPQDraft ") + MPQDRAFT_VERSION +
		"\n  By Quantam (Justin Olbrantz)" +
		"\n  Updated by milestone-dev and Ojan (Johan Sjöblom)";
	app.set_version_flag("-v,--version", versionStr);
	app.require_subcommand(0, 1);  // 0 or 1 subcommand

	// =========================================================================
	// Patch subcommand
	// =========================================================================
	auto* patch = app.add_subcommand("patch", "Patch and launch a game executable");

	// Use custom formatter for better visual grouping
	auto patchFormatter = std::make_shared<GroupedFormatter>();
	patch->formatter(patchFormatter);

	patch->add_option("-t,--target", m_patchCommand.target,
		"Target executable to patch and launch")
		->required()
		->check(CLI::ExistingFile)
		->group("Target");

	patch->add_option("-m,--mpq", m_patchCommand.mpqs,
		"MPQ archive(s) to load (can specify multiple)")
		->check(CLI::ExistingFile)
		->group("MPQ and Plugins");

	patch->add_option("-p,--plugin", m_patchCommand.plugins,
		"Plugin file(s) to load (can specify multiple)")
		->check(CLI::ExistingFile)
		->group("MPQ and Plugins");

	patch->add_option("--params", m_patchCommand.parameters,
		"Command-line parameters to pass to the target")
		->group("Patching Options");

	patch->add_flag("--extended-redir,!--no-extended-redir", m_patchCommand.extendedRedir,
		"Enable/disable extended file redirection (default: enabled)")
		->group("Patching Options");

	patch->add_flag("--no-spawning", m_patchCommand.noSpawning,
		"Do not inject into child processes")
		->group("Patching Options");

	patch->add_option("--shunt-count", m_patchCommand.shuntCount,
		"Number of times target restarts before patching activates (default: 0)")
		->default_val(0)
		->check(CLI::NonNegativeNumber)
		->group("Patching Options");

	// =========================================================================
	// SEMPQ subcommand
	// =========================================================================
	auto* sempq = app.add_subcommand("sempq", "Create a Self-Executing MPQ");

	// Use custom formatter for better visual grouping
	auto formatter = std::make_shared<GroupedFormatter>();
	sempq->formatter(formatter);

	// -------------------------------------------------------------------------
	// Output options
	// -------------------------------------------------------------------------
	sempq->add_option("-o,--output", m_sempqCommand.outputPath,
		"Output SEMPQ file path")
		->required()
		->group("Output");

	sempq->add_option("-n,--name", m_sempqCommand.sempqName,
		"Display name for the SEMPQ")
		->required()
		->group("Output");

	sempq->add_option("--icon", m_sempqCommand.iconPath,
		"Custom icon file for the SEMPQ")
		->check(CLI::ExistingFile)
		->group("Output");

	// -------------------------------------------------------------------------
	// MPQ and Plugins (at least one must be specified - validated after parsing)
	// -------------------------------------------------------------------------
	sempq->add_option("-m,--mpq", m_sempqCommand.mpqPath,
		"MPQ archive to embed")
		->check(CLI::ExistingFile)
		->group("MPQ and Plugins");

	sempq->add_option("-p,--plugin", m_sempqCommand.plugins,
		"Plugin file(s) to embed (can specify multiple)")
		->check(CLI::ExistingFile)
		->group("MPQ and Plugins");

	// -------------------------------------------------------------------------
	// Target selection - Mode 1: Supported game (by alias)
	// -------------------------------------------------------------------------
	sempq->add_option("-g,--game", m_sempqCommand.gameName,
		"Game alias (use 'list-games' to see available aliases)")
		->group("Supported Game");

	// -------------------------------------------------------------------------
	// Target selection - Mode 2: Custom registry
	// -------------------------------------------------------------------------
	sempq->add_option("--reg-key", m_sempqCommand.registryKey,
		"Registry key for game install path")
		->group("Custom Registry Target");

	sempq->add_option("--reg-value", m_sempqCommand.registryValue,
		"Registry value name")
		->group("Custom Registry Target");

	sempq->add_option("--exe-file", m_sempqCommand.exeFileName,
		"Executable filename to launch")
		->group("Custom Registry Target");

	sempq->add_option("--target-file", m_sempqCommand.targetFileName,
		"Target filename for patching")
		->group("Custom Registry Target");

	sempq->add_flag("--full-path", m_sempqCommand.fullPath,
		"Registry value is full path (not directory)")
		->group("Custom Registry Target");

	// -------------------------------------------------------------------------
	// Target selection - Mode 3: Custom target path
	// -------------------------------------------------------------------------
	sempq->add_option("--target", m_sempqCommand.targetPath,
		"Direct path to target executable")
		->group("Custom Target Path");

	// -------------------------------------------------------------------------
	// Patching options
	// -------------------------------------------------------------------------
	sempq->add_option("--params", m_sempqCommand.parameters,
		"Command-line parameters to pass to the target")
		->group("Patching Options");

	sempq->add_flag("--extended-redir,!--no-extended-redir", m_sempqCommand.extendedRedir,
		"Enable/disable extended file redirection (default: enabled)")
		->group("Patching Options");

	sempq->add_flag("--no-spawning", m_sempqCommand.noSpawning,
		"Do not inject into child processes")
		->group("Patching Options");

	sempq->add_option("--shunt-count", m_sempqCommand.shuntCount,
		"Number of times target restarts before patching activates (default: 0)")
		->default_val(0)
		->check(CLI::NonNegativeNumber)
		->group("Patching Options");

	// =========================================================================
	// List-games subcommand
	// =========================================================================
	auto* listGames = app.add_subcommand("list-games", "List supported games");

	// =========================================================================
	// Parse
	// =========================================================================
	try {
		app.parse(argc, argv);
	} catch (const CLI::CallForVersion& e) {
		// Version was requested
		m_versionRequested = true;
		m_message = e.what();
		return true;
	} catch ([[maybe_unused]] const CLI::CallForHelp& e) {
		// Help was requested
		m_helpRequested = true;
		m_message = app.help();
		return true;
	} catch (const CLI::ParseError& e) {
		if (e.get_exit_code() == 0) {
			// Some other success exit (shouldn't happen but handle it)
			m_helpRequested = true;
			m_message = app.help();
			return true;
		}
		// Parse error
		m_message = e.what() + std::string("\n\n") + app.help();
		return false;
	}

	// Check which subcommand was used
	if (app.got_subcommand(patch)) {
		m_commandType = CommandType::Patch;

		// Validate: need at least one MPQ or plugin
		if (m_patchCommand.mpqs.empty() && m_patchCommand.plugins.empty()) {
			m_message = "Error: At least one MPQ (--mpq) or plugin (--plugin) must be specified.\n\n" + patch->help();
			return false;
		}
		return true;
	}

	if (app.got_subcommand(sempq)) {
		m_commandType = CommandType::SEMPQ;

		// Validate that at least one of --mpq or --plugin is specified
		bool hasMpq = !m_sempqCommand.mpqPath.empty();
		bool hasPlugins = !m_sempqCommand.plugins.empty();
		if (!hasMpq && !hasPlugins) {
			m_message = "Error: Must specify at least one of --mpq or --plugin\n\n" + sempq->help();
			return false;
		}

		// Determine which mode was specified
		bool hasGame = !m_sempqCommand.gameName.empty();
		bool hasRegistry = !m_sempqCommand.registryKey.empty() || !m_sempqCommand.registryValue.empty();
		bool hasTarget = !m_sempqCommand.targetPath.empty();

		int modeCount = (hasGame ? 1 : 0) + (hasRegistry ? 1 : 0) + (hasTarget ? 1 : 0);

		if (modeCount == 0) {
			m_message = "Error: Must specify a target mode: --game, --reg-key/--reg-value, or --target\n\n" + sempq->help();
			return false;
		}

		if (modeCount > 1) {
			m_message = "Error: Cannot mix target modes. Use only one of: --game, --reg-key/--reg-value, or --target\n\n" + sempq->help();
			return false;
		}

		if (hasGame) {
			m_sempqCommand.mode = SEMPQTargetMode::SupportedGame;

			// Validate game alias exists
			const SupportedGame* game = nullptr;
			const GameComponent* component = nullptr;
			if (!findGameByAlias(m_sempqCommand.gameName, &game, &component)) {
				m_message = "Error: Unknown game alias '" + m_sempqCommand.gameName + "'\n\n" + buildGameList();
				return false;
			}
		} else if (hasRegistry) {
			m_sempqCommand.mode = SEMPQTargetMode::CustomRegistry;

			// Validate required fields
			if (m_sempqCommand.registryKey.empty()) {
				m_message = "Error: --reg-key is required for custom registry mode\n\n" + sempq->help();
				return false;
			}
			if (m_sempqCommand.registryValue.empty()) {
				m_message = "Error: --reg-value is required for custom registry mode\n\n" + sempq->help();
				return false;
			}
			if (!m_sempqCommand.fullPath) {
				// If not full path mode, we need exe and target filenames
				if (m_sempqCommand.exeFileName.empty()) {
					m_message = "Error: --exe-file is required when --full-path is not set\n\n" + sempq->help();
					return false;
				}
				if (m_sempqCommand.targetFileName.empty()) {
					m_message = "Error: --target-file is required when --full-path is not set\n\n" + sempq->help();
					return false;
				}
			}
		} else if (hasTarget) {
			m_sempqCommand.mode = SEMPQTargetMode::CustomTarget;
		}

		return true;
	}

	if (app.got_subcommand(listGames)) {
		m_commandType = CommandType::ListGames;
		m_message = buildGameList();
		return true;
	}

	// No subcommand - show help
	m_helpRequested = true;
	m_message = app.help();
	return true;
}
