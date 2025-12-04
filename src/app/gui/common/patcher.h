/*
    The contents of this file are subject to the Common Development and Distribution License Version 1.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.sun.com/cddl/cddl.html.

    Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the specific language governing rights and limitations under the License.

    The Initial Developer of the Original Code is Justin Olbrantz. The Original Code Copyright (C) 2008 Justin Olbrantz. All Rights Reserved.
*/

// Simplified version of src/dll/Patcher.h for Qt GUI
// Contains only the constants and types needed by the GUI

#ifndef PATCHER_H
#define PATCHER_H

// The MPQDraft patcher forms an additional layer of patching outside of the patcher in QInjectDLL. 
// It adds on all the features necessary to do the patching MPQDraft requires, as opposed to merely 
// providing basic patching services.

// These constants define the size of arrays used in the patching process. It should be possible to 
// remove these limitations by dynamically allocating the memory for the arrays; I just haven't 
// (though I'm generally wary of allocating memory in target processes).
#define MAX_PATCH_MPQS 8
#define MAX_MPQDRAFT_PLUGINS 8
#define MAX_AUXILIARY_MODULES 32

#endif // PATCHER_H

