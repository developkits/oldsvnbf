#define ENG_VERSION		62					// engine version, integer is divided by 100.f
#define ENG_NAME		"Blood Frontier"	// engine namn
#define ENG_RELEASE		"Alpha 2"			// engine release name

#define MASTER_PORT		28800

#include "tools.h"
#if !defined(STANDALONE) && !defined(DAEMON)
#include "geom.h"
#include "attrs.h"
#include "command.h"
#include "ents.h"
#endif
