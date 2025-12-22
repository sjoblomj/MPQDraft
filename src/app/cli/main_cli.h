/*
	The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

	Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

	The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// main_cli.h : CLI entry point declaration

#ifndef MAIN_CLI_H
#define MAIN_CLI_H

// Entry point for the CLI
// Called from the main entry point when CLI arguments are provided
// argc: Number of command line arguments
// argv: Array of command line argument strings
// Returns: 0 on success, non-zero on failure
int runCli(int argc, char** argv);

#endif // MAIN_CLI_H

