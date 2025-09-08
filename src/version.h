#ifndef VERSION_H
#define VERSION_H

#define MAJOR_VERSION 2
#define MINOR_VERSION 1
#define PATCH_VERSION 1
#define BUILD_VERSION 3

#ifdef INNO_SETUP
    #define APP_VERSION Str(MAJOR_VERSION) + "." + Str(MINOR_VERSION) + "." + Str(PATCH_VERSION)
#else
	#define str(s) #s
	#define xstr(s) str(s)
    #define APP_VERSION xstr(MAJOR_VERSION) "." xstr(MINOR_VERSION) "." xstr(PATCH_VERSION)

	// If it is defined by the compiler, then it is a nightly build, and in the YYYYMMDD format.
	#ifndef BUILD_VERSION
		#define BUILD_VERSION 0
	#endif
#endif

// GUI repo (already used by your GUI checker)
#define APP_REPO_OWNER   "invictuscockpits"
#define APP_REPO_NAME    "HOTASConfigurator"

// FIRMWARE repo (adjust to your real repo)
#define FW_REPO_OWNER    "invictuscockpits"
#define FW_REPO_NAME     "invictus-ssc-firmware"

// Prefix for firmware tags like "fw-v2.0.3", set this:
#define FW_TAG_PREFIX    "fw-"

#endif // VERSION_H
